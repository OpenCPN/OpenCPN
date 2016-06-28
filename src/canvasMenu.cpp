/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  CanvasMenuHandler
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers
#include "wx/image.h"
#include <wx/graphics.h>
#include <wx/listbook.h>
#include <wx/clipbrd.h>
#include <wx/aui/aui.h>

#include "dychart.h"

#include <wx/listimpl.cpp>

#include "canvasMenu.h"

#include "chcanv.h"
#include "TCWin.h"
#include "georef.h"
#include "styles.h"
#include "routeman.h"
#include "navutil.h"
#include "kml.h"
#include "chartdb.h"
#include "chart1.h"
#include "cutil.h"
#include "routeprop.h"
#include "TrackPropDlg.h"
#include "tcmgr.h"
#include "routemanagerdialog.h"
#include "pluginmanager.h"
#include "undo.h"
#include "tide_time.h"
#include "Quilt.h"
#include "FontMgr.h"
#include "AIS_Decoder.h"
#include "AIS_Target_Data.h"
#include "SendToGpsDlg.h"


#ifdef USE_S57
#include "cm93.h"                   // for chart outline draw
#include "s57chart.h"               // for ArrayOfS57Obj
#include "s52plib.h"
#endif

#include "ais.h"

#include <vector>

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif

// ----------------------------------------------------------------------------
// Useful Prototypes
// ----------------------------------------------------------------------------
extern void pupHandler_PasteRoute();
extern void pupHandler_PasteTrack();
extern void pupHandler_PasteWaypoint();


extern AIS_Decoder      *g_pAIS;
extern bool             g_bShowAIS;
extern bool             g_bShowAreaNotices;
extern ChartBase        *Current_Ch;
extern bool             bGPSValid;
extern Routeman         *g_pRouteMan;
extern bool             g_bCourseUp;
extern bool             g_bskew_comp;
extern double           gLat, gLon, gSog, gCog, vLat, vLon;
extern MyFrame          *gFrame;
extern ChartGroupArray  *g_pGroupArray;
extern PlugInManager    *g_pi_manager;
extern int              g_nAWMax;
extern int              g_nAWDefault;
extern RoutePoint       *pAnchorWatchPoint1;
extern RoutePoint       *pAnchorWatchPoint2;
extern double           AnchorPointMinDist;
extern bool             AnchorAlertOn1;
extern bool             AnchorAlertOn2;
extern wxString         g_AW1GUID;
extern wxString         g_AW2GUID;
extern int              g_click_stop;
extern RouteManagerDialog *pRouteManagerDialog;
extern MarkInfoImpl     *pMarkPropDialog;
extern RouteProp        *pRoutePropDialog;
extern TrackPropDlg     *pTrackPropDialog;
extern ActiveTrack      *g_pActiveTrack;
extern bool             g_bConfirmObjectDelete;
extern WayPointman      *pWayPointMan;
extern MyConfig         *pConfig;
extern Select           *pSelect;

#ifdef USE_S57
extern s52plib          *ps52plib;
extern CM93OffsetDialog  *g_pCM93OffsetDialog;
#endif

extern GoToPositionDialog *pGoToPositionDialog;
extern RouteList        *pRouteList;
extern wxString         g_default_wp_icon;
extern ChartStack       *pCurrentStack;
extern bool              g_btouch;
extern int              g_GroupIndex;
extern bool             g_bBasicMenus;



//    Constants for right click menus
enum
{
    ID_DEF_MENU_MAX_DETAIL = 1,
    ID_DEF_MENU_SCALE_IN,
    ID_DEF_MENU_SCALE_OUT,
    ID_DEF_MENU_DROP_WP,
    ID_DEF_MENU_QUERY,
    ID_DEF_MENU_MOVE_BOAT_HERE,
    ID_DEF_MENU_GOTO_HERE,
    ID_DEF_MENU_GOTOPOSITION,

    ID_WP_MENU_GOTO,
    ID_WP_MENU_DELPOINT,
    ID_WP_MENU_PROPERTIES,
    ID_RT_MENU_ACTIVATE,
    ID_RT_MENU_DEACTIVATE,
    ID_RT_MENU_INSERT,
    ID_RT_MENU_APPEND,
    ID_RT_MENU_COPY,
    ID_TK_MENU_COPY,
    ID_WPT_MENU_COPY,
    ID_WPT_MENU_SENDTOGPS,
    ID_WPT_MENU_SENDTONEWGPS,
    ID_PASTE_WAYPOINT,
    ID_PASTE_ROUTE,
    ID_PASTE_TRACK,
    ID_RT_MENU_DELETE,
    ID_RT_MENU_REVERSE,
    ID_RT_MENU_DELPOINT,
    ID_RT_MENU_ACTPOINT,
    ID_RT_MENU_DEACTPOINT,
    ID_RT_MENU_ACTNXTPOINT,
    ID_RT_MENU_REMPOINT,
    ID_RT_MENU_PROPERTIES,
    ID_RT_MENU_SENDTOGPS,
    ID_RT_MENU_SENDTONEWGPS,
    ID_WP_MENU_SET_ANCHORWATCH,
    ID_WP_MENU_CLEAR_ANCHORWATCH,
    ID_DEF_MENU_AISTARGETLIST,

    ID_RC_MENU_SCALE_IN,
    ID_RC_MENU_SCALE_OUT,
    ID_RC_MENU_ZOOM_IN,
    ID_RC_MENU_ZOOM_OUT,
    ID_RC_MENU_FINISH,
    ID_DEF_MENU_AIS_QUERY,
    ID_DEF_MENU_AIS_CPA,
    ID_DEF_MENU_AISSHOWTRACK,
    ID_DEF_MENU_ACTIVATE_MEASURE,
    ID_DEF_MENU_DEACTIVATE_MEASURE,

    ID_UNDO,
    ID_REDO,

    ID_DEF_MENU_CM93OFFSET_DIALOG,

    ID_TK_MENU_PROPERTIES,
    ID_TK_MENU_DELETE,
    ID_WP_MENU_ADDITIONAL_INFO,

    ID_DEF_MENU_QUILTREMOVE,
    ID_DEF_MENU_COGUP,
    ID_DEF_MENU_NORTHUP,
    ID_DEF_MENU_TOGGLE_FULL,
    ID_DEF_MENU_TIDEINFO,
    ID_DEF_MENU_CURRENTINFO,
    ID_DEF_ZERO_XTE,
    
    ID_DEF_MENU_GROUPBASE,  // Must be last entry, as chart group identifiers are created dynamically

    
    ID_DEF_MENU_LAST
};

//------------------------------------------------------------------------------
//    CanvasMenuHandler Implementation
//------------------------------------------------------------------------------

// Define a constructor for my canvas
CanvasMenuHandler::CanvasMenuHandler(ChartCanvas *parentCanvas,
                  Route *selectedRoute,
                  Track *selectedTrack,
                  RoutePoint *selectedPoint,
                  int selectedAIS_MMSI,
                  void *selectedTCIndex )

{
    parent = parentCanvas;
    m_pSelectedRoute = selectedRoute;
    m_pSelectedTrack = selectedTrack;
    m_pFoundRoutePoint = selectedPoint;
    m_FoundAIS_MMSI = selectedAIS_MMSI;
    m_pIDXCandidate = selectedTCIndex;
}

CanvasMenuHandler::~CanvasMenuHandler()
{

}


//-------------------------------------------------------------------------------
//          Popup Menu Handling
//-------------------------------------------------------------------------------

