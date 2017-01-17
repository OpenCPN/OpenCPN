/******************************************************************************
 * Project:  OpenCPN
 * Purpose:  Squiddio plugin
 *
 ***************************************************************************
 *   Copyright (C) 2014 by Mauro Calvi                                     *
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

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include "squiddio_pi.h"
#include "logs.h"
#include <wx/fileconf.h>

WX_DEFINE_LIST (LayerList);
WX_DEFINE_LIST (HyperlinkList);
WX_DEFINE_LIST (Plugin_HyperlinkList);

#ifdef __OCPN__ANDROID__
#include "qdebug.h"
#endif

#ifdef __OCPN__ANDROID__

QString qtStyleSheet = "QScrollBar:horizontal {\
border: 0px solid grey;\
background-color: rgb(240, 240, 240);\
height: 35px;\
margin: 0px 1px 0 1px;\
}\
QScrollBar::handle:horizontal {\
background-color: rgb(200, 200, 200);\
min-width: 20px;\
border-radius: 10px;\
}\
QScrollBar::add-line:horizontal {\
border: 0px solid grey;\
background: #32CC99;\
width: 0px;\
subcontrol-position: right;\
subcontrol-origin: margin;\
}\
QScrollBar::sub-line:horizontal {\
border: 0px solid grey;\
background: #32CC99;\
width: 0px;\
subcontrol-position: left;\
subcontrol-origin: margin;\
}\
QScrollBar:vertical {\
border: 0px solid grey;\
background-color: rgb(240, 240, 240);\
width: 35px;\
margin: 1px 0px 1px 0px;\
}\
QScrollBar::handle:vertical {\
background-color: rgb(200, 200, 200);\
min-height: 20px;\
border-radius: 10px;\
}\
QScrollBar::add-line:vertical {\
border: 0px solid grey;\
background: #32CC99;\
height: 0px;\
subcontrol-position: top;\
subcontrol-origin: margin;\
}\
QScrollBar::sub-line:vertical {\
border: 0px solid grey;\
background: #32CC99;\
height: 0px;\
subcontrol-position: bottom;\
subcontrol-origin: margin;\
}\
QCheckBox {\
spacing: 25px;\
}\
QCheckBox::indicator {\
width: 30px;\
height: 30px;\
}\
";

#endif





// the class factories, used to create and destroy instances of the PlugIn
//


// these variables are shared with NavObjectCollection
PoiMan *pPoiMan;
class logsWindow;

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr) {
    return new squiddio_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p) {
    delete p;
}

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

int period_secs(int period) {
    switch (period) {
    case 0: return 0;
    case 1: return 85400;
    case 2: return 43200;
    case 3: return 3600;
    case 4: return 1800;
    case 5: return 60;
    case 6: return 30;
    case 7: return 10;
    default: break;
    }
    return 0;
}

BEGIN_EVENT_TABLE( squiddio_pi, wxEvtHandler )
    EVT_MYEVENT( squiddio_pi::OnThreadActionFinished )
END_EVENT_TABLE()

squiddio_pi::squiddio_pi(void *ppimgr) :
        opencpn_plugin_113(ppimgr) // constructor initialization
{
    // Create the PlugIn icons
    initialize_images();
    SetThreadRunning(false);
}

squiddio_pi::~squiddio_pi(void) {
    delete _img_marina_grn;
    delete _img_anchor_blu;
    delete _img_aton_gry;
    delete _img_club_pur;
    delete _img_fuelpump_red;
    delete _img_pier_yel;
    delete _img_ramp_azu;
    delete _img_logimg_N;
    delete _img_logimg_NE;
    delete _img_logimg_E;
    delete _img_logimg_SE;
    delete _img_logimg_S;
    delete _img_logimg_SW;
    delete _img_logimg_W;
    delete _img_logimg_NW;
    delete _img_logimg_C;
    delete _img_logimg_U;

}

int squiddio_pi::Init(void) {

    wxLogMessage(_T("squiddio_pi: Init()"));

    AddLocaleCatalog( _T("opencpn-squiddio_pi") );
    
    m_plogs_window = NULL;
    g_PostPeriod = 0;
    g_RetrievePeriod = 0;
    
    // Get a pointer to the opencpn display canvas, to use as a parent for windows created
    m_parent_window = GetOCPNCanvasWindow();

    last_online_chk = 0;
    last_online =false;

    wxMenu dummy_menu;

    wxMenuItem *pmi = new wxMenuItem(&dummy_menu, -1,
            _("Show local sQuiddio destinations"));
    m_show_id = AddCanvasContextMenuItem(pmi, this);
    SetCanvasContextMenuItemViz(m_show_id, false);

    wxMenuItem *pmih = new wxMenuItem(&dummy_menu, -1,
            _("Hide local sQuiddio destinations"));
    m_hide_id = AddCanvasContextMenuItem(pmih, this);
    SetCanvasContextMenuItemViz(m_hide_id, false);

    wxMenuItem *updi = new wxMenuItem(&dummy_menu, -1,
            _("Download local sQuiddio destinations"));
    m_update_id = AddCanvasContextMenuItem(updi, this);
    SetCanvasContextMenuItemViz(m_update_id, true);

    wxMenuItem *repi = new wxMenuItem(&dummy_menu, -1,
            _("Report a Destination at this location"));
    m_report_id = AddCanvasContextMenuItem(repi, this);
    SetCanvasContextMenuItemViz(m_report_id, true);

    AddCustomWaypointIcon(_img_marina_grn, _T("marina_grn"), _T("Marina"));
    AddCustomWaypointIcon(_img_anchor_blu, _T("anchor_blu"),
            _T("Anchorage/Buoys"));
    AddCustomWaypointIcon(_img_aton_gry, _T("aton_gry"),
            _T("AIS ATON Marker"));
    AddCustomWaypointIcon(_img_club_pur, _T("club_pur"), _T("Yacht Club"));
    AddCustomWaypointIcon(_img_fuelpump_red, _T("fuelpump_red"),
            _T("Fuel Station"));
    AddCustomWaypointIcon(_img_pier_yel, _T("pier_yel"), _T("Dock/Pier"));
    AddCustomWaypointIcon(_img_ramp_azu, _T("ramp_azu"), _T("Boat Ramp"));

    AddCustomWaypointIcon(_img_logimg_N, _T("logimg_N"), _T("North"));
    AddCustomWaypointIcon(_img_logimg_NE, _T("logimg_NE"), _T("North East"));
    AddCustomWaypointIcon(_img_logimg_E, _T("logimg_E"), _T("East"));
    AddCustomWaypointIcon(_img_logimg_SE, _T("logimg_SE"), _T("South East"));
    AddCustomWaypointIcon(_img_logimg_S, _T("logimg_S"), _T("South"));
    AddCustomWaypointIcon(_img_logimg_SW, _T("logimg_SW"), _T("South West"));
    AddCustomWaypointIcon(_img_logimg_W, _T("logimg_W"), _T("West"));
    AddCustomWaypointIcon(_img_logimg_NW, _T("logimg_NW"), _T("North West"));
    AddCustomWaypointIcon(_img_logimg_C, _T("logimg_C"), _T("Checked in"));
    AddCustomWaypointIcon(_img_logimg_U, _T("logimg_U"), _T("Unknown heading"));

    pLayerList = new LayerList;
    pPoiMan = new PoiMan;
    link = new Plugin_Hyperlink;

    m_pconfig = GetOCPNConfigObject();
    LoadConfig();

    layerdir = *GetpPrivateApplicationDataLocation();
    layerdir += wxFileName::GetPathSeparator();
    layerdir += _T("squiddio");

    if (!wxDir::Exists(layerdir))
        wxFileName::Mkdir(layerdir);

    if (wxDir::Exists(layerdir)) {
        wxString laymsg;
        laymsg.Printf(wxT("squiddio_pi: getting .gpx layer files from: %s"),
                layerdir.c_str());
        wxLogMessage(laymsg);

        LoadLayers(layerdir);

        Layer * l;
        LayerList::iterator it;
        int index = 0;
        for (it = (*pLayerList).begin(); it != (*pLayerList).end();
                ++it, ++index) {
            l = (Layer *) (*it);
            l->SetVisibleNames(false);
            RenderLayerContentsOnChart(l);
        }
    }

    //    This PlugIn needs a toolbar icon, so request its insertion
    m_leftclick_tool_id = InsertPlugInTool(_T(""), _img_plugin_logo,
            _img_plugin_logo, wxITEM_NORMAL, _("sQuiddio"), _T(""), NULL,
            SQUIDDIO_TOOL_POSITION, 0, this);
            
    m_pThread = new SquiddioThread(this);
    wxThreadError err = m_pThread->Run();

    if ( err != wxTHREAD_NO_ERROR )
    {
        delete m_pThread;
        m_pThread = NULL;
    }

    return (
    INSTALLS_CONTEXTMENU_ITEMS |
    WANTS_CURSOR_LATLON |
    WANTS_NMEA_SENTENCES |
    WANTS_PREFERENCES |
    USES_AUI_MANAGER |
    WANTS_CONFIG |
    WANTS_TOOLBAR_CALLBACK |
    INSTALLS_TOOLBAR_TOOL |
    WANTS_LATE_INIT
    );
}

bool squiddio_pi::DeInit(void) {
    RemovePlugInTool(m_leftclick_tool_id);

    if (m_plogs_window) {
        m_AUImgr->DetachPane(m_plogs_window);
        m_plogs_window->Close();
    }

    RemoveCanvasContextMenuItem(m_show_id);
    RemoveCanvasContextMenuItem(m_hide_id);
    RemoveCanvasContextMenuItem(m_update_id);
    RemoveCanvasContextMenuItem(m_report_id);

    LayerList::iterator it;
    int index = 0;
    for (it = (*pLayerList).begin(); it != (*pLayerList).end(); ++it, ++index) {
        Layer * l = (Layer *) (*it);
        pLayerList->DeleteObject(l);
    }
    SaveConfig();
    RequestRefresh(m_parent_window);
    
    {
        wxCriticalSectionLocker enter(m_pThreadCS);
        if (m_pThread) // does the thread still exist?
        {
            while (m_pThread->IsWorking())
            {
                wxMilliSleep(10);
            }
            if (m_pThread->Delete() != wxTHREAD_NO_ERROR )
                wxLogError(_T("Can't delete the thread!"));
        }
    } // exit from the critical section to give the thread
        // the possibility to enter its destructor
        // (which is guarded with m_pThreadCS critical section!)
    while (1)
    {
        { // was the ~MyThread() function executed?
            wxCriticalSectionLocker enter(m_pThreadCS);
            if (!m_pThread)
                break;
        }
        // wait for thread completion
    }
    
    //Last resort check for thread completion, wait if it looks bad
    #define THREAD_WAIT_SECONDS  5
    //  Try to wait a bit to see if all compression threads exit nicely
    wxDateTime now = wxDateTime::Now();
    time_t stall = now.GetTicks();
    time_t end = stall + THREAD_WAIT_SECONDS;
    
    while(IsThreadRunning() && stall < end ){
        wxDateTime later = wxDateTime::Now();
        stall = later.GetTicks();
        
        wxYield();
        wxSleep(1);
        if( !IsThreadRunning() )
            break;
    }

    delete pLayerList;
    delete pPoiMan;
    delete link;
    delete m_plogs_window;
    return true;
}
bool squiddio_pi::LoadConfig(void) {
    wxFileConfig *pConf = (wxFileConfig *) m_pconfig;

    if (!pConf)
        return false;

    pConf->SetPath(_T("/PlugIns/libsquiddio_pi.so"));
    pConf->Read(_T("VisibleSqLayers"), &g_VisibleLayers);
    pConf->Read(_T("InvisibleSqLayers"), &g_InvisibleLayers);
    pConf->Read(_T("PostPeriod"), &g_PostPeriod);
    pConf->Read(_T("RetrievePeriod"), &g_RetrievePeriod);
    pConf->Read(_T("LastLogSent"), &g_LastLogSent);
    pConf->Read(_T("LastLogsRcvd"), &g_LastLogsRcvd);
    pConf->Read(_T("Email"), &g_Email);
    pConf->Read(_T("ApiKey"), &g_ApiKey);
    pConf->Read(_T("ViewMarinas"), &g_ViewMarinas, true);
    pConf->Read(_T("ViewAnchorages"), &g_ViewAnchorages, true);
    pConf->Read(_T("ViewDocks"), &g_ViewDocks, true);
    pConf->Read(_T("ViewYachtClubs"), &g_ViewYachtClubs, true);
    pConf->Read(_T("ViewFuelStations"), &g_ViewFuelStations, true);
    pConf->Read(_T("ViewRamps"), &g_ViewRamps, true);
    pConf->Read(_T("ViewOthers"), &g_ViewOthers, true);

    return true;
}

bool squiddio_pi::SaveConfig(void) {
    wxFileConfig *pConf = (wxFileConfig *) m_pconfig;

    if (!pConf)
        return false;

    pConf->SetPath(_T("/PlugIns/libsquiddio_pi.so"));
    pConf->Write(_T("VisibleSqLayers"), g_VisibleLayers);
    pConf->Write(_T("InvisibleSqLayers"), g_InvisibleLayers);
    pConf->Write(_T("PostPeriod"), g_PostPeriod);
    pConf->Write(_T("RetrievePeriod"), g_RetrievePeriod);
    pConf->Write(_T("LastLogSent"), g_LastLogSent);
    pConf->Write(_T("LastLogsRcvd"), g_LastLogsRcvd);
    pConf->Write(_T("Email"), g_Email);
    pConf->Write(_T("ApiKey"), g_ApiKey);
    pConf->Write(_T("ViewMarinas"), g_ViewMarinas);
    pConf->Write(_T("ViewAnchorages"), g_ViewAnchorages);
    pConf->Write(_T("ViewDocks"), g_ViewDocks);
    pConf->Write(_T("ViewYachtClubs"), g_ViewYachtClubs);
    pConf->Write(_T("ViewRamps"), g_ViewRamps);
    pConf->Write(_T("ViewFuelStations"), g_ViewFuelStations);
    pConf->Write(_T("ViewOthers"), g_ViewOthers);

    return true;
}

bool squiddio_pi::LoadLayers(wxString &path) {
    wxArrayString file_array;
    wxDir dir;
    Layer *l;
    dir.Open(path);
    if (dir.IsOpened()) {
        wxString filename;
        bool cont = dir.GetFirst(&filename);
        while (cont) {
            file_array.Clear();
            filename.Prepend(wxFileName::GetPathSeparator());
            filename.Prepend(path);
            wxFileName f(filename);
            size_t nfiles = 0;
            if (f.GetExt().IsSameAs(wxT("gpx")))
                file_array.Add(filename); // single-gpx-file layer
            else {
                wxDir dir(filename);
                if (dir.IsOpened()) {
                    nfiles = dir.GetAllFiles(filename, &file_array,
                            wxT("*.gpx"));      // layers subdirectory set
                }
            }

            if (file_array.GetCount()) {
                l = new Layer();
                l->m_LayerID = ++g_LayerIdx;
                l->m_LayerFileName = file_array[0];
                if (file_array.GetCount() <= 1)
                    wxFileName::SplitPath(file_array[0], NULL, NULL,
                            &(l->m_LayerName), NULL, NULL);
                else
                    wxFileName::SplitPath(filename, NULL, NULL,
                            &(l->m_LayerName), NULL, NULL);

                bool bLayerViz = false;

                if ((g_VisibleLayers.Contains(l->m_LayerName)) || (l->m_LayerName.Contains(_T("logs")) && g_RetrievePeriod > 0))
                    bLayerViz = true;

                l->m_bIsVisibleOnChart = bLayerViz;

                wxString laymsg;
                laymsg.Printf(wxT("squiddio_pi: new layer %d: %s"),
                        l->m_LayerID, l->m_LayerName.c_str());
                wxLogMessage(laymsg);

                pLayerList->Insert(l);

                //  Load the entire file array as a single layer
                for (unsigned int i = 0; i < file_array.GetCount(); i++) {
                    wxString file_path = file_array[i];
                    if (::wxFileExists(file_path)) {
                        LoadLayerItems(file_path, l, bLayerViz);
                    }
                //delete l;
                }
            }
            cont = dir.GetNext(&filename);
        }
    }
    return true;

}

bool squiddio_pi::LoadLayerItems(wxString &file_path, Layer *l, bool show) {
    NavObjectCollection1 *pSet = new NavObjectCollection1;
    pSet->load_file(file_path.fn_str());
    long nItems = pSet->LoadAllGPXObjectsAsLayer(l->m_LayerID, show);

    l->m_NoOfItems += nItems;

    wxString objmsg;
    objmsg.Printf(wxT("squiddio_pi: loaded GPX file %s with %d items."),
            file_path.c_str(), nItems);
    wxLogMessage(objmsg);
    delete pSet;
    return nItems > 0;
}

Layer * squiddio_pi::LoadLayer(wxString file_path, wxString region) {
    Layer * l = new Layer();
    if (::wxFileExists(file_path)) {

        l->m_LayerID = ++g_LayerIdx;
        if (file_path.Contains(_T("logs.gpx"))) {
            l->m_LayerName = _T("logs");
        } else {
            l->m_LayerName = _T("SQ_") + region;
        }
        l->m_LayerFileName = file_path;
        l->m_bIsVisibleOnChart = true;
        pLayerList->Insert(l);

        LoadLayerItems(file_path, l, true);
    }
    return l;
}

bool squiddio_pi::ShowType(Poi * wp) {
    if (wp->m_IconName == _T("marina_grn"))
        return g_ViewMarinas;
    else if (wp->m_IconName == _T("anchor_blu"))
        return g_ViewAnchorages;
    else if (wp->m_IconName == _T("club_pur"))
        return g_ViewYachtClubs;
    else if (wp->m_IconName == _T("fuelpump_red"))
        return g_ViewFuelStations;
    else if (wp->m_IconName == _T("pier_yel"))
        return g_ViewDocks;
    else if (wp->m_IconName == _T("ramp_azu"))
        return g_ViewRamps;
    else if (wp->m_IconName == _T("others"))
        return g_ViewOthers;
    else if (wp->m_IconName == _T("aton_gry"))
        return g_ViewAIS;
    else
        return true;
}

void squiddio_pi::RenderLayers() {
    Layer * l;
    LayerList::iterator it;
    int index = 0;
    for (it = (*pLayerList).begin(); it != (*pLayerList).end(); ++it, ++index) {
        l = (Layer *) (*it);
        l->SetVisibleNames(false);
        RenderLayerContentsOnChart(l);
    }
}

void squiddio_pi::RenderLayerContentsOnChart(Layer *layer, bool save_config) {

    // Process POIs in this layer
    //if (layer->m_LayerName.Contains(_T("logs")) && g_RetrievePeriod ==0 )
    //    return;

    wxPoiListNode *node = pPoiMan->GetWaypointList()->GetFirst();

    while (node) {
        Poi *rp = node->GetData();
        if (rp && (rp->m_LayerID == layer->m_LayerID)) {
            rp->SetVisible(layer->IsVisibleOnChart());
            rp->SetNameShown(false);
            if (layer->IsVisibleOnChart() && ShowType(rp)  )
                ShowPOI(rp);
            else
                HidePOI(rp);
        }
        node = node->GetNext();
    }

    if (layer->IsVisibleOnChart()) {
        if (!g_VisibleLayers.Contains(layer->m_LayerName))
            g_VisibleLayers.Append(layer->m_LayerName + _T(";"));
        g_InvisibleLayers.Replace(layer->m_LayerName + _T(";"), wxEmptyString);
    } else {
        if (!g_InvisibleLayers.Contains(layer->m_LayerName))
            g_InvisibleLayers.Append(layer->m_LayerName + _T(";"));
        g_VisibleLayers.Replace(layer->m_LayerName + _T(";"), wxEmptyString);
    }
    RequestRefresh(m_parent_window);
    if (save_config)
        SaveConfig();
}
bool squiddio_pi::ShowPOI(Poi * wp) {
    double lat = wp->m_lat;
    double lon = wp->m_lon;
    wxString name = wp->GetName();
    wxString m_GUID = wp->m_GUID;
    wxString m_iconname = wp->m_IconName;

    PlugIn_Waypoint * pPoint = new PlugIn_Waypoint(lat, lon, m_iconname, name,
            m_GUID);
    pPoint->m_MarkDescription = wp->m_MarkDescription;

    wxHyperlinkListNode *linknode = wp->m_HyperlinkList->GetFirst();
    wp_link = linknode->GetData();
    link->Link = wp_link->Link;
    link->DescrText = wp_link->DescrText;
    link->Type = wxEmptyString;

    pPoint->m_HyperlinkList = new Plugin_HyperlinkList;
    pPoint->m_HyperlinkList->Insert(link);

    bool added = AddSingleWaypoint(pPoint, false);
    return added;
}

bool squiddio_pi::HidePOI(Poi * wp) {
    return DeleteSingleWaypoint(wp->m_GUID);
}

void squiddio_pi::UpdateAuiStatus(void) {
    //    This method is called after the PlugIn is initialized
    //    and the frame has done its initial layout, possibly from a saved wxAuiManager "Perspective"
    //    It is a chance for the PlugIn to syncronize itself internally with the state of any Panes that
    //    were added to the frame in the PlugIn ctor.

    //    We use this callback here to keep the context menu selection in sync with the window state
    //SetCanvasContextMenuItemViz(m_hide_id, false);
    //SetCanvasContextMenuItemViz(m_show_id, false);

    //IsOnline(); //sets last_online boolean, resets last_online_chk time and sQuiddio options in contextual menu
    //SetLogsWindow();

}
void squiddio_pi::LateInit(void){
    SetCanvasContextMenuItemViz(m_hide_id, false);
    SetCanvasContextMenuItemViz(m_show_id, false);

//    CheckIsOnline(); //sets last_online boolean, resets last_online_chk time and sQuiddio options in contextual menu
    SetLogsWindow();
}
void squiddio_pi::SetCursorLatLon(double lat, double lon) {
    m_cursor_lon = lon;
    m_cursor_lat = lat;

    int lat_sector = abs((int)m_cursor_lat / 5);
    int lon_sector = abs((int)m_cursor_lon / 5);
    wxString lat_quadrant = (m_cursor_lat > 0) ? (_T("N")) : (_T("S"));
    wxString lon_quadrant = (m_cursor_lon > 0) ? (_T("E")) : (_T("W"));
    local_region = lat_quadrant << wxString::Format(wxT("%02i"), lat_sector)
            << lon_quadrant << wxString::Format(wxT("%02i"), lon_sector);

    local_sq_layer = GetLocalLayer();
    if (local_sq_layer != NULL) {
        SetCanvasContextMenuItemViz(m_hide_id,
                local_sq_layer->IsVisibleOnChart());
        SetCanvasContextMenuItemViz(m_show_id,
                !local_sq_layer->IsVisibleOnChart());
    } else {
        SetCanvasContextMenuItemViz(m_hide_id, false);
        SetCanvasContextMenuItemViz(m_show_id, false);
    }
}

void squiddio_pi::RefreshLayer()
{
    wxString layerContents;
    Layer * new_layer = NULL;
    wxString versionMajor = wxString::Format(wxT("%i"),PLUGIN_VERSION_MAJOR);
    wxString versionMinor = wxString::Format(wxT("%i"),PLUGIN_VERSION_MINOR);

    //version << wxString::Format(wxT("%i"),PLUGIN_VERSION_MINOR);

    if (CheckIsOnline()){
        layerContents = DownloadLayer(
        _T("/places/download_xml_layers.xml?version=")+versionMajor+versionMinor+_T("&region=")
        + m_rgn_to_dld);
    }
    else{
        wxMessageBox( _("Squiddio_pi download failed.\nDevice is offline."),
                        _("Squiddio PlugIn"), wxOK | wxICON_ERROR );
        
        return;
    }

    wxString gpxFilePath = layerdir;
    appendOSDirSlash(&gpxFilePath);
    gpxFilePath.Append(_T("SQ_") + m_rgn_to_dld + _T(".gpx"));

    if (layerContents.length() != 0) {
        if (layerContents.length() > 400) {
            isLayerUpdate = SaveLayer(layerContents, gpxFilePath);
            new_layer = LoadLayer(gpxFilePath, m_rgn_to_dld);
            new_layer->SetVisibleNames(false);
            RenderLayerContentsOnChart(new_layer, true);

            if (isLayerUpdate) {
                wxLogMessage( _("Local destinations have been updated") );
            }
        } else {
            wxLogMessage( _("No destinations available for the region") );
        }
    } else {
        wxLogMessage( _("Server not responding. Check your Internet connection") );
    }
}

void squiddio_pi::OnContextMenuItemCallback(int id) {
    //wxLogMessage(_T("squiddio_pi: OnContextMenuCallBack()"));

    if (id == m_show_id || id == m_hide_id) {
        local_sq_layer->SetVisibleOnChart(!local_sq_layer->IsVisibleOnChart());
        RenderLayerContentsOnChart(local_sq_layer, true);
        wxLogMessage(
                _T("squiddio_pi: toggled layer: ")
                        + local_sq_layer->m_LayerName);
    } else if (id == m_update_id) {
        if (local_sq_layer != NULL) {
            // hide and delete the current layer
            local_sq_layer->SetVisibleOnChart(false);
            RenderLayerContentsOnChart(local_sq_layer, true);
            pLayerList->DeleteObject(local_sq_layer);
        }
        m_rgn_to_dld = local_region;
        RefreshLayer();
    } else if (id == m_report_id) {
        wxString url_path = _T("http://squidd.io/locations/new?lat=");
        url_path.Append(
                wxString::Format(wxT("%f"), m_cursor_lat) << _T("&lon=")
                        << wxString::Format(wxT("%f"), m_cursor_lon));
        
        if (!CheckIsOnline() || !LaunchDefaultBrowser_Plugin(url_path))
            wxMessageBox( _("Squiddio_pi could not launch default browser.\n Check your Internet connection") );
    }
}

wxString squiddio_pi::DownloadLayer(wxString url_path) {
    wxString res = wxEmptyString;
    //size_t result = http.Get( response, _T("https://squidd.io") + url_path );
    //size_t result = http.Get( response, _T("http://squidd.io") + url_path );

    // Find a known writable location for a temporary file
    wxString wdir = *GetpPrivateApplicationDataLocation();
    wxChar sep = wxFileName::GetPathSeparator();
    if( wdir.Last() != sep ) wdir.Append( sep );

    wxDateTime now = wxDateTime::Now();
    wxString fn = wdir + _T("squiddio_pi_") + now.Format(_T("%j%H%M%S"));         //  somewhat unique...
    
    _OCPN_DLStatus result = OCPN_downloadFile( _T("http://squidd.io") + url_path, fn, _("Downloading"), _("Downloading: "), wxNullBitmap, m_parent_window, OCPN_DLDS_ELAPSED_TIME|OCPN_DLDS_AUTO_CLOSE|OCPN_DLDS_SIZE|OCPN_DLDS_SPEED|OCPN_DLDS_REMAINING_TIME, 10 );

    if( result == OCPN_DL_NO_ERROR )
    {
        if(wxFileExists(fn)){
             wxFile f( fn );
             f.ReadAll( &res );
             f.Close();
         }
    }
    else
    {
        wxMessageBox( _("Squiddio_pi download failed.\nVerify there is a working Internet connection.\nRepeat the operation if needed."),
                                          _("Squiddio PlugIn"), wxOK | wxICON_ERROR );
        
        wxLogMessage(_("Squiddio_pi: unable to connect to host"));
    }

    if(wxFileExists(fn))
        wxRemoveFile( fn );
        
    if(res.Length() < 20){
        wxMessageBox( _("Squiddio_pi download failed with short or corrupt file.\nRepeat the operation if needed."),
                      _("Squiddio PlugIn"), wxOK | wxICON_ERROR );
    }
    return res;
}

bool squiddio_pi::SaveLayer(wxString layerStr, wxString file_path) {
    // write file to the squiddio directory
    bool isUpdate = wxFile::Exists(file_path);
    wxFile gpxFile;

    if (isUpdate)
        wxLogMessage(_T("squiddio_pi: updating existing layer file"));
    else
        wxLogMessage(_T("squiddio_pi: creating new layer file"));

    if (gpxFile.Create(file_path, true)) {
        gpxFile.Write(layerStr);
        gpxFile.Close();
    } else
        wxLogMessage(_T("squiddio_pi: unable to create layer file"));
    return isUpdate;
}

bool squiddio_pi::CheckIsOnline()
{
    return OCPN_isOnline();
}

Layer * squiddio_pi::GetLocalLayer() {
    LayerList::iterator it;
    int index = 0;
    wxString layer_name = _T("SQ_") + local_region;
    Layer *lay = NULL;

    for (it = (*pLayerList).begin(); it != (*pLayerList).end(); ++it, ++index) {
        Layer * l = (Layer *) (*it);
        if (l->m_LayerName == layer_name) {
            lay = l;
            break;
        }
    }
    return lay;
}

void squiddio_pi::appendOSDirSlash(wxString* pString) {
    wxChar sep = wxFileName::GetPathSeparator();
    if (pString->Last() != sep)
        pString->Append(sep);
}

int squiddio_pi::GetAPIVersionMajor() {
    return MY_API_VERSION_MAJOR;
}

int squiddio_pi::GetAPIVersionMinor() {
    return MY_API_VERSION_MINOR;
}

int squiddio_pi::GetPlugInVersionMajor() {
    return PLUGIN_VERSION_MAJOR;
}

int squiddio_pi::GetPlugInVersionMinor() {
    return PLUGIN_VERSION_MINOR;
}
wxBitmap *squiddio_pi::GetPlugInBitmap() {
    return _img_plugin_logo;
}

wxString squiddio_pi::GetCommonName() {
    return _("Squiddio");
}

wxString squiddio_pi::GetShortDescription() {
    return _("Squiddio for OpenCPN");
}

wxString squiddio_pi::GetLongDescription() {
    return _(
"== User-sourced database of sailing destinations ==\n\
To download destinations for a desired region (requires Internet connection):\n\
* Position cursor on area where you want to view destinations and right click mouse\n\
* Select 'Download local sQuiddio destinations' from context-sensitive menu.\n\n\
Destinations appear as OpenCPN waypoints: \n\
* Right-click on waypoint for link to sQuiddio's destination page. \n\
* Follow link to rate destination and add comments online.\n\n\
Other menu options: \n\
* Toggle visibility for local destinations on/off \n\
* Submit a new destination (requires Internet connection and free user account)\n\
\n== In-chart log-sharing for cruisers ==\n\
* Share your GPS coordinates with your cruising friends and visualize their position\n\
on your OpenCPN charts (requires a free sQuiddio account)\n\n\
IMPORTANT: By using this plugin you are agreeing to the sQuidd.io Terms \n\
and Conditions, available at http://squidd.io/enduser_agreement");
}

bool squiddio_pi::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp) {
    return false;
}

bool squiddio_pi::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp) {
    return false;

}
int squiddio_pi::GetToolbarToolCount(void) {
    return 1;
}

void SetBackColor( wxWindow* ctrl, wxColour col)
{
    static int depth = 0; // recursion count
    if ( depth == 0 ) {   // only for the window root, not for every child

        ctrl->SetBackgroundColour( col );
    }
    
    wxWindowList kids = ctrl->GetChildren();
    for( unsigned int i = 0; i < kids.GetCount(); i++ ) {
        wxWindowListNode *node = kids.Item( i );
        wxWindow *win = node->GetData();
        
        if( win->IsKindOf( CLASSINFO(wxListBox) ) )
            ( (wxListBox*) win )->SetBackgroundColour( col );
        
        else if( win->IsKindOf( CLASSINFO(wxTextCtrl) ) )
            ( (wxTextCtrl*) win )->SetBackgroundColour( col );
        
//        else if( win->IsKindOf( CLASSINFO(wxStaticText) ) )
//            ( (wxStaticText*) win )->SetForegroundColour( uitext );
        
            else if( win->IsKindOf( CLASSINFO(wxChoice) ) )
                ( (wxChoice*) win )->SetBackgroundColour( col );
            
            else if( win->IsKindOf( CLASSINFO(wxComboBox) ) )
                ( (wxComboBox*) win )->SetBackgroundColour( col );
            
            else if( win->IsKindOf( CLASSINFO(wxRadioButton) ) )
                ( (wxRadioButton*) win )->SetBackgroundColour( col );
            
            else if( win->IsKindOf( CLASSINFO(wxScrolledWindow) ) ) {
                ( (wxScrolledWindow*) win )->SetBackgroundColour( col );
            }
            
           
            else if( win->IsKindOf( CLASSINFO(wxButton) ) ) {
                ( (wxButton*) win )->SetBackgroundColour( col );
            }
            
            else {
                ;
            }
                
            if( win->GetChildren().GetCount() > 0 ) {
                    depth++;
                    wxWindow * w = win;
                    SetBackColor( w, col );
                    depth--;
            }
    }
}

void squiddio_pi::PreferencesDialog(wxWindow* parent) {
    {

        SquiddioPrefsDialog * dialog = new SquiddioPrefsDialog(*this, parent);
        
        wxFont fo = GetOCPNGUIScaledFont_PlugIn(_T("Dialog"));
        dialog->SetFont(fo);
 
        if( m_parent_window ){
            int xmax = m_parent_window->GetSize().GetWidth();
            int ymax = m_parent_window->GetParent()->GetSize().GetHeight();  // This would be the Options dialog itself
            dialog->SetSize( xmax, ymax );
            dialog->Layout();
            
            dialog->Move(0,0);
        }
        
        wxColour cl = wxColour(214,218,222);
        SetBackColor( dialog, cl );
        
        if (g_ViewMarinas && g_ViewAnchorages == true
                && g_ViewYachtClubs == true && g_ViewDocks == true
                && g_ViewRamps == true && g_ViewFuelStations == true
                && g_ViewOthers == true) {
            dialog->m_checkBoxAll->SetValue(true);
            dialog->m_checkBoxMarinas->Enable(false);
            dialog->m_checkBoxAnchorages->Enable(false);
            dialog->m_checkBoxYachtClubs->Enable(false);
            dialog->m_checkBoxDocks->Enable(false);
            dialog->m_checkBoxRamps->Enable(false);
            dialog->m_checkBoxFuelStations->Enable(false);
            dialog->m_checkBoxOthers->Enable(false);

        } else {
            dialog->m_checkBoxAll->SetValue(false);
        }

        dialog->m_choiceHowOften->SetSelection(g_PostPeriod);
        dialog->m_choiceReceive->SetSelection(g_RetrievePeriod);
        dialog->m_textSquiddioID->SetValue(g_Email);
        dialog->m_textApiKey->SetValue(g_ApiKey);
        dialog->m_checkBoxMarinas->SetValue(g_ViewMarinas);
        dialog->m_checkBoxAnchorages->SetValue(g_ViewAnchorages);
        dialog->m_checkBoxYachtClubs->SetValue(g_ViewYachtClubs);
        dialog->m_checkBoxDocks->SetValue(g_ViewDocks);
        dialog->m_checkBoxRamps->SetValue(g_ViewRamps);
        dialog->m_checkBoxFuelStations->SetValue(g_ViewFuelStations);
        dialog->m_checkBoxOthers->SetValue(g_ViewOthers);
        dialog->m_checkBoxAIS->SetValue(g_ViewAIS);

        if (g_PostPeriod > 0 || g_RetrievePeriod > 0) {
            dialog->m_textSquiddioID->Enable(true);
            dialog->m_textApiKey->Enable(true);
        }

        dialog->Show();
        return;
    }
}
void squiddio_pi::ShowPreferencesDialog(wxWindow* parent) {
    PreferencesDialog(parent);
}

void squiddio_pi::SetLogsWindow() {
    if (g_Email.Length() > 0 && g_ApiKey.Length() > 0
            && (g_PostPeriod > 0 || g_RetrievePeriod > 0)) {
        // auth info available and either log type requested: open status window
        if (!m_plogs_window) {
            // open window if not yet open

            m_plogs_window = new logsWindow(this, m_parent_window, wxID_ANY);

            m_AUImgr = GetFrameAuiManager();
            m_AUImgr->AddPane(m_plogs_window);
            m_AUImgr->GetPane(m_plogs_window).Name(_T("Demo Window Name"));
            m_AUImgr->GetPane(m_plogs_window).Float();
            m_AUImgr->GetPane(m_plogs_window).FloatingPosition(300, 600);
            m_AUImgr->GetPane(m_plogs_window).Caption(_T("sQuiddio log updates"));
            m_AUImgr->GetPane(m_plogs_window).CaptionVisible(false);
            m_AUImgr->GetPane(m_plogs_window).GripperTop(false);
            m_AUImgr->GetPane(m_plogs_window).CloseButton(false);
            m_AUImgr->GetPane(m_plogs_window).MinimizeButton(true); //doesn't seem to work https://www.kirix.com/forums/viewtopic.php?f=15&t=658
        }
        // now make it visible
        m_AUImgr->GetPane(m_plogs_window).Show(true);
        m_AUImgr->Update();

    } else if (m_plogs_window) {
        // log updates no longer requested: hide status window and stop timer

        m_AUImgr->GetPane(m_plogs_window).Show(false);
        m_AUImgr->Update();
    }
}

void squiddio_pi::OnToolbarToolCallback(int id) {
    PreferencesDialog(m_parent_window);
}
void squiddio_pi::SetPluginMessage(wxString &message_id,
        wxString &message_body) {
}
void squiddio_pi::SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix) {

}
void squiddio_pi::SetNMEASentence(wxString &sentence) {
    if (m_plogs_window && g_PostPeriod > 0 && wxDateTime::GetTimeNow() > g_LastLogSent + period_secs(g_PostPeriod))
        if (CheckIsOnline())
            m_plogs_window->SetSentence(sentence);
}

void squiddio_pi::OnThreadActionFinished(SquiddioEvent& event)
{
    //Whatever is needed after an action was performed in the background
}

void squiddio_pi::UpdatePrefs(SquiddioPrefsDialog *dialog)
{
    int curr_retrieve_period = g_RetrievePeriod;
    
    g_PostPeriod = dialog->m_choiceHowOften->GetSelection();
    g_RetrievePeriod = dialog->m_choiceReceive->GetSelection();
    g_Email = dialog->m_textSquiddioID->GetValue().Trim();
    g_ApiKey = dialog->m_textApiKey->GetValue().Trim();
    g_ViewMarinas = dialog->m_checkBoxMarinas->GetValue();
    g_ViewAnchorages = dialog->m_checkBoxAnchorages->GetValue();
    g_ViewYachtClubs = dialog->m_checkBoxYachtClubs->GetValue();
    g_ViewDocks = dialog->m_checkBoxDocks->GetValue();
    g_ViewRamps = dialog->m_checkBoxRamps->GetValue();
    g_ViewFuelStations = dialog->m_checkBoxFuelStations->GetValue();
    g_ViewOthers = dialog->m_checkBoxOthers->GetValue();
    g_ViewAIS = dialog->m_checkBoxAIS->GetValue();
    
    if ((g_RetrievePeriod > 0 || g_PostPeriod > 0) && (g_Email.Length() == 0 || g_ApiKey.Length() == 0))
    {
        wxMessageBox(_("Log sharing was not activated. Please enter your sQuiddio user ID and API Key. \n\nTo obtain your API Key, sign up for sQuiddio (http://squidd.io/signup) and visit your online profile page (see Edit Profile link in the Dashboard), 'Numbers & Keys' tab."));
        g_RetrievePeriod=0;
        g_PostPeriod    =0;
    }
    
    SetLogsWindow();
    
    if (m_plogs_window) {
        if (g_RetrievePeriod != curr_retrieve_period){
            if (g_RetrievePeriod > 0){
                m_plogs_window->SetTimer(period_secs(g_RetrievePeriod));
            }else{
                m_plogs_window->SetTimer(0);
            }
        }
        m_plogs_window->m_ErrorCondition = wxEmptyString;
    }
    
    Layer * l;
    LayerList::iterator it;
    for (it = (*pLayerList).begin(); it != (*pLayerList).end(); ++it ) {
        l = (Layer *) (*it);
        if (l->m_LayerName.Contains(_T("logs")))
            l->m_bIsVisibleOnChart = g_RetrievePeriod > 0;
    }
    
    SaveConfig();
    RenderLayers();
}


//---------------------------------------------- preferences dialog event handlers
void SquiddioPrefsDialog::OnCheckBoxAll(wxCommandEvent& event) {
    wxCheckBox *checkbox = (wxCheckBox*) event.GetEventObject();
    if (checkbox->IsChecked()) {
        m_checkBoxMarinas->SetValue(true);
        m_checkBoxAnchorages->SetValue(true);
        m_checkBoxYachtClubs->SetValue(true);
        m_checkBoxDocks->SetValue(true);
        m_checkBoxRamps->SetValue(true);
        m_checkBoxFuelStations->SetValue(true);
        m_checkBoxOthers->SetValue(true);

        m_checkBoxMarinas->Enable(false);
        m_checkBoxAnchorages->Enable(false);
        m_checkBoxYachtClubs->Enable(false);
        m_checkBoxDocks->Enable(false);
        m_checkBoxRamps->Enable(false);
        m_checkBoxFuelStations->Enable(false);
        m_checkBoxOthers->Enable(false);
    } else {
        m_checkBoxMarinas->Enable(true);
        m_checkBoxAnchorages->Enable(true);
        m_checkBoxYachtClubs->Enable(true);
        m_checkBoxDocks->Enable(true);
        m_checkBoxRamps->Enable(true);
        m_checkBoxFuelStations->Enable(true);
        m_checkBoxOthers->Enable(true);
    }
}

void SquiddioPrefsDialog::LaunchHelpPage(wxCommandEvent& event) {
    if (!wxLaunchDefaultBrowser(_T("http://squidd.io/faq#opencpn_setup")))
        wxMessageBox(
                _("Could not launch default browser. Check your Internet connection"));
}
void SquiddioPrefsDialog::OnShareChoice(wxCommandEvent& event) {
    if (m_choiceHowOften->GetSelection() == 0
            && m_choiceReceive->GetSelection() == 0) {
        m_textSquiddioID->Enable(false);
        m_textApiKey->Enable(false);
    } else {
        m_textSquiddioID->Enable(true);
        m_textApiKey->Enable(true);
    }
    Refresh(false);
}

void SquiddioPrefsDialog::OnOKClick(wxCommandEvent& event) {
    
    m_sq_pi.UpdatePrefs( this );
    Close();
}

    