void MenuPrepend1( wxMenu *menu, int id, wxString label)
{
    wxMenuItem *item = new wxMenuItem(menu, id, label);
#if defined(__WXMSW__)
    wxFont *qFont = GetOCPNScaledFont(_T("Menu"));
    item->SetFont(*qFont);
#endif
    
#ifdef __WXQT__
    wxFont sFont = GetOCPNGUIScaledFont(_T("Menu"));
    item->SetFont(sFont);
#endif
    
    menu->Prepend(item);
}

void MenuAppend1( wxMenu *menu, int id, wxString label)
{
    wxMenuItem *item = new wxMenuItem(menu, id, label);
#if defined(__WXMSW__)
   
    wxFont *qFont = GetOCPNScaledFont(_("Menu"));
    item->SetFont(*qFont);
#endif
    
#ifdef __WXQT__
    wxFont sFont = GetOCPNGUIScaledFont(_T("Menu"));
    item->SetFont(sFont);
#endif
    
    menu->Append(item);
}

void SetMenuItemFont1(wxMenuItem *item)
{
#if defined(__WXMSW__) || defined(__OCPN__ANDROID__)
    
    wxFont *qFont = GetOCPNScaledFont(_("Menu"));
    item->SetFont(*qFont);
#endif
}

void CanvasMenuHandler::CanvasPopupMenu( int x, int y, int seltype )
{
    wxMenu* contextMenu = new wxMenu;
    wxMenu* menuWaypoint = new wxMenu( _("Waypoint") );
    wxMenu* menuRoute = new wxMenu( _("Route") );
    wxMenu* menuTrack = new wxMenu( _("Track") );
    wxMenu* menuAIS = new wxMenu( _("AIS") );

    wxMenu *subMenuChart = new wxMenu;

    wxMenu *menuFocus = contextMenu;    // This is the one that will be shown

    popx = x;
    popy = y;

    if( seltype == SELTYPE_ROUTECREATE ) {
        MenuAppend1( contextMenu, ID_RC_MENU_FINISH, _menuText( _( "End Route" ), _T("Esc") ) );
    }

    if( ! parent->m_pMouseRoute ) {
        if( parent->m_bMeasure_Active )
            MenuPrepend1( contextMenu, ID_DEF_MENU_DEACTIVATE_MEASURE, _menuText( _("Measure Off"), _T("Esc") ) );
        else
            MenuPrepend1( contextMenu, ID_DEF_MENU_ACTIVATE_MEASURE, _menuText( _( "Measure" ), _T("M") ) );
//            contextMenu->Prepend( ID_DEF_MENU_ACTIVATE_MEASURE, _menuText( _( "Measure" ), _T("F4") ) );
    }

    if( !g_bBasicMenus || (seltype != SELTYPE_ROUTECREATE )) {
        if( parent->undo->AnythingToUndo() ) {
            wxString undoItem;
            undoItem << _("Undo") << _T(" ") << parent->undo->GetNextUndoableAction()->Description();
            MenuPrepend1( contextMenu, ID_UNDO, _menuText( undoItem, _T("Ctrl-Z") ) );
        }

        if( parent->undo->AnythingToRedo() ) {
            wxString redoItem;
            redoItem << _("Redo") << _T(" ") << parent->undo->GetNextRedoableAction()->Description();
#ifdef __WXOSX__
            MenuPrepend1( contextMenu, ID_REDO, _menuText( redoItem, _T("Shift-Ctrl-Z") ) );
#else
            MenuPrepend1( contextMenu, ID_REDO, _menuText( redoItem, _T("Ctrl-Y") ) );
#endif
        }
    }

    bool ais_areanotice = false;
    if( g_pAIS && g_bShowAIS && g_bShowAreaNotices ) {

        AIS_Target_Hash* an_sources = g_pAIS->GetAreaNoticeSourcesList();

        float vp_scale = parent->GetVPScale();

        for( AIS_Target_Hash::iterator target = an_sources->begin(); target != an_sources->end(); ++target ) {
            AIS_Target_Data* target_data = target->second;
            if( !target_data->area_notices.empty() ) {
                for( AIS_Area_Notice_Hash::iterator ani = target_data->area_notices.begin(); ani != target_data->area_notices.end(); ++ani ) {
                    Ais8_001_22& area_notice = ani->second;
                    wxBoundingBox bbox;

                    for( Ais8_001_22_SubAreaList::iterator sa = area_notice.sub_areas.begin(); sa != area_notice.sub_areas.end(); ++sa ) {
                        switch( sa->shape ) {
                            case AIS8_001_22_SHAPE_CIRCLE: {
                                wxPoint target_point;
                                parent->GetCanvasPointPix( sa->latitude, sa->longitude, &target_point );
                                bbox.Expand( target_point );
                                if( sa->radius_m > 0.0 )
                                    bbox.EnLarge( sa->radius_m * vp_scale );
                                break;
                            }
                            case AIS8_001_22_SHAPE_POLYGON:
                            case AIS8_001_22_SHAPE_POLYLINE: {
                                double lat = sa->latitude;
                                double lon = sa->longitude;
                                for( int i = 0; i < 4; ++i ) {
                                    ll_gc_ll( lat, lon, sa->angles[i], sa->dists_m[i] / 1852.0,
                                              &lat, &lon );
                                    wxPoint target_point;
                                    parent->GetCanvasPointPix( lat, lon, &target_point );
                                    bbox.Expand( target_point );
                                }
                            }
                        }
                    }

                    if( bbox.PointInBox( x, y ) ) {
                        ais_areanotice = true;
                        break;
                    }
                }
            }
        }
    }
    if( !parent->GetVP().b_quilt ) {
        if( parent->parent_frame->GetnChartStack() > 1 ) {
            MenuAppend1( contextMenu, ID_DEF_MENU_MAX_DETAIL, _( "Max Detail Here" ) );
            MenuAppend1( contextMenu, ID_DEF_MENU_SCALE_IN, _menuText( _( "Scale In" ), _T("Ctrl-Left") ) );
            MenuAppend1( contextMenu, ID_DEF_MENU_SCALE_OUT, _menuText( _( "Scale Out" ), _T("Ctrl-Right") ) );
        }

        if( ( Current_Ch && ( Current_Ch->GetChartFamily() == CHART_FAMILY_VECTOR ) ) || ais_areanotice ) {
            MenuAppend1( contextMenu, ID_DEF_MENU_QUERY, _( "Object Query..." ) );
        }

    } else {
        ChartBase *pChartTest = parent->m_pQuilt->GetChartAtPix( parent->GetVP(), wxPoint( x, y ) );
        if( ( pChartTest && ( pChartTest->GetChartFamily() == CHART_FAMILY_VECTOR ) ) || ais_areanotice ) {
            MenuAppend1( contextMenu, ID_DEF_MENU_QUERY, _( "Object Query..." ) );
        } else {
#ifndef __OCPN__ANDROID__            
            if( !g_bBasicMenus && (parent->parent_frame->GetnChartStack() > 1 ) ) {
                MenuAppend1( contextMenu, ID_DEF_MENU_SCALE_IN, _menuText( _( "Scale In" ), _T("Ctrl-Left") ) );
                MenuAppend1( contextMenu, ID_DEF_MENU_SCALE_OUT, _menuText( _( "Scale Out" ), _T("Ctrl-Right") ) );
            }
#endif            
        }
    }

    if( !g_bBasicMenus || (seltype != SELTYPE_ROUTECREATE )) {
        MenuAppend1( contextMenu, ID_DEF_MENU_DROP_WP, _menuText( _( "Drop Mark" ), _T("Ctrl-M") ) );

        if( !bGPSValid )
            MenuAppend1( contextMenu, ID_DEF_MENU_MOVE_BOAT_HERE, _( "Move Boat Here" ) );
    }

    if( !g_bBasicMenus && (!( g_pRouteMan->GetpActiveRoute() || ( seltype & SELTYPE_MARKPOINT )) ) )
        MenuAppend1( contextMenu, ID_DEF_MENU_GOTO_HERE, _( "Navigate To Here" ) );

    if( !g_bBasicMenus)
        MenuAppend1( contextMenu, ID_DEF_MENU_GOTOPOSITION, _("Center View...") );

    if( !g_bBasicMenus){
        if( !g_bCourseUp )
            MenuAppend1( contextMenu, ID_DEF_MENU_COGUP, _("Course Up Mode") );
        else {
            if( !parent->GetVP().b_quilt && Current_Ch && ( fabs( Current_Ch->GetChartSkew() ) > .01 )
                && !g_bskew_comp ) MenuAppend1( contextMenu, ID_DEF_MENU_NORTHUP, _("Chart Up Mode") );
            else
                MenuAppend1( contextMenu, ID_DEF_MENU_NORTHUP, _("North Up Mode") );
        }
    }

    if( !g_bBasicMenus){
            bool full_toggle_added = false;
#ifndef __OCPN__ANDROID__
        if(g_btouch){
            MenuAppend1( contextMenu, ID_DEF_MENU_TOGGLE_FULL, _("Toggle Full Screen") );
            full_toggle_added = true;
        }
            
        
        if(!full_toggle_added){
            if(gFrame->IsFullScreen())
                MenuAppend1( contextMenu, ID_DEF_MENU_TOGGLE_FULL, _("Toggle Full Screen") );
        }
#endif            
        
        if ( g_pRouteMan->IsAnyRouteActive() && g_pRouteMan->GetCurrentXTEToActivePoint() > 0. ) 
            MenuAppend1( contextMenu, ID_DEF_ZERO_XTE, _("Zero XTE") );

        Kml* kml = new Kml;
        int pasteBuffer = kml->ParsePasteBuffer();
        if( pasteBuffer != KML_PASTE_INVALID ) {
            switch( pasteBuffer ) {
                case KML_PASTE_WAYPOINT: {
                    MenuAppend1( contextMenu, ID_PASTE_WAYPOINT, _( "Paste Waypoint" ) );
                    break;
                }
                case KML_PASTE_ROUTE: {
                    MenuAppend1( contextMenu, ID_PASTE_ROUTE, _( "Paste Route" ) );
                    break;
                }
                case KML_PASTE_TRACK: {
                    MenuAppend1( contextMenu, ID_PASTE_TRACK, _( "Paste Track" ) );
                    break;
                }
                case KML_PASTE_ROUTE_TRACK: {
                    MenuAppend1( contextMenu, ID_PASTE_ROUTE, _( "Paste Route" ) );
                    MenuAppend1( contextMenu, ID_PASTE_TRACK, _( "Paste Track" ) );
                    break;
                }
            }
        }
        delete kml;

        if( !parent->GetVP().b_quilt && Current_Ch && ( Current_Ch->GetChartType() == CHART_TYPE_CM93COMP ) ) {
            MenuAppend1( contextMenu, ID_DEF_MENU_CM93OFFSET_DIALOG, _( "CM93 Offset Dialog..." ) );
        }

    }   //if( !g_bBasicMenus){
        
 
#ifndef __OCPN__ANDROID__        
    if( ( parent->GetVP().b_quilt ) && ( pCurrentStack && pCurrentStack->b_valid ) ) {
        int dbIndex = parent->m_pQuilt->GetChartdbIndexAtPix( parent->GetVP(), wxPoint( popx, popy ) );
        if( dbIndex != -1 )
            MenuAppend1( contextMenu, ID_DEF_MENU_QUILTREMOVE, _( "Hide This Chart" ) );
    }
#endif


#ifdef __WXMSW__
    //  If we dismiss the context menu without action, we need to discard some mouse events....
    //  Eat the next 2 button events, which happen as down-up on MSW XP
    g_click_stop = 2;
#endif

    //  ChartGroup SubMenu
    wxMenuItem* subItemChart = contextMenu->AppendSubMenu( subMenuChart, _("Chart Groups") );
    SetMenuItemFont1(subItemChart);
    
    if( g_pGroupArray->GetCount() ) {

#ifdef __WXMSW__
          const wxString l[] = { _T(" "), wxString::Format( _T("\u2022") ) };
          wxMenuItem* subItem1 = subMenuChart->AppendRadioItem( wxID_CANCEL , _T("temporary") );
          SetMenuItemFont1(subItem1);
#endif
          wxMenuItem* subItem0 = subMenuChart->AppendRadioItem( ID_DEF_MENU_GROUPBASE ,
#ifdef __WXMSW__
                  ( g_GroupIndex == 0 ? l[1] : l[0] ) +
#endif
                  _("All Active Charts") );



        SetMenuItemFont1(subItem0);

        for( unsigned int i = 0; i < g_pGroupArray->GetCount(); i++ ) {
            subItem0 = subMenuChart->AppendRadioItem( ID_DEF_MENU_GROUPBASE + i + 1,
#ifdef __WXMSW__
                     ( i == g_GroupIndex - 1 ? l[1] : l[0] ) +
#endif
                     g_pGroupArray->Item( i )->m_group_name );
            SetMenuItemFont1(subItem0);
        }
        
#ifdef __WXMSW__
    subMenuChart->Remove( wxID_CANCEL );
#endif
        subMenuChart->Check( ID_DEF_MENU_GROUPBASE + g_GroupIndex, true );
    }
    
        
    //  Add PlugIn Context Menu items
    ArrayOfPlugInMenuItems item_array = g_pi_manager->GetPluginContextMenuItemArray();

    for( unsigned int i = 0; i < item_array.GetCount(); i++ ) {
        PlugInMenuItemContainer *pimis = item_array.Item( i );
        {
            if( pimis->b_viz ) {
                wxMenu *submenu = NULL;
                if(pimis->pmenu_item->GetSubMenu()) {
                    submenu = new wxMenu();
                    const wxMenuItemList &items = pimis->pmenu_item->GetSubMenu()->GetMenuItems();
                    for( wxMenuItemList::const_iterator it = items.begin(); it != items.end(); ++it ) {
                        int id = -1;
                        for( unsigned int j = 0; j < item_array.GetCount(); j++ ) {
                            PlugInMenuItemContainer *pimis = item_array.Item( j );
                            if(pimis->pmenu_item == *it)
                                id = pimis->id;
                        }

                        wxMenuItem *pmi = new wxMenuItem( submenu, id,
#if wxCHECK_VERSION(3,0,0)
                                                        (*it)->GetItemLabelText(),
#else
                                                        (*it)->GetLabel(),
#endif
                                                        (*it)->GetHelp(),
                                                          (*it)->GetKind());
                        submenu->Append(pmi);
                        pmi->Check((*it)->IsChecked());
                    }
                }
                
                wxMenuItem *pmi = new wxMenuItem( contextMenu, pimis->id,
#if wxCHECK_VERSION(3,0,0)
                                                  pimis->pmenu_item->GetItemLabelText(),
#else
                                                  pimis->pmenu_item->GetLabel(),
#endif
                                                  pimis->pmenu_item->GetHelp(),
                                                  pimis->pmenu_item->GetKind(),
                                                  submenu );
#ifdef __WXMSW__
                pmi->SetFont(pimis->pmenu_item->GetFont());
#endif
                contextMenu->Append( pmi );
                contextMenu->Enable( pimis->id, !pimis->b_grey );
            }
        }
    }

    //  This is the default context menu
    menuFocus = contextMenu;

    if( !g_bBasicMenus || (seltype != SELTYPE_ROUTECREATE )) {
        if( g_pAIS ) {
            MenuAppend1( contextMenu, ID_DEF_MENU_AISTARGETLIST, _("AIS Target List...") );

            if( g_bShowAIS && (seltype & SELTYPE_AISTARGET) ) {
                MenuAppend1( menuAIS, ID_DEF_MENU_AIS_QUERY, _( "Target Query..." ) );
                AIS_Target_Data *myptarget = g_pAIS->Get_Target_Data_From_MMSI( m_FoundAIS_MMSI );
                if( myptarget && myptarget->bCPA_Valid && (myptarget->n_alert_state != AIS_ALERT_SET) ) {
                    if( myptarget->b_show_AIS_CPA )
                        MenuAppend1( menuAIS, ID_DEF_MENU_AIS_CPA, _( "Hide Target CPA" ) );
                    else
                        MenuAppend1( menuAIS, ID_DEF_MENU_AIS_CPA, _( "Show Target CPA" ) );
                }
                MenuAppend1( menuAIS, ID_DEF_MENU_AISTARGETLIST, _("Target List...") );
                if ( 1 /*g_bAISShowTracks*/ ) {
                    if( myptarget && myptarget->b_show_track )
                        MenuAppend1( menuAIS, ID_DEF_MENU_AISSHOWTRACK, _("Hide Target Track") );
                    else
                        MenuAppend1( menuAIS, ID_DEF_MENU_AISSHOWTRACK, _("Show Target Track") );
                }
                
                menuAIS->AppendSeparator();
                
                if( !parent->GetVP().b_quilt ) {
                    if( ( Current_Ch && ( Current_Ch->GetChartFamily() == CHART_FAMILY_VECTOR ) ) ) {
                        MenuAppend1( menuAIS, ID_DEF_MENU_QUERY, _( "Object Query..." ) );
                    }
                    
                } else {
                    ChartBase *pChartTest = parent->m_pQuilt->GetChartAtPix( parent->GetVP(), wxPoint( x, y ) );
                    if( ( pChartTest && ( pChartTest->GetChartFamily() == CHART_FAMILY_VECTOR ) ) ) {
                        MenuAppend1( menuAIS, ID_DEF_MENU_QUERY, _( "Object Query..." ) );
                    }
                }
                        
                
                menuFocus = menuAIS;
            }
        }
    }

    if( seltype & SELTYPE_ROUTESEGMENT ) {
        bool blay = false;
        if( m_pSelectedRoute && m_pSelectedRoute->m_bIsInLayer )
            blay = true;

        if( blay ){
            delete menuRoute;
            menuRoute = new wxMenu( _("Layer Route") );
            MenuAppend1( menuRoute, ID_RT_MENU_PROPERTIES, _( "Properties..." ) );
            if( m_pSelectedRoute ) {
                if( m_pSelectedRoute->IsActive() ) {
                    int indexActive = m_pSelectedRoute->GetIndexOf( m_pSelectedRoute->m_pRouteActivePoint );
                    if( ( indexActive + 1 ) <= m_pSelectedRoute->GetnPoints() ) {
                        MenuAppend1( menuRoute, ID_RT_MENU_ACTNXTPOINT, _( "Activate Next Waypoint" ) );
                    }
                    MenuAppend1( menuRoute, ID_RT_MENU_DEACTIVATE, _( "Deactivate" ) );
                }
                else {
                    MenuAppend1( menuRoute, ID_RT_MENU_ACTIVATE, _( "Activate" ) );
                }
            }
        }
        else {
            MenuAppend1( menuRoute, ID_RT_MENU_PROPERTIES, _( "Properties..." ) );
            if( m_pSelectedRoute ) {
                if( m_pSelectedRoute->IsActive() ) {
                    int indexActive = m_pSelectedRoute->GetIndexOf( m_pSelectedRoute->m_pRouteActivePoint );
                    if( ( indexActive + 1 ) <= m_pSelectedRoute->GetnPoints() ) {
                        MenuAppend1( menuRoute, ID_RT_MENU_ACTNXTPOINT, _( "Activate Next Waypoint" ) );
                    }
                    MenuAppend1( menuRoute, ID_RT_MENU_DEACTIVATE, _( "Deactivate" ) );
                }
                else {
                    MenuAppend1( menuRoute, ID_RT_MENU_ACTIVATE, _( "Activate" ) );
                }
            }
            MenuAppend1( menuRoute, ID_RT_MENU_INSERT, _( "Insert Waypoint" ) );
            MenuAppend1( menuRoute, ID_RT_MENU_APPEND, _( "Append Waypoint" ) );
            MenuAppend1( menuRoute, ID_RT_MENU_COPY, _( "Copy as KML..." ) );
            MenuAppend1( menuRoute, ID_RT_MENU_DELETE, _( "Delete..." ) );
            MenuAppend1( menuRoute, ID_RT_MENU_REVERSE, _( "Reverse..." ) );

#ifndef __OCPN__ANDROID__
            wxString port = parent->FindValidUploadPort();
            parent->m_active_upload_port = port;
            wxString item = _( "Send to GPS" );
            if( !port.IsEmpty() ) {
                item.Append( _T(" ( ") );
                item.Append( port );
                item.Append(_T(" )") );
            }
            MenuAppend1( menuRoute, ID_RT_MENU_SENDTOGPS, item );

            if( !port.IsEmpty() ) {
                wxString item = _( "Send to new GPS" );
                MenuAppend1( menuRoute, ID_RT_MENU_SENDTONEWGPS, item );
            }
#endif                
                
        }
        //      Set this menu as the "focused context menu"
        menuFocus = menuRoute;
    }

    if( seltype & SELTYPE_TRACKSEGMENT ) {
        bool blay = false;
        if( m_pSelectedTrack && m_pSelectedTrack->m_bIsInLayer )
            blay = true;

        if( blay ) {
            delete menuTrack;
            menuTrack = new wxMenu( _("Layer Track") );
            MenuAppend1( menuTrack, ID_TK_MENU_PROPERTIES, _( "Properties..." ) );
        }
        else {
            MenuAppend1( menuTrack, ID_TK_MENU_PROPERTIES, _( "Properties..." ) );
            MenuAppend1( menuTrack, ID_TK_MENU_COPY, _( "Copy As KML" ) );
            MenuAppend1( menuTrack, ID_TK_MENU_DELETE, _( "Delete..." ) );
        }

        //      Set this menu as the "focused context menu"
        menuFocus = menuTrack;
    }

    if( seltype & SELTYPE_ROUTEPOINT ) {
        bool blay = false;
        if( m_pFoundRoutePoint && m_pFoundRoutePoint->m_bIsInLayer )
            blay = true;

        if( blay ){
            delete menuWaypoint;
            menuWaypoint = new wxMenu( _("Layer Routepoint") );
            MenuAppend1( menuWaypoint, ID_WP_MENU_PROPERTIES, _( "Properties..." ) );

            if( m_pSelectedRoute && m_pSelectedRoute->IsActive() )
                MenuAppend1( menuWaypoint, ID_RT_MENU_ACTPOINT, _( "Activate" ) );
        }
        else {
            MenuAppend1( menuWaypoint, ID_WP_MENU_PROPERTIES, _( "Properties..." ) );
            if( m_pSelectedRoute && m_pSelectedRoute->IsActive() ) {
                if(m_pSelectedRoute->m_pRouteActivePoint != m_pFoundRoutePoint )
                    MenuAppend1( menuWaypoint, ID_RT_MENU_ACTPOINT, _( "Activate" ) );
            }

            if( m_pSelectedRoute && m_pSelectedRoute->IsActive() ) {
                if(m_pSelectedRoute->m_pRouteActivePoint == m_pFoundRoutePoint ) {
                    int indexActive = m_pSelectedRoute->GetIndexOf( m_pSelectedRoute->m_pRouteActivePoint );
                    if( ( indexActive + 1 ) <= m_pSelectedRoute->GetnPoints() )
                        MenuAppend1( menuWaypoint, ID_RT_MENU_ACTNXTPOINT, _( "Activate Next Waypoint" ) );
                }
            }
            if( m_pSelectedRoute && m_pSelectedRoute->GetnPoints() > 2 )
                MenuAppend1( menuWaypoint, ID_RT_MENU_REMPOINT, _( "Remove from Route" ) );

            MenuAppend1( menuWaypoint, ID_WPT_MENU_COPY, _( "Copy as KML" ) );

            if( m_pFoundRoutePoint && m_pFoundRoutePoint->GetIconName() != _T("mob") )
                MenuAppend1( menuWaypoint, ID_RT_MENU_DELPOINT,  _( "Delete" ) );

            wxString port = parent->FindValidUploadPort();
            parent->m_active_upload_port = port;
            wxString item = _( "Send to GPS" );
            if( !port.IsEmpty() ) {
                item.Append( _T(" ( ") );
                item.Append( port );
                item.Append(_T(" )") );
            }
            MenuAppend1( menuWaypoint, ID_WPT_MENU_SENDTOGPS, item );
            
            if( !port.IsEmpty() ) {
                wxString item = _( "Send to new GPS" );
                MenuAppend1( menuWaypoint, ID_WPT_MENU_SENDTONEWGPS, item );
            }
            
            
        }
        //      Set this menu as the "focused context menu"
        menuFocus = menuWaypoint;
    }

    if( seltype & SELTYPE_MARKPOINT ) {
        bool blay = false;
        if( m_pFoundRoutePoint && m_pFoundRoutePoint->m_bIsInLayer )
            blay = true;

        if( blay ){
            delete menuWaypoint;
            menuWaypoint = new wxMenu( _("Layer Waypoint") );
            MenuAppend1( menuWaypoint, ID_WP_MENU_PROPERTIES, _( "Properties..." ) );
        }
        else {
            MenuAppend1( menuWaypoint, ID_WP_MENU_PROPERTIES, _( "Properties..." ) );

            if( !g_pRouteMan->GetpActiveRoute() )
                MenuAppend1( menuWaypoint, ID_WP_MENU_GOTO, _( "Navigate To This" ) );

            MenuAppend1( menuWaypoint, ID_WPT_MENU_COPY, _( "Copy as KML" ) );

            if( m_pFoundRoutePoint && m_pFoundRoutePoint->GetIconName() != _T("mob") )
                MenuAppend1( menuWaypoint, ID_WP_MENU_DELPOINT, _( "Delete" ) );

            wxString port = parent->FindValidUploadPort();
            parent->m_active_upload_port = port;
            wxString item = _( "Send to GPS" );
            if( !port.IsEmpty() ) {
                item.Append( _T(" ( ") );
                item.Append( port );
                item.Append(_T(" )") );
            }
            MenuAppend1( menuWaypoint, ID_WPT_MENU_SENDTOGPS, item );


            if( ( m_pFoundRoutePoint == pAnchorWatchPoint1 ) || ( m_pFoundRoutePoint == pAnchorWatchPoint2 ) )
                MenuAppend1( menuWaypoint, ID_WP_MENU_CLEAR_ANCHORWATCH, _( "Clear Anchor Watch" ) );
            else {
                if( m_pFoundRoutePoint && !( m_pFoundRoutePoint->m_bIsInLayer )
                    && ( ( NULL == pAnchorWatchPoint1 ) || ( NULL == pAnchorWatchPoint2 ) ) ) {

                    double dist;
                    double brg;
                    DistanceBearingMercator( m_pFoundRoutePoint->m_lat, m_pFoundRoutePoint->m_lon, gLat,
                                         gLon, &brg, &dist );
                    if( dist * 1852. <= g_nAWMax )
                        MenuAppend1( menuWaypoint,  ID_WP_MENU_SET_ANCHORWATCH,  _( "Set Anchor Watch" ) );
                }
            }
        }

        //      Set this menu as the "focused context menu"
        menuFocus = menuWaypoint;
    }

    if( ! subMenuChart->GetMenuItemCount() ) contextMenu->Destroy( subItemChart );

    //  Add the Tide/Current selections if the item was not activated by shortcut in right-click handlers
    bool bsep = false;
    if( seltype & SELTYPE_TIDEPOINT ){
        menuFocus->AppendSeparator();
        bsep = true;
        MenuAppend1( menuFocus, ID_DEF_MENU_TIDEINFO, _( "Show Tide Information" ) );
    }

    if( seltype & SELTYPE_CURRENTPOINT ) {
        if( !bsep )
            menuFocus->AppendSeparator();
        MenuAppend1( menuFocus,  ID_DEF_MENU_CURRENTINFO, _( "Show Current Information" ) );
    }

    //        Invoke the correct focused drop-down menu
    parent->PopupMenu( menuFocus, x, y );


    // Cleanup
    delete contextMenu;
    delete menuAIS;
    delete menuRoute;
    delete menuTrack;
    delete menuWaypoint;
}


void CanvasMenuHandler::PopupMenuHandler( wxCommandEvent& event )
{
    RoutePoint *pLast;

    wxPoint r;
    double zlat, zlon;

    parent->GetCanvasPixPoint( popx, popy, zlat, zlon );

    switch( event.GetId() ) {
    case ID_DEF_MENU_MAX_DETAIL:
        vLat = zlat;
        vLon = zlon;
        parent->ClearbFollow();

        parent->parent_frame->DoChartUpdate();

        parent->parent_frame->SelectChartFromStack( 0, false, CHART_TYPE_DONTCARE,
                                            CHART_FAMILY_RASTER );
        break;

    case ID_DEF_MENU_SCALE_IN:
        parent->parent_frame->DoStackDown();
        break;

    case ID_DEF_MENU_SCALE_OUT:
        parent->parent_frame->DoStackUp();
        break;

    case ID_UNDO:
        parent->undo->UndoLastAction();
        parent->InvalidateGL();
        parent->Refresh( false );
        break;

    case ID_REDO:
        parent->undo->RedoNextAction();
        parent->InvalidateGL();
        parent->Refresh( false );
        break;

    case ID_DEF_MENU_MOVE_BOAT_HERE:
        gLat = zlat;
        gLon = zlon;
        break;

    case ID_DEF_MENU_GOTO_HERE: {
        RoutePoint *pWP_dest = new RoutePoint( zlat, zlon, g_default_wp_icon, wxEmptyString,
                                               GPX_EMPTY_STRING );
        pSelect->AddSelectableRoutePoint( zlat, zlon, pWP_dest );

        RoutePoint *pWP_src = new RoutePoint( gLat, gLon, g_default_wp_icon, wxEmptyString,
                                              GPX_EMPTY_STRING );
        pSelect->AddSelectableRoutePoint( gLat, gLon, pWP_src );

        Route *temp_route = new Route();
        pRouteList->Append( temp_route );

        temp_route->AddPoint( pWP_src );
        temp_route->AddPoint( pWP_dest );

        pSelect->AddSelectableRouteSegment( gLat, gLon, zlat, zlon, pWP_src, pWP_dest,
                                            temp_route );

        temp_route->m_RouteNameString = _("Temporary GOTO Route");
        temp_route->m_RouteStartString = _("Here");
        ;
        temp_route->m_RouteEndString = _("There");

        temp_route->m_bDeleteOnArrival = true;

        if( g_pRouteMan->GetpActiveRoute() ) g_pRouteMan->DeactivateRoute();

        g_pRouteMan->ActivateRoute( temp_route, pWP_dest );

        break;
    }

    case ID_DEF_MENU_DROP_WP: {
        RoutePoint *pWP = new RoutePoint( zlat, zlon, g_default_wp_icon, wxEmptyString,
                                          GPX_EMPTY_STRING );
        pWP->m_bIsolatedMark = true;                      // This is an isolated mark
        pSelect->AddSelectableRoutePoint( zlat, zlon, pWP );
        pConfig->AddNewWayPoint( pWP, -1 );    // use auto next num

        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) pRouteManagerDialog->UpdateWptListCtrl();
        parent->undo->BeforeUndoableAction( Undo_CreateWaypoint, pWP, Undo_HasParent, NULL );
        parent->undo->AfterUndoableAction( NULL );
        parent->InvalidateGL();
        parent->Refresh( false );      // Needed for MSW, why not GTK??
        break;
    }

    case ID_DEF_MENU_AISTARGETLIST:
        parent->ShowAISTargetList();
        break;

    case ID_WP_MENU_GOTO: {
        RoutePoint *pWP_src = new RoutePoint( gLat, gLon, g_default_wp_icon, wxEmptyString,
                                              GPX_EMPTY_STRING );
        pSelect->AddSelectableRoutePoint( gLat, gLon, pWP_src );

        Route *temp_route = new Route();
        pRouteList->Append( temp_route );

        temp_route->AddPoint( pWP_src );
        temp_route->AddPoint( m_pFoundRoutePoint );
        m_pFoundRoutePoint->m_bKeepXRoute = true;

        pSelect->AddSelectableRouteSegment( gLat, gLon, m_pFoundRoutePoint->m_lat,
                                            m_pFoundRoutePoint->m_lon, pWP_src, m_pFoundRoutePoint, temp_route );

        wxString name = m_pFoundRoutePoint->GetName();
        if( name.IsEmpty() ) name = _("(Unnamed Waypoint)");
        wxString rteName = _("Go to ");
        rteName.Append( name );
        temp_route->m_RouteNameString = rteName;
        temp_route->m_RouteStartString = _("Here");
        ;
        temp_route->m_RouteEndString = name;
        temp_route->m_bDeleteOnArrival = true;

        if( g_pRouteMan->GetpActiveRoute() ) g_pRouteMan->DeactivateRoute();

        g_pRouteMan->ActivateRoute( temp_route, m_pFoundRoutePoint );

        break;
    }

    case ID_DEF_MENU_COGUP:
        gFrame->ToggleCourseUp();
        break;

    case ID_DEF_MENU_NORTHUP:
        gFrame->ToggleCourseUp();
        break;
        
    case ID_DEF_MENU_TOGGLE_FULL:
        gFrame->ToggleFullScreen();
        break;

    case ID_DEF_MENU_GOTOPOSITION:
        if( NULL == pGoToPositionDialog ) // There is one global instance of the Go To Position Dialog
            pGoToPositionDialog = new GoToPositionDialog( parent );
        pGoToPositionDialog->CheckPasteBufferForPosition();
        pGoToPositionDialog->Show();
        break;

    case ID_WP_MENU_DELPOINT: {
        if( m_pFoundRoutePoint == pAnchorWatchPoint1 ) {
            pAnchorWatchPoint1 = NULL;
            g_AW1GUID.Clear();
        } else if( m_pFoundRoutePoint == pAnchorWatchPoint2 ) {
            pAnchorWatchPoint2 = NULL;
            g_AW2GUID.Clear();
        }

        if( m_pFoundRoutePoint && !( m_pFoundRoutePoint->m_bIsInLayer )
                && ( m_pFoundRoutePoint->GetIconName() != _T("mob") ) ) {

            // If the WP belongs to an invisible route, we come here instead of to ID_RT_MENU_DELPOINT
            //  Check it, and if so then remove the point from its routes
            wxArrayPtrVoid *proute_array = g_pRouteMan->GetRouteArrayContaining( m_pFoundRoutePoint );
            if( proute_array ) {
                pWayPointMan->DestroyWaypoint( m_pFoundRoutePoint );
             }
            else {
                parent->undo->BeforeUndoableAction( Undo_DeleteWaypoint, m_pFoundRoutePoint, Undo_IsOrphanded, NULL/*m_pFoundPoint*/ );
                pConfig->DeleteWayPoint( m_pFoundRoutePoint );
                pSelect->DeleteSelectablePoint( m_pFoundRoutePoint, SELTYPE_ROUTEPOINT );
                if( NULL != pWayPointMan )
                    pWayPointMan->RemoveRoutePoint( m_pFoundRoutePoint );
                parent->undo->AfterUndoableAction( NULL );
            }

            if( pMarkPropDialog ) {
                pMarkPropDialog->SetRoutePoint( NULL );
                pMarkPropDialog->UpdateProperties();
            }

            if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
                pRouteManagerDialog->UpdateWptListCtrl();

            parent->InvalidateGL();
        }
        break;
    }
    case ID_WP_MENU_PROPERTIES:
        parent->ShowMarkPropertiesDialog( m_pFoundRoutePoint );
        break;

    case ID_WP_MENU_CLEAR_ANCHORWATCH:
        if( pAnchorWatchPoint1 == m_pFoundRoutePoint ) {
            pAnchorWatchPoint1 = NULL;
            g_AW1GUID.Clear();
        } else if( pAnchorWatchPoint2 == m_pFoundRoutePoint ) {
            pAnchorWatchPoint2 = NULL;
            g_AW2GUID.Clear();
        }
        break;

    case ID_WP_MENU_SET_ANCHORWATCH:
        if( pAnchorWatchPoint1 == NULL ) {
            pAnchorWatchPoint1 = m_pFoundRoutePoint;
            g_AW1GUID = pAnchorWatchPoint1->m_GUID;
            wxString nn;
            nn = m_pFoundRoutePoint->GetName();
            if( nn.IsNull() ) {
                nn.Printf( _T("%d m"), g_nAWDefault );
                m_pFoundRoutePoint->SetName( nn );
            }
        } else if( pAnchorWatchPoint2 == NULL ) {
            pAnchorWatchPoint2 = m_pFoundRoutePoint;
            g_AW2GUID = pAnchorWatchPoint2->m_GUID;
            wxString nn;
            nn = m_pFoundRoutePoint->GetName();
            if( nn.IsNull() ) {
                nn.Printf( _T("%d m"), g_nAWDefault );
                m_pFoundRoutePoint->SetName( nn );
            }
        }
        break;

    case ID_DEF_MENU_ACTIVATE_MEASURE:
        parent->StartMeasureRoute();
        break;

    case ID_DEF_MENU_DEACTIVATE_MEASURE:
        parent->CancelMeasureRoute();
        gFrame->SurfaceToolbar();
        parent->InvalidateGL();
        parent->Refresh( false );
        break;

#ifdef USE_S57
    case ID_DEF_MENU_CM93OFFSET_DIALOG:
    {
        if( NULL == g_pCM93OffsetDialog ) {
            g_pCM93OffsetDialog = new CM93OffsetDialog( parent->parent_frame );
        }
        
        cm93compchart *pch = NULL;
        if( !parent->GetVP().b_quilt && Current_Ch  && ( Current_Ch->GetChartType() == CHART_TYPE_CM93COMP ) ) {
                pch = (cm93compchart *) Current_Ch;
        }
        
        if( g_pCM93OffsetDialog ){
            g_pCM93OffsetDialog->SetCM93Chart( pch );
            g_pCM93OffsetDialog->Show();
            g_pCM93OffsetDialog->UpdateMCOVRList( parent->GetVP() );
        }

        break;
    }
    case ID_DEF_MENU_QUERY: {
        parent->ShowObjectQueryWindow( popx, popy, zlat, zlon );
        break;
    }
#endif
    case ID_DEF_MENU_AIS_QUERY: {
        wxWindow *pwin = wxDynamicCast(parent, wxWindow);
        ShowAISTargetQueryDialog( pwin, m_FoundAIS_MMSI );
        break;
    }

    case ID_DEF_MENU_AIS_CPA: {             
        AIS_Target_Data *myptarget = g_pAIS->Get_Target_Data_From_MMSI(m_FoundAIS_MMSI); 
        if ( myptarget )                    
            myptarget->Toggle_AIS_CPA();     
        break;                              
    }

    case ID_DEF_MENU_AISSHOWTRACK: {
        AIS_Target_Data *myptarget = g_pAIS->Get_Target_Data_From_MMSI(m_FoundAIS_MMSI);
        if ( myptarget )
            myptarget->ToggleShowTrack();
        break;
    }

    case ID_DEF_MENU_QUILTREMOVE: {
        if( parent->GetVP().b_quilt ) {
            int dbIndex = parent->m_pQuilt->GetChartdbIndexAtPix( parent->GetVP(), wxPoint( popx, popy ) );
            parent->parent_frame->RemoveChartFromQuilt( dbIndex );

            parent->ReloadVP();

        }

        break;
    }

    case ID_DEF_MENU_CURRENTINFO: {
        parent->DrawTCWindow( popx, popy, (void *) m_pIDXCandidate );
        parent->Refresh( false );

        break;
    }

    case ID_DEF_MENU_TIDEINFO: {
        parent->DrawTCWindow( popx, popy, (void *) m_pIDXCandidate );
        parent->Refresh( false );

        break;
    }
    case ID_RT_MENU_REVERSE: {
        if( m_pSelectedRoute->m_bIsInLayer ) break;

        int ask_return = OCPNMessageBox( parent, g_pRouteMan->GetRouteReverseMessage(),
                               _("Rename Waypoints?"), wxYES_NO | wxCANCEL );

        if( ask_return != wxID_CANCEL ) {
            pSelect->DeleteAllSelectableRouteSegments( m_pSelectedRoute );
            m_pSelectedRoute->Reverse( ask_return == wxID_YES );
            pSelect->AddAllSelectableRouteSegments( m_pSelectedRoute );

            pConfig->UpdateRoute( m_pSelectedRoute );

            if( pRoutePropDialog && ( pRoutePropDialog->IsShown() ) ) {
                pRoutePropDialog->SetRouteAndUpdate( m_pSelectedRoute );
                pRoutePropDialog->UpdateProperties();
            }
        }
        break;
    }

    case ID_RT_MENU_DELETE: {
        int dlg_return = wxID_YES;
        if( g_bConfirmObjectDelete ) {
            dlg_return = OCPNMessageBox( parent,  _("Are you sure you want to delete this route?"),
                _("OpenCPN Route Delete"), (long) wxYES_NO | wxCANCEL | wxYES_DEFAULT );
        }

        if( dlg_return == wxID_YES ) {
            if( g_pRouteMan->GetpActiveRoute() == m_pSelectedRoute ) g_pRouteMan->DeactivateRoute();

            if( m_pSelectedRoute->m_bIsInLayer )
                break;

            if( !g_pRouteMan->DeleteRoute( m_pSelectedRoute ) )
                break;
            if( pRoutePropDialog && ( pRoutePropDialog->IsShown()) && (m_pSelectedRoute == pRoutePropDialog->GetRoute()) ) {
                pRoutePropDialog->Hide();
            }

            if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
                pRouteManagerDialog->UpdateRouteListCtrl();

            if( pMarkPropDialog && pMarkPropDialog->IsShown() ) {
                pMarkPropDialog->ValidateMark();
                pMarkPropDialog->UpdateProperties();
            }

            parent->undo->InvalidateUndo();

            parent->InvalidateGL();
        }
        break;
    }

    case ID_RT_MENU_ACTIVATE: {
        if( g_pRouteMan->GetpActiveRoute() )
            g_pRouteMan->DeactivateRoute();

        //  If this is an auto-created MOB route, always select the second point (the MOB)
        // as the destination.
        RoutePoint *best_point;
        if(m_pSelectedRoute){
            if(wxNOT_FOUND == m_pSelectedRoute->m_RouteNameString.Find(_T("MOB")) ){
                best_point = g_pRouteMan->FindBestActivatePoint( m_pSelectedRoute, gLat,
                                 gLon, gCog, gSog );
            }
            else
                best_point = m_pSelectedRoute->GetPoint( 2 );
        
            g_pRouteMan->ActivateRoute( m_pSelectedRoute, best_point );
            m_pSelectedRoute->m_bRtIsSelected = false;
        }
            

        break;
    }

    case ID_RT_MENU_DEACTIVATE:
        g_pRouteMan->DeactivateRoute();
        m_pSelectedRoute->m_bRtIsSelected = false;

        break;

    case ID_RT_MENU_INSERT:

        if( m_pSelectedRoute->m_bIsInLayer ) break;

        m_pSelectedRoute->InsertPointAfter( m_pFoundRoutePoint, zlat, zlon );

        pSelect->DeleteAllSelectableRoutePoints( m_pSelectedRoute );
        pSelect->DeleteAllSelectableRouteSegments( m_pSelectedRoute );

        pSelect->AddAllSelectableRouteSegments( m_pSelectedRoute );
        pSelect->AddAllSelectableRoutePoints( m_pSelectedRoute );

        //    As a special case (which comes up often)...
        //    If the inserted waypoint is on the active leg of an active route
        /*            if(m_pSelectedRoute->m_bRtIsActive)
         {
         if(m_pSelectedRoute->m_nRouteActivePoint == np + 1)
         {
         pNew_Point = m_pSelectedRoute->GetPoint(np + 2);
         pRouteMan->ActivateRoutePoint(m_pSelectedRoute, pNew_Point);
         }
         }
         */
        m_pSelectedRoute->RebuildGUIDList();          // ensure the GUID list is intact and good
        pConfig->UpdateRoute( m_pSelectedRoute );

        if( pRoutePropDialog && ( pRoutePropDialog->IsShown() ) ) {
            pRoutePropDialog->SetRouteAndUpdate( m_pSelectedRoute, true );
        }

        break;

    case ID_RT_MENU_APPEND:

        if( m_pSelectedRoute->m_bIsInLayer ) break;

        parent->m_pMouseRoute = m_pSelectedRoute;
        parent->parent_frame->nRoute_State = m_pSelectedRoute->GetnPoints() + 1;
        parent->m_pMouseRoute->m_lastMousePointIndex = m_pSelectedRoute->GetnPoints();
        parent->m_pMouseRoute->SetHiLite(50);
        
        pLast = m_pSelectedRoute->GetLastPoint();

        parent->m_prev_rlat = pLast->m_lat;
        parent->m_prev_rlon = pLast->m_lon;
        parent->m_prev_pMousePoint = pLast;

        parent->m_bAppendingRoute = true;

        parent->SetCursor( *parent->pCursorPencil );
        #ifdef __OCPN__ANDROID__
        androidSetRouteAnnunciator( true );
        #endif        
        
        break;

    case ID_RT_MENU_COPY:
        if( m_pSelectedRoute ) Kml::CopyRouteToClipboard( m_pSelectedRoute );
        break;

    case ID_TK_MENU_COPY:
        if( m_pSelectedTrack ) Kml::CopyTrackToClipboard( m_pSelectedTrack );
        break;

    case ID_WPT_MENU_COPY:
        if( m_pFoundRoutePoint ) Kml::CopyWaypointToClipboard( m_pFoundRoutePoint );
        break;

    case ID_WPT_MENU_SENDTOGPS:
        if( m_pFoundRoutePoint ) {
            if( parent->m_active_upload_port.Length() )
                m_pFoundRoutePoint->SendToGPS( parent->m_active_upload_port.BeforeFirst(' '), NULL );
             else {
                 SendToGpsDlg dlg;
                 dlg.SetWaypoint( m_pFoundRoutePoint );

                 dlg.Create( NULL, -1, _( "Send To GPS..." ), _T("") );
                 dlg.ShowModal();
             }
        }
        break;

    case ID_WPT_MENU_SENDTONEWGPS:
        if( m_pFoundRoutePoint ) {
            SendToGpsDlg dlg;
            dlg.SetWaypoint( m_pFoundRoutePoint );
                
            dlg.Create( NULL, -1, _( "Send To GPS..." ), _T("") );
            dlg.ShowModal();
        }
        break;
        
    case ID_RT_MENU_SENDTOGPS:
        if( m_pSelectedRoute ) {
            if( parent->m_active_upload_port.Length() )
                m_pSelectedRoute->SendToGPS( parent->m_active_upload_port.BeforeFirst(' '), true, NULL );
            else {
                SendToGpsDlg dlg;
                dlg.SetRoute( m_pSelectedRoute );

                dlg.Create( NULL, -1, _( "Send To GPS..." ), _T("") );
                dlg.ShowModal();
            }

        }
        break;

    case ID_RT_MENU_SENDTONEWGPS:
        if( m_pSelectedRoute ) {
            SendToGpsDlg dlg;
            dlg.SetRoute( m_pSelectedRoute );
                
            dlg.Create( NULL, -1, _( "Send To GPS..." ), _T("") );
            dlg.ShowModal();
        }
        break;
        
    case ID_PASTE_WAYPOINT:
        pupHandler_PasteWaypoint();
        break;

    case ID_PASTE_ROUTE:
        pupHandler_PasteRoute();
        break;

    case ID_PASTE_TRACK:
        pupHandler_PasteTrack();
        break;

    case ID_RT_MENU_DELPOINT:
        if( m_pSelectedRoute ) {
            if( m_pSelectedRoute->m_bIsInLayer ) break;

            pWayPointMan->DestroyWaypoint( m_pFoundRoutePoint );

            if( pRoutePropDialog && ( pRoutePropDialog->IsShown() ) ) {
                //    Selected route may have been deleted as one-point route, so check it
                if( g_pRouteMan->IsRouteValid( m_pSelectedRoute ) ){        
                    pRoutePropDialog->SetRouteAndUpdate( m_pSelectedRoute, true );
                }
                else
                    pRoutePropDialog->Hide();

            }

            if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) {
                pRouteManagerDialog->UpdateWptListCtrl();
                pRouteManagerDialog->UpdateRouteListCtrl();
            }

            parent->InvalidateGL();
        }

        break;

    case ID_RT_MENU_REMPOINT:
        if( m_pSelectedRoute ) {
            if( m_pSelectedRoute->m_bIsInLayer ) break;
            parent->RemovePointFromRoute( m_pFoundRoutePoint, m_pSelectedRoute );
            parent->InvalidateGL();
        }
        break;

    case ID_RT_MENU_ACTPOINT:
        if( g_pRouteMan->GetpActiveRoute() == m_pSelectedRoute ) {
            g_pRouteMan->ActivateRoutePoint( m_pSelectedRoute, m_pFoundRoutePoint );
            m_pSelectedRoute->m_bRtIsSelected = false;
        }

        break;

    case ID_RT_MENU_DEACTPOINT:
        break;

    case ID_RT_MENU_ACTNXTPOINT:
        if( g_pRouteMan->GetpActiveRoute() == m_pSelectedRoute ) {
            g_pRouteMan->ActivateNextPoint( m_pSelectedRoute, true );
            m_pSelectedRoute->m_bRtIsSelected = false;
        }

        break;

    case ID_RT_MENU_PROPERTIES: {
        parent->ShowRoutePropertiesDialog( _("Route Properties"), m_pSelectedRoute );
        break;
    }

    case ID_TK_MENU_PROPERTIES: {
        parent->ShowTrackPropertiesDialog( m_pSelectedTrack );
        break;
    }

    case ID_TK_MENU_DELETE: {
        int dlg_return = wxID_YES;
        if( g_bConfirmObjectDelete ) {
            dlg_return = OCPNMessageBox( parent, _("Are you sure you want to delete this track?"),
                _("OpenCPN Track Delete"), (long) wxYES_NO | wxCANCEL | wxYES_DEFAULT );
        }

        if( dlg_return == wxID_YES ) {

            if( m_pSelectedTrack == g_pActiveTrack )
                parent->parent_frame->TrackOff();
            g_pAIS->DeletePersistentTrack( m_pSelectedTrack );
            pConfig->DeleteConfigTrack( m_pSelectedTrack );
            g_pRouteMan->DeleteTrack( m_pSelectedTrack );

            if( pTrackPropDialog && ( pTrackPropDialog->IsShown()) && (m_pSelectedTrack == pTrackPropDialog->GetTrack()) ) {
                pTrackPropDialog->Hide();
            }

            if( pRouteManagerDialog && pRouteManagerDialog->IsShown() ) {
                pRouteManagerDialog->UpdateTrkListCtrl();
                pRouteManagerDialog->UpdateRouteListCtrl();
            }
            parent->InvalidateGL();
        }
        break;
    }

    case ID_RC_MENU_SCALE_IN:
        parent->parent_frame->DoStackDown();
        parent->GetCanvasPointPix( zlat, zlon, &r );
        parent->WarpPointer( r.x, r.y );
        break;

    case ID_RC_MENU_SCALE_OUT:
        parent->parent_frame->DoStackUp();
        parent->GetCanvasPointPix( zlat, zlon, &r );
        parent->WarpPointer( r.x, r.y );
        break;

    case ID_RC_MENU_ZOOM_IN:
        parent->SetVPScale( parent->GetVPScale() * 2 );
        parent->GetCanvasPointPix( zlat, zlon, &r );
        parent->WarpPointer( r.x, r.y );
        break;

    case ID_RC_MENU_ZOOM_OUT:
        parent->SetVPScale( parent->GetVPScale() / 2 );
        parent->GetCanvasPointPix( zlat, zlon, &r );
        parent->WarpPointer( r.x, r.y );
        break;

    case ID_RC_MENU_FINISH:
        parent->FinishRoute();
        gFrame->SurfaceToolbar();
        parent->Refresh( false );
        break;

    case ID_DEF_ZERO_XTE:
        g_pRouteMan->ZeroCurrentXTEToActivePoint();
        break;

    default: {
        //  Look for PlugIn Context Menu selections
        //  If found, make the callback
        ArrayOfPlugInMenuItems item_array = g_pi_manager->GetPluginContextMenuItemArray();

        for( unsigned int i = 0; i < item_array.GetCount(); i++ ) {
            PlugInMenuItemContainer *pimis = item_array.Item( i );
            {
                if( pimis->id == event.GetId() ) {
                    if( pimis->m_pplugin )
                        pimis->m_pplugin->OnContextMenuItemCallback( pimis->id );
                }
            }
        }

        break;
    }
    }           // switch

    //  Chart Groups....
    if( ( event.GetId() >= ID_DEF_MENU_GROUPBASE )
            && ( event.GetId() <= ID_DEF_MENU_GROUPBASE + (int) g_pGroupArray->GetCount() ) ) {
        gFrame->SetGroupIndex( event.GetId() - ID_DEF_MENU_GROUPBASE );
    }

    parent->InvalidateGL();
    
    g_click_stop = 0;    // Context menu was processed, all is well

}

