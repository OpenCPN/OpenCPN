/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  PlugIn Manager Object
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

#include <typeinfo>
#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/aui/aui.h>
#include <wx/statline.h>
#include <wx/tokenzr.h>
#ifndef __WXMSW__
#include <cxxabi.h>
#endif // __WXMSW__
#include "dychart.h"

#include "pluginmanager.h"
#include "navutil.h"
#include "ais.h"
#include "chartbase.h"        // for ChartPlugInWrapper
#include "chartdb.h"
#include "chartdbs.h"
#include "ocpndc.h"
#include "styles.h"
#include "options.h"
#include "multiplexer.h"
#include "chartbarwin.h"
#include "routeman.h"
#include "FontMgr.h"
#include "AIS_Decoder.h"
#include "AIS_Target_Data.h"
#include "OCPN_DataStreamEvent.h"
#include "georef.h"
#include "routemanagerdialog.h"
#include "NavObjectCollection.h"
#include "OCPNRegion.h"
#include "s52plib.h"
#include "ocpn_pixel.h"
#include "s52utils.h"
#include "gshhs.h"
#include "mygeom.h"
#include "OCPNPlatform.h"

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#endif

extern MyConfig        *pConfig;
extern AIS_Decoder     *g_pAIS;
extern wxAuiManager    *g_pauimgr;

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3,0,0)
extern wxLocale        *plocale_def_lang;
#endif

extern OCPNPlatform     *g_Platform;
extern ChartDB         *ChartData;
extern MyFrame         *gFrame;
extern ocpnStyle::StyleManager* g_StyleManager;
extern options         *g_pOptions;
extern Multiplexer     *g_pMUX;
extern bool             g_bShowChartBar;
extern Piano           *g_Piano;
extern Routeman        *g_pRouteMan;
extern WayPointman     *pWayPointMan;
extern Select          *pSelect;
extern RouteManagerDialog *pRouteManagerDialog;
extern RouteList       *pRouteList;
extern PlugInManager   *g_pi_manager;
extern s52plib         *ps52plib;
extern wxString         ChartListFileName;
extern wxString         gExe_path;
extern wxString         g_Plugin_Dir;
extern bool             g_boptionsactive;
extern options         *g_options;
extern ColorScheme      global_color_scheme;
extern ChartCanvas     *cc1;
extern wxArrayString    g_locale_catalog_array;

unsigned int      gs_plib_flags;

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(Plugin_WaypointList);
WX_DEFINE_LIST(Plugin_HyperlinkList);

//    Some static helper funtions
//    Scope is local to this module

PlugIn_ViewPort CreatePlugInViewport( const ViewPort &vp)
{
    //    Create a PlugIn Viewport
    ViewPort tvp = vp;
    PlugIn_ViewPort pivp;

    pivp.clat =                   tvp.clat;                   // center point
    pivp.clon =                   tvp.clon;
    pivp.view_scale_ppm =         tvp.view_scale_ppm;
    pivp.skew =                   tvp.skew;
    pivp.rotation =               tvp.rotation;
    pivp.chart_scale =            tvp.chart_scale;
    pivp.pix_width =              tvp.pix_width;
    pivp.pix_height =             tvp.pix_height;
    pivp.rv_rect =                tvp.rv_rect;
    pivp.b_quilt =                tvp.b_quilt;
    pivp.m_projection_type =      tvp.m_projection_type;

    pivp.lat_min =                tvp.GetBBox().GetMinY();
    pivp.lat_max =                tvp.GetBBox().GetMaxY();
    pivp.lon_min =                tvp.GetBBox().GetMinX();
    pivp.lon_max =                tvp.GetBBox().GetMaxX();

    pivp.bValid =                 tvp.IsValid();                 // This VP is valid

    return pivp;
}

ViewPort CreateCompatibleViewport( const PlugIn_ViewPort &pivp)
{
    //    Create a system ViewPort
    ViewPort vp;
    
    vp.clat =                   pivp.clat;                   // center point
    vp.clon =                   pivp.clon;
    vp.view_scale_ppm =         pivp.view_scale_ppm;
    vp.skew =                   pivp.skew;
    vp.rotation =               pivp.rotation;
    vp.chart_scale =            pivp.chart_scale;
    vp.pix_width =              pivp.pix_width;
    vp.pix_height =             pivp.pix_height;
    vp.rv_rect =                pivp.rv_rect;
    vp.b_quilt =                pivp.b_quilt;
    vp.m_projection_type =      pivp.m_projection_type;
 
    if(cc1)
        vp.ref_scale = cc1->GetVP().ref_scale;
    else
        vp.ref_scale = vp.chart_scale;
    
    vp.SetBoxes();
    vp.Validate();                 // This VP is valid
    
    return vp;
}


//------------------------------------------------------------------------------
//    NMEA Event Implementation
//    PlugIn Messaging scheme Event
//------------------------------------------------------------------------------

const wxEventType wxEVT_OCPN_MSG = wxNewEventType();

OCPN_MsgEvent::OCPN_MsgEvent( wxEventType commandType, int id )
:wxEvent(id, commandType)
{
}

OCPN_MsgEvent::~OCPN_MsgEvent( )
{
}

wxEvent* OCPN_MsgEvent::Clone() const
{
    OCPN_MsgEvent *newevent=new OCPN_MsgEvent(*this);
    newevent->m_MessageID=this->m_MessageID.c_str();  // this enforces a deep copy of the string data
    newevent->m_MessageText=this->m_MessageText.c_str();
    return newevent;
}

//------------------------------------------------------------------------------------------------
//
//          The PlugInToolbarToolContainer Implementation
//
//------------------------------------------------------------------------------------------------
PlugInToolbarToolContainer::PlugInToolbarToolContainer()
{
    bitmap_dusk = NULL;
    bitmap_night = NULL;
    bitmap_day = NULL;
    bitmap_Rollover = NULL;;
}

PlugInToolbarToolContainer::~PlugInToolbarToolContainer()
{
    delete bitmap_dusk;
    delete bitmap_night;
    delete bitmap_day;
    delete bitmap_Rollover;
}

//-----------------------------------------------------------------------------------------------------
//
//          The PlugIn Manager Implementation
//
//-----------------------------------------------------------------------------------------------------
PlugInManager *s_ppim;

PlugInManager::PlugInManager(MyFrame *parent)
{
    pParent = parent;
    s_ppim = this;

    MyFrame *pFrame = GetParentFrame();
    if(pFrame)
    {
        m_plugin_menu_item_id_next = pFrame->GetCanvasWindow()->GetNextContextMenuId();
        m_plugin_tool_id_next = pFrame->GetNextToolbarToolId();
    }
}

PlugInManager::~PlugInManager()
{
}


bool PlugInManager::LoadAllPlugIns(const wxString &plugin_dir, bool load_enabled, bool b_enable_blackdialog)
{
    pConfig->SetPath( _T("/PlugIns/") );
    SetPluginOrder( pConfig->Read( _T("PluginOrder"), wxEmptyString ) );
    
    m_benable_blackdialog = b_enable_blackdialog;
    
    m_plugin_location = plugin_dir;

    wxString msg(_T("PlugInManager searching for PlugIns in location "));
    msg += m_plugin_location;
    wxLogMessage(msg);

#ifdef __WXMSW__
    wxString pispec = _T("*_pi.dll");
#else
#ifdef __WXOSX__
    wxString pispec = _T("*_pi.dylib");
#else
    wxString pispec = _T("*_pi.so");
#endif
#endif

    if(!::wxDirExists(m_plugin_location))
    {
        msg = m_plugin_location;
        msg.Prepend(_T("   Directory "));
        msg.Append(_T(" does not exist."));
        wxLogMessage(msg);
        return false;
    }

    wxArrayString file_list;
        
    int get_flags =  wxDIR_FILES | wxDIR_DIRS;
#ifdef __WXMSW__
#ifdef _DEBUG
    get_flags =  wxDIR_FILES;
#endif        
#endif        

    bool ret = false; // return true if at least one new plugins gets loaded/unloaded
    wxDir::GetAllFiles( m_plugin_location, &file_list, pispec, get_flags );
    
    for(unsigned int i=0 ; i < file_list.GetCount() ; i++) {
        wxString file_name = file_list[i];
        wxString plugin_file = wxFileName(file_name).GetFullName();
        wxDateTime plugin_modification = wxFileName(file_name).GetModificationTime();

        // this gets called every time we switch to the plugins tab.
        // this allows plugins to be installed and enabled without restarting opencpn.
        // For this reason we must check that we didn't already load this plugin
        bool loaded = false;
        for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
        {
            PlugInContainer *pic = plugin_array.Item(i);
            if(pic->m_plugin_filename == plugin_file) {
                if(pic->m_plugin_modification != plugin_modification) {
                    // modification times don't match, reload plugin
                    plugin_array.Remove(pic);
                    i--;

                    DeactivatePlugIn(pic);
                    pic->m_destroy_fn(pic->m_pplugin);
                    
                    delete pic->m_plibrary;            // This will unload the PlugIn
                    delete pic;
                    ret = true;
                } else {
                    loaded = true;
                    break;
                }
            }
        }
        if(loaded)
            continue;

        //    Check the config file to see if this PlugIn is user-enabled
        wxString config_section = ( _T ( "/PlugIns/" ) );
        config_section += plugin_file;
        pConfig->SetPath ( config_section );
        bool enabled;
        pConfig->Read ( _T ( "bEnabled" ), &enabled, false );

        // only loading enabled plugins? check that it is enabled
        if(load_enabled && !enabled)
            continue;
            
        bool b_compat = CheckPluginCompatibility(file_name);
            
        if(!b_compat)
        {
            wxLogMessage(wxString::Format(_("    Incompatible PlugIn detected: %s"), file_name.c_str()));
            OCPNMessageBox( NULL, wxString::Format(_("The plugin %s is not compatible with this version of OpenCPN, please get an updated version."), plugin_file.c_str()), wxString(_("OpenCPN Info")), wxICON_INFORMATION | wxOK, 10 );
        }
            
        PlugInContainer *pic = NULL;
        if(b_compat)
            pic = LoadPlugIn(file_name);
        if(pic)
        {
            if(pic->m_pplugin)
            {
                plugin_array.Add(pic);
                    
                //    The common name is available without initialization and startup of the PlugIn
                pic->m_common_name = pic->m_pplugin->GetCommonName();
                    
                pic->m_plugin_filename = plugin_file;
                pic->m_plugin_modification = plugin_modification;
                pic->m_bEnabled = enabled;
                if(pic->m_bEnabled)
                {
                    pic->m_cap_flag = pic->m_pplugin->Init();
                    pic->m_bInitState = true;
                }
                    
                pic->m_short_description = pic->m_pplugin->GetShortDescription();
                pic->m_long_description = pic->m_pplugin->GetLongDescription();
                pic->m_version_major = pic->m_pplugin->GetPlugInVersionMajor();
                pic->m_version_minor = pic->m_pplugin->GetPlugInVersionMinor();
                pic->m_bitmap = pic->m_pplugin->GetPlugInBitmap();

                ret = true;
            }
            else        // not loaded
            {
                wxString msg;
                msg.Printf(_T("    PlugInManager: Unloading invalid PlugIn, API version %d "), pic->m_api_version );
                wxLogMessage(msg);
                    
                pic->m_destroy_fn(pic->m_pplugin);
                    
                delete pic->m_plibrary;            // This will unload the PlugIn
                delete pic;
            }
        }
    }
    
    std::map<int, PlugInContainer*> ap;
    for( unsigned int i = 0; i < plugin_array.GetCount(); i++ )
    {
        int index = m_plugin_order.Index( plugin_array.Item(i)->m_common_name );
        if( index != wxNOT_FOUND )
        {
            ap[index] = plugin_array.Item(i);
        }
        else
            ap[10000 + i] = plugin_array.Item(i);
    }
    plugin_array.Empty();
    for (std::map<int, PlugInContainer*>::reverse_iterator iter = ap.rbegin(); iter != ap.rend(); ++iter)
    {
        plugin_array.Insert( iter->second, 0 );
    }
    ap.clear();
    
    UpDateChartDataTypes();

    // Inform plugins of the current color scheme
    g_pi_manager->SetColorSchemeForAllPlugIns( global_color_scheme );
    
    return ret;
}

bool PlugInManager::CallLateInit(void)
{
    bool bret = true;

    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);

        switch(pic->m_api_version)
        {
            case 110:
                if(pic->m_cap_flag & WANTS_LATE_INIT) {
                    wxString msg(_T("PlugInManager: Calling LateInit PlugIn: "));
                    msg += pic->m_plugin_file;
                    wxLogMessage(msg);

                    opencpn_plugin_110* ppi = dynamic_cast<opencpn_plugin_110*>(pic->m_pplugin);
                    ppi->LateInit();
                    }
                break;
        }
    }

    return bret;
}

void PlugInManager::SendVectorChartObjectInfo(const wxString &chart, const wxString &feature, const wxString &objname, double &lat, double &lon, double &scale, int &nativescale)
{
    wxString decouple_chart(chart);
    wxString decouple_feature(feature);
    wxString decouple_objname(objname);
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_VECTOR_CHART_OBJECT_INFO)
            {
                switch(pic->m_api_version)
                {
                case 112:
                case 113:
                {
                    opencpn_plugin_112 *ppi = dynamic_cast<opencpn_plugin_112 *>(pic->m_pplugin);
                    if(ppi)
                        ppi->SendVectorChartObjectInfo(decouple_chart, decouple_feature, decouple_objname, lat, lon, scale, nativescale);
                    break;
                }
                default:
                    break;
                }
            }
        }
    }
}


bool PlugInManager::UpdatePlugIns()
{
    bool bret = false;

    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);

        if(pic->m_bEnabled && !pic->m_bInitState)
        {
            wxString msg(_T("PlugInManager: Initializing PlugIn: "));
            msg += pic->m_plugin_file;
            wxLogMessage(msg);

            pic->m_cap_flag = pic->m_pplugin->Init();
            pic->m_pplugin->SetDefaults();
            pic->m_bInitState = true;
            pic->m_short_description = pic->m_pplugin->GetShortDescription();
            pic->m_long_description = pic->m_pplugin->GetLongDescription();
            pic->m_version_major = pic->m_pplugin->GetPlugInVersionMajor();
            pic->m_version_minor = pic->m_pplugin->GetPlugInVersionMinor();
            pic->m_bitmap = pic->m_pplugin->GetPlugInBitmap();
            bret = true;
        }
        else if(!pic->m_bEnabled && pic->m_bInitState)
        {
            bret = DeactivatePlugIn(pic);

        }
    }

    UpDateChartDataTypes();

    return bret;
}


bool PlugInManager::UpDateChartDataTypes(void)
{
    bool bret = false;
    if(NULL == ChartData)
        return bret;

    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);

        if(pic->m_bInitState) {
          if((pic->m_cap_flag & INSTALLS_PLUGIN_CHART) || (pic->m_cap_flag & INSTALLS_PLUGIN_CHART_GL))
              bret = true;
        }
    }

    if(bret)
        ChartData->UpdateChartClassDescriptorArray();

    return bret;
}


bool PlugInManager::DeactivatePlugIn(PlugInContainer *pic)
{
    bool bret = false;

    if(pic)
    {
        wxString msg(_T("PlugInManager: Deactivating PlugIn: "));
        msg += pic->m_plugin_file;
        wxLogMessage(msg);

        pic->m_pplugin->DeInit();

        //    Deactivate (Remove) any ToolbarTools added by this PlugIn
        for(unsigned int i=0; i < m_PlugInToolbarTools.GetCount(); i++)
        {
            PlugInToolbarToolContainer *pttc = m_PlugInToolbarTools.Item(i);

            if(pttc->m_pplugin == pic->m_pplugin)
            {
                m_PlugInToolbarTools.Remove(pttc);
                delete pttc;
            }
        }

        //    Deactivate (Remove) any ContextMenu items addded by this PlugIn
        for(unsigned int i=0; i < m_PlugInMenuItems.GetCount(); i++)
        {
            PlugInMenuItemContainer *pimis = m_PlugInMenuItems.Item(i);
            if(pimis->m_pplugin == pic->m_pplugin)
            {
                m_PlugInMenuItems.Remove(pimis);
                delete pimis;
            }
        }

        pic->m_bInitState = false;
        bret = true;
    }

    return bret;
}

void PlugInManager::SetPluginOrder( wxString serialized_names )
{
    m_plugin_order.Empty();
    wxStringTokenizer tokenizer( serialized_names, _T(";") );
    while( tokenizer.HasMoreTokens() )
    {
        m_plugin_order.Add( tokenizer.GetNextToken() );
    }
}

wxString PlugInManager::GetPluginOrder()
{
    wxString plugins = wxEmptyString;
    for( unsigned int i = 0; i < plugin_array.GetCount(); i++ )
    {
        plugins.Append( plugin_array.Item(i)->m_common_name );
        if( i < plugin_array.GetCount() - 1 )
            plugins.Append(';');
    }
    return plugins;
}

bool PlugInManager::UpdateConfig()
{
    pConfig->SetPath( _T("/PlugIns/") );
    pConfig->Write( _T("PluginOrder"), GetPluginOrder() );

    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);

        wxString config_section = ( _T ( "/PlugIns/" ) );
        config_section += pic->m_plugin_filename;
        pConfig->SetPath ( config_section );
        pConfig->Write ( _T ( "bEnabled" ), pic->m_bEnabled );
    }

    return true;
}

bool PlugInManager::UnLoadAllPlugIns()
{
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        wxString msg(_T("PlugInManager: UnLoading PlugIn: "));
        msg += pic->m_plugin_file;
        wxLogMessage(msg);

        pic->m_destroy_fn(pic->m_pplugin);

        delete pic->m_plibrary;            // This will unload the PlugIn

        pic->m_bInitState = false;

        delete pic;
    }
    return true;
}

bool PlugInManager::DeactivateAllPlugIns()
{
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic && pic->m_bEnabled && pic->m_bInitState)
            DeactivatePlugIn(pic);
    }
    return true;
}

#ifdef __WXMSW__
/*Convert Virtual Address to File Offset */
DWORD Rva2Offset(DWORD rva, PIMAGE_SECTION_HEADER psh, PIMAGE_NT_HEADERS pnt)
{
    size_t i = 0;
    PIMAGE_SECTION_HEADER pSeh;
    if (rva == 0)
    {
        return (rva);
    }
    pSeh = psh;
    for (i = 0; i < pnt->FileHeader.NumberOfSections; i++)
    {
        if (rva >= pSeh->VirtualAddress && rva < pSeh->VirtualAddress +
            pSeh->Misc.VirtualSize)
        {
            break;
        }
        pSeh++;
    }
    return (rva - pSeh->VirtualAddress + pSeh->PointerToRawData);
}
#endif

bool PlugInManager::CheckPluginCompatibility(wxString plugin_file)
{
    bool b_compat = true;

#ifdef __WXMSW__
    char strver[22]; //Enough space even for very big integers...
    sprintf(strver, "%i%i", wxMAJOR_VERSION, wxMINOR_VERSION);
    LPCWSTR fNmae = plugin_file.wc_str();
    HANDLE handle = CreateFile(fNmae, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    DWORD byteread, size = GetFileSize(handle, NULL);
    PVOID virtualpointer = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
    ReadFile(handle, virtualpointer, size, &byteread, NULL);
    CloseHandle(handle);
    // Get pointer to NT header
    PIMAGE_NT_HEADERS           ntheaders = (PIMAGE_NT_HEADERS)(PCHAR(virtualpointer) + PIMAGE_DOS_HEADER(virtualpointer)->e_lfanew);
    PIMAGE_SECTION_HEADER       pSech = IMAGE_FIRST_SECTION(ntheaders);//Pointer to first section header
    PIMAGE_IMPORT_DESCRIPTOR    pImportDescriptor; //Pointer to import descriptor 
    if (ntheaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size != 0)/*if size of the table is 0 - Import Table does not exist */
    {
        pImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD_PTR)virtualpointer + \
            Rva2Offset(ntheaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress, pSech, ntheaders));
        LPSTR libname[256];
        size_t i = 0;
        // Walk until you reached an empty IMAGE_IMPORT_DESCRIPTOR
        while (pImportDescriptor->Name != NULL)
        {
            //Get the name of each DLL
            libname[i] = (PCHAR)((DWORD_PTR)virtualpointer + Rva2Offset(pImportDescriptor->Name, pSech, ntheaders));
            //wxMessageBox(wxString::Format(_T("%s"), libname[i]));
            if (strstr(libname[i], "wx") != NULL)
            {
                if (strstr(libname[i], strver) == NULL)
                    b_compat = false;
                break;
            }
            pImportDescriptor++; //advance to next IMAGE_IMPORT_DESCRIPTOR
            i++;
        }
    }
    else
    {
        wxLogMessage(wxString::Format(_T("No Import Table! in %s"), plugin_file.c_str()));
    }
    if (virtualpointer)
        VirtualFree(virtualpointer, size, MEM_DECOMMIT);
#endif
#ifdef __WXGTK__
    wxString cmd = _T("ldd ") + plugin_file + _T(" 2>&1");
    FILE *ldd = popen( cmd.mb_str(), "r" );
    if (ldd != NULL)
    {
        char buf[1024];
        
        char strver[22]; //Enough space even for very big integers...
        sprintf( strver, "%i.%i", wxMAJOR_VERSION, wxMINOR_VERSION );

        while( fscanf(ldd, "%s", buf) != EOF )
        {
            if( strstr(buf, "libwx") != NULL )
            {
                if(  strstr(buf, strver) == NULL )
                    b_compat = false;
                break;
            }
        }
        fclose(ldd);
    }
#endif // __WXGTK__

    return b_compat;
}

void PlugInManager::ShowDeferredBlacklistMessages()
{
    for( unsigned int i=0 ; i < m_deferred_blacklist_messages.GetCount() ; i++){
        OCPNMessageBox ( NULL, m_deferred_blacklist_messages.Item(i), wxString( _("OpenCPN Info") ), wxICON_INFORMATION | wxOK, 5 );  // 5 second timeout
    }
        
}

bool PlugInManager::CheckBlacklistedPlugin(opencpn_plugin* plugin)
{
    int len = sizeof(PluginBlacklist) / sizeof(BlackListedPlugin);
    int major = plugin->GetPlugInVersionMajor();
    int minor = plugin->GetPlugInVersionMinor();
    
#ifdef __WXMSW__
    wxString name = wxString::FromAscii(typeid(*plugin).name());
    name.Replace(_T("class "), wxEmptyString);
#else
    const std::type_info &ti = typeid(*plugin);
    int status;
    char *realname = abi::__cxa_demangle(ti.name(), 0, 0, &status);
    wxString name = wxString::FromAscii(realname);
    free(realname);
#endif // __WXMSW__
    for (int i = 0; i < len; i++) {
        if( ( PluginBlacklist[i].all_lower && name == PluginBlacklist[i].name && PluginBlacklist[i].version_major >= major && PluginBlacklist[i].version_minor >= minor ) ||
            ( !PluginBlacklist[i].all_lower && name == PluginBlacklist[i].name && PluginBlacklist[i].version_major == major && PluginBlacklist[i].version_minor == minor ) )
        {
            wxString msg;
            wxString msg1;
            if ( PluginBlacklist[i].hard ){
                msg = wxString::Format(_("PlugIn %s (%s), version %i.%i was detected.\n This version is known to be unstable and will not be loaded.\n Please update this PlugIn at the opencpn.org website."),
                                              PluginBlacklist[i].name.c_str(), plugin->GetCommonName().c_str(), major, minor), _("Blacklisted plugin detected...");
                msg1= wxString::Format(_T("    PlugIn %s (%s), version %i.%i was detected. Hard blacklisted. Not loaded."),
                                              PluginBlacklist[i].name.c_str(), plugin->GetCommonName().c_str(), major, minor);
            }
            else{
                msg = wxString::Format(_("PlugIn %s (%s), version %i.%i was detected.\n This version is known to be unstable.\n Please update this PlugIn at the opencpn.org website."),
                                              PluginBlacklist[i].name.c_str(), plugin->GetCommonName().c_str(), major, minor), _("Blacklisted plugin detected...");
                msg1= wxString::Format(_T("    PlugIn %s (%s), version %i.%i was detected. Soft blacklisted. Loaded."),
                                              PluginBlacklist[i].name.c_str(), plugin->GetCommonName().c_str(), major, minor);
            }
            
            wxLogMessage(msg1);
            if(m_benable_blackdialog)
                OCPNMessageBox ( NULL, msg, wxString( _("OpenCPN Info") ), wxICON_INFORMATION | wxOK, 5 );  // 5 second timeout
            else
                m_deferred_blacklist_messages.Add(msg);
            
            return PluginBlacklist[i].hard;
        }
    }
    return false;
}

PlugInContainer *PlugInManager::LoadPlugIn(wxString plugin_file)
{
    wxString msg(_T("PlugInManager: Loading PlugIn: "));
    msg += plugin_file;
    wxLogMessage(msg);

    PlugInContainer *pic = new PlugInContainer;
    pic->m_plugin_file = plugin_file;

    // load the library
    wxDynamicLibrary *plugin = new wxDynamicLibrary(plugin_file);
    pic->m_plibrary = plugin;     // Save a pointer to the wxDynamicLibrary for later deletion
    
    if( !wxIsReadable(plugin_file) )
    {
        msg = _("Unreadable PlugIn library detected, check the file permissions:\n");
        msg += plugin_file;
        msg += _T("\n\n");
        OCPNMessageBox ( NULL, msg, wxString( _("OpenCPN Info") ), wxICON_INFORMATION | wxOK, 10 );  // 10 second timeout
    }
    else if(!plugin->IsLoaded())
    {
        //  Look in the Blacklist, try to match a filename, to give some kind of message
        //  extract the probable plugin name
        wxFileName fn( plugin_file );
        wxString prob_pi_name;
        wxString name = fn.GetName();
        prob_pi_name = name;
        
#ifdef __WXGTK__
        prob_pi_name = name.Mid(3);     // lop off "lib"
#endif        
#ifdef __WXOSX__
        prob_pi_name = name.Mid(3);     // lop off "lib"
#endif        
        
        int len = sizeof(PluginBlacklist) / sizeof(BlackListedPlugin);
        for (int i = 0; i < len; i++) {
            wxString candidate = PluginBlacklist[i].name.Lower();
            if( prob_pi_name.Lower().EndsWith(candidate)){
                wxString msg = _("Incompatible PlugIn detected:\n");
                msg += plugin_file;
                msg += _T("\n\n");
                
                wxString msg1;
                msg1 = wxString::Format(_("PlugIn [ %s ] version %i.%i"),
                                        PluginBlacklist[i].name.c_str(),
                                        PluginBlacklist[i].version_major, PluginBlacklist[i].version_minor);
                msg += msg1;
                if(PluginBlacklist[i].all_lower)
                    msg += _(", and all previous versions,");
                msg += _(" is incompatible with this version of OpenCPN."),
                                        
                OCPNMessageBox ( NULL, msg, wxString( _("OpenCPN Info") ), wxICON_INFORMATION | wxOK, 10 );  // 10 second timeout
                break;
            }
        }
        
        
        
        wxString msg(_T("   PlugInManager: Cannot load library: "));
        msg += plugin_file;
        msg += _T(" ");
        wxLogMessage(msg);
        delete plugin;
        delete pic;
        return NULL;
    }


    // load the factory symbols
    create_t* create_plugin = (create_t*)plugin->GetSymbol(_T("create_pi"));
    if (NULL == create_plugin)
    {
        wxString msg(_T("   PlugInManager: Cannot load symbol create_pi: "));
        msg += plugin_file;
        wxLogMessage(msg);
        delete plugin;
        delete pic;
        return NULL;
    }

    destroy_t* destroy_plugin = (destroy_t*) plugin->GetSymbol(_T("destroy_pi"));
    pic->m_destroy_fn = destroy_plugin;
    if (NULL == destroy_plugin) {
        wxString msg(_T("   PlugInManager: Cannot load symbol destroy_pi: "));
        msg += plugin_file;
        wxLogMessage(msg);
        delete plugin;
        delete pic;
        return NULL;
    }


    // create an instance of the plugin class
    opencpn_plugin* plug_in = create_plugin(this);

    int api_major = plug_in->GetAPIVersionMajor();
    int api_minor = plug_in->GetAPIVersionMinor();
    int ver = (api_major * 100) + api_minor;
    pic->m_api_version = ver;

    int pi_major = plug_in->GetPlugInVersionMajor();
    int pi_minor = plug_in->GetPlugInVersionMinor();
    int pi_ver = (pi_major * 100) + pi_minor;
    
    if ( CheckBlacklistedPlugin(plug_in) ) {
        delete plugin;
        delete pic;
        return NULL;
    }

    switch(ver)
    {
    case 105:
        pic->m_pplugin = dynamic_cast<opencpn_plugin*>(plug_in);
        break;

    case 106:
        pic->m_pplugin = dynamic_cast<opencpn_plugin_16*>(plug_in);
        break;

    case 107:
        pic->m_pplugin = dynamic_cast<opencpn_plugin_17*>(plug_in);
        break;

    case 108:
        pic->m_pplugin = dynamic_cast<opencpn_plugin_18*>(plug_in);
        break;

    case 109:
        pic->m_pplugin = dynamic_cast<opencpn_plugin_19*>(plug_in);
        break;

    case 110:
        pic->m_pplugin = dynamic_cast<opencpn_plugin_110*>(plug_in);
        break;
        
    case 111:
        pic->m_pplugin = dynamic_cast<opencpn_plugin_111*>(plug_in);
        break;
        
    case 112:
        pic->m_pplugin = dynamic_cast<opencpn_plugin_112*>(plug_in);
        break;

    case 113:
        pic->m_pplugin = dynamic_cast<opencpn_plugin_113*>(plug_in);
        break;
        
    default:
        break;
    }

    if(pic->m_pplugin)
    {
        msg = _T("  ");
        msg += plugin_file;
        wxString msg1;
        msg1.Printf(_T("\n              API Version detected: %d"), ver);
        msg += msg1;
        msg1.Printf(_T("\n              PlugIn Version detected: %d"), pi_ver);
        msg += msg1;
        wxLogMessage(msg);
    }
    else
    {
        msg = _T("    ");
        msg += plugin_file;
        wxString msg1 = _T(" cannot be loaded");
        msg += msg1;
        wxLogMessage(msg);
    }

    return pic;
}

bool PlugInManager::RenderAllCanvasOverlayPlugIns( ocpnDC &dc, const ViewPort &vp)
{
    for(unsigned int i = 0; i < plugin_array.GetCount(); i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_OVERLAY_CALLBACK)
            {
                PlugIn_ViewPort pivp = CreatePlugInViewport( vp );

                wxDC *pdc = dc.GetDC();
                if(pdc)                       // not in OpenGL mode
                {
                    switch(pic->m_api_version)
                    {
                    case 106:
                    {
                        opencpn_plugin_16 *ppi = dynamic_cast<opencpn_plugin_16 *>(pic->m_pplugin);
                        if(ppi)
                            ppi->RenderOverlay(*pdc, &pivp);
                        break;
                    }
                    case 107:
                    {
                        opencpn_plugin_17 *ppi = dynamic_cast<opencpn_plugin_17 *>(pic->m_pplugin);
                        if(ppi)
                            ppi->RenderOverlay(*pdc, &pivp);
                        break;
                    }
                    case 108:
                    case 109:
                    case 110:
                    case 111:
                    case 112:
                    case 113:
                    {
                        opencpn_plugin_18 *ppi = dynamic_cast<opencpn_plugin_18 *>(pic->m_pplugin);
                        if(ppi)
                            ppi->RenderOverlay(*pdc, &pivp);
                        break;
                    }

                    default:
                        break;
                    }
                }
                else
                {
                    //    If in OpenGL mode, and the PlugIn has requested OpenGL render callbacks,
                    //    then there is no need to render by wxDC here.
                    if(pic->m_cap_flag & WANTS_OPENGL_OVERLAY_CALLBACK)
                        continue;


                    if((m_cached_overlay_bm.GetWidth() != vp.pix_width) || (m_cached_overlay_bm.GetHeight() != vp.pix_height))
                        m_cached_overlay_bm.Create(vp.pix_width, vp.pix_height, -1);

                    wxMemoryDC mdc;
                    mdc.SelectObject ( m_cached_overlay_bm );
                    mdc.SetBackground ( *wxBLACK_BRUSH );
                    mdc.Clear();


                    bool b_rendered = false;

                    switch(pic->m_api_version)
                    {
                    case 106:
                    {
                        opencpn_plugin_16 *ppi = dynamic_cast<opencpn_plugin_16 *>(pic->m_pplugin);
                        if(ppi)
                            b_rendered = ppi->RenderOverlay(mdc, &pivp);
                        break;
                    }
                    case 107:
                    {
                        opencpn_plugin_17 *ppi = dynamic_cast<opencpn_plugin_17 *>(pic->m_pplugin);
                        if(ppi)
                            b_rendered = ppi->RenderOverlay(mdc, &pivp);
                        break;
                    }
                    case 108:
                    case 109:
                    case 110:
                    case 111:
                    case 112:
                    case 113:
                    {
                        opencpn_plugin_18 *ppi = dynamic_cast<opencpn_plugin_18 *>(pic->m_pplugin);
                        if(ppi)
                            b_rendered = ppi->RenderOverlay(mdc, &pivp);
                        break;
                    }

                    default:
                    {
                        b_rendered = pic->m_pplugin->RenderOverlay(&mdc, &pivp);
                        break;
                    }
                    }

                    mdc.SelectObject(wxNullBitmap);

                    if(b_rendered)
                    {
                        wxMask *p_msk = new wxMask(m_cached_overlay_bm, wxColour(0,0,0));
                        m_cached_overlay_bm.SetMask(p_msk);

                        dc.DrawBitmap(m_cached_overlay_bm, 0, 0, true);
                    }
                }
            }
            else if(pic->m_cap_flag & WANTS_OPENGL_OVERLAY_CALLBACK)
            {
            }

        }
    }

    return true;
}

bool PlugInManager::RenderAllGLCanvasOverlayPlugIns( wxGLContext *pcontext, const ViewPort &vp)
{
    for(unsigned int i = 0; i < plugin_array.GetCount(); i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_OPENGL_OVERLAY_CALLBACK)
            {
                PlugIn_ViewPort pivp = CreatePlugInViewport( vp );

                switch(pic->m_api_version)
                {
                case 107:
                {
                    opencpn_plugin_17 *ppi = dynamic_cast<opencpn_plugin_17 *>(pic->m_pplugin);
                    if(ppi)
                        ppi->RenderGLOverlay(pcontext, &pivp);
                    break;
                }

                case 108:
                case 109:
                case 110:
                case 111:
                case 112:
                case 113:
                {
                    opencpn_plugin_18 *ppi = dynamic_cast<opencpn_plugin_18 *>(pic->m_pplugin);
                    if(ppi)
                        ppi->RenderGLOverlay(pcontext, &pivp);
                    break;
                }
                default:
                    break;
                }
            }
        }
    }

    return true;
}

bool PlugInManager::SendMouseEventToPlugins( wxMouseEvent &event)
{
    bool bret = false;
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_MOUSE_EVENTS){
            {
                switch(pic->m_api_version)
                {
                    case 112:
                    case 113:
                    {
                        opencpn_plugin_112 *ppi = dynamic_cast<opencpn_plugin_112*>(pic->m_pplugin);
                            if(ppi)
                                if(ppi->MouseEventHook( event ))
                                    bret = true;
                            break;
                        }
                        
                        default:
                            break;
                    }
                }
            }
        }
    }
    
    return bret;;
}

bool PlugInManager::SendKeyEventToPlugins( wxKeyEvent &event)
{
    bool bret = false;
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_KEYBOARD_EVENTS){
                {
                    switch(pic->m_api_version)
                    {
                        case 113:
                        {
                            opencpn_plugin_113 *ppi = dynamic_cast<opencpn_plugin_113*>(pic->m_pplugin);
                            if(ppi)
                                if(ppi->KeyboardEventHook( event ))
                                    bret = true;
                                break;
                        }
                        
                        default:
                            break;
                    }
                }
            }
        }
    }
    
    return bret;;
}


void PlugInManager::SendViewPortToRequestingPlugIns( ViewPort &vp )
{
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_ONPAINT_VIEWPORT)
            {
                PlugIn_ViewPort pivp = CreatePlugInViewport( vp );
                pic->m_pplugin->SetCurrentViewPort(pivp);
            }
        }
    }
}

void PlugInManager::SendCursorLatLonToAllPlugIns( double lat, double lon)
{
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_CURSOR_LATLON)
                pic->m_pplugin->SetCursorLatLon(lat, lon);
        }
    }
}

void NotifySetupOptionsPlugin( PlugInContainer *pic )
{
    if(pic->m_bEnabled && pic->m_bInitState)
    {
        if(pic->m_cap_flag & INSTALLS_TOOLBOX_PAGE)
        {
            switch(pic->m_api_version)
            {
            case 109:
            case 110:
            case 111:
            case 112:
            case 113:
            {
                opencpn_plugin_19 *ppi = dynamic_cast<opencpn_plugin_19 *>(pic->m_pplugin);
                if(ppi) {
                    ppi->OnSetupOptions();
                    pic->m_bToolboxPanel = true;
                }
                break;
            }
            default:
                break;
            }
        }
    }
}

void PlugInManager::NotifySetupOptions()
{
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        NotifySetupOptionsPlugin( pic );
    }
}

void PlugInManager::CloseAllPlugInPanels( int ok_apply_cancel)
{
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if((pic->m_cap_flag & INSTALLS_TOOLBOX_PAGE) && ( pic->m_bToolboxPanel))
            {
                pic->m_pplugin->OnCloseToolboxPanel(0, ok_apply_cancel);
                pic->m_bToolboxPanel = false;
            }
        }
    }

}

int PlugInManager::AddCanvasContextMenuItem(wxMenuItem *pitem, opencpn_plugin *pplugin )
{
    PlugInMenuItemContainer *pmic = new PlugInMenuItemContainer;
    pmic->pmenu_item = pitem;
    pmic->m_pplugin = pplugin;
    pmic->id = pitem->GetId()==wxID_SEPARATOR?wxID_SEPARATOR:m_plugin_menu_item_id_next;
    pmic->b_viz = true;
    pmic->b_grey = false;

    m_PlugInMenuItems.Add(pmic);

    m_plugin_menu_item_id_next++;

    return pmic->id;
}




void PlugInManager::RemoveCanvasContextMenuItem(int item)
{
    for(unsigned int i=0; i < m_PlugInMenuItems.GetCount(); i++)
    {
        PlugInMenuItemContainer *pimis = m_PlugInMenuItems.Item(i);
        {
            if(pimis->id == item)
            {
                m_PlugInMenuItems.Remove(pimis);
                delete pimis;
                break;
            }
        }
    }
}

void PlugInManager::SetCanvasContextMenuItemViz(int item, bool viz)
{
    for(unsigned int i=0; i < m_PlugInMenuItems.GetCount(); i++)
    {
        PlugInMenuItemContainer *pimis = m_PlugInMenuItems.Item(i);
        {
            if(pimis->id == item)
            {
                pimis->b_viz = viz;
                break;
            }
        }
    }
}

void PlugInManager::SetCanvasContextMenuItemGrey(int item, bool grey)
{
    for(unsigned int i=0; i < m_PlugInMenuItems.GetCount(); i++)
    {
        PlugInMenuItemContainer *pimis = m_PlugInMenuItems.Item(i);
        {
            if(pimis->id == item)
            {
                pimis->b_grey = grey;
                break;
            }
        }
    }
}

void PlugInManager::SendNMEASentenceToAllPlugIns(const wxString &sentence)
{
    wxString decouple_sentence(sentence); // decouples 'const wxString &' and 'wxString &' to keep bin compat for plugins
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_NMEA_SENTENCES)
                pic->m_pplugin->SetNMEASentence(decouple_sentence);
        }
    }
}

void PlugInManager::SendJSONMessageToAllPlugins(const wxString &message_id, wxJSONValue v)
{
    wxJSONWriter w;
    wxString out;
    w.Write(v, out);
    SendMessageToAllPlugins(message_id,out);
//   wxLogMessage(message_id);
//   wxLogMessage(out);
}

void PlugInManager::SendMessageToAllPlugins(const wxString &message_id, const wxString &message_body)
{
    wxString decouple_message_id(message_id); // decouples 'const wxString &' and 'wxString &' to keep bin compat for plugins
    wxString decouple_message_body(message_body); // decouples 'const wxString &' and 'wxString &' to keep bin compat for plugins
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_PLUGIN_MESSAGING)
            {
                switch(pic->m_api_version)
                {
                case 106:
                {
                    opencpn_plugin_16 *ppi = dynamic_cast<opencpn_plugin_16 *>(pic->m_pplugin);
                    if(ppi)
                        ppi->SetPluginMessage(decouple_message_id, decouple_message_body);
                    break;
                }
                case 107:
                {
                    opencpn_plugin_17 *ppi = dynamic_cast<opencpn_plugin_17 *>(pic->m_pplugin);
                    if(ppi)
                        ppi->SetPluginMessage(decouple_message_id, decouple_message_body);
                    break;
                }
                case 108:
                case 109:
                case 110:
                case 111:
                case 112:
                case 113:
                {
                    opencpn_plugin_18 *ppi = dynamic_cast<opencpn_plugin_18 *>(pic->m_pplugin);
                    if(ppi)
                        ppi->SetPluginMessage(decouple_message_id, decouple_message_body);
                    break;
                }
                default:
                    break;
                }
            }
        }
    }
}


void PlugInManager::SendAISSentenceToAllPlugIns(const wxString &sentence)
{
    wxString decouple_sentence(sentence); // decouples 'const wxString &' and 'wxString &' to keep bin compat for plugins
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_AIS_SENTENCES)
                pic->m_pplugin->SetAISSentence(decouple_sentence);
        }
    }
}

void PlugInManager::SendPositionFixToAllPlugIns(GenericPosDatEx *ppos)
{
    //    Send basic position fix
    PlugIn_Position_Fix pfix;
    pfix.Lat = ppos->kLat;
    pfix.Lon = ppos->kLon;
    pfix.Cog = ppos->kCog;
    pfix.Sog = ppos->kSog;
    pfix.Var = ppos->kVar;
    pfix.FixTime = ppos->FixTime;
    pfix.nSats = ppos->nSats;

    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_NMEA_EVENTS)
                pic->m_pplugin->SetPositionFix(pfix);
        }
    }

    //    Send extended position fix to PlugIns at API 108 and later
    PlugIn_Position_Fix_Ex pfix_ex;
    pfix_ex.Lat = ppos->kLat;
    pfix_ex.Lon = ppos->kLon;
    pfix_ex.Cog = ppos->kCog;
    pfix_ex.Sog = ppos->kSog;
    pfix_ex.Var = ppos->kVar;
    pfix_ex.FixTime = ppos->FixTime;
    pfix_ex.nSats = ppos->nSats;
    pfix_ex.Hdt = ppos->kHdt;
    pfix_ex.Hdm = ppos->kHdm;

    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
        {
            if(pic->m_cap_flag & WANTS_NMEA_EVENTS)
            {
                switch(pic->m_api_version)
                {
                case 108:
                case 109:
                case 110:
                case 111:
                case 112:
                case 113:
                {
                    opencpn_plugin_18 *ppi = dynamic_cast<opencpn_plugin_18 *>(pic->m_pplugin);
                    if(ppi)
                        ppi->SetPositionFixEx(pfix_ex);
                    break;
                }

                default:
                    break;
                }
            }
        }
    }
}

void PlugInManager::SendResizeEventToAllPlugIns(int x, int y)
{
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
            pic->m_pplugin->ProcessParentResize(x, y);
    }
}

void PlugInManager::SetColorSchemeForAllPlugIns(ColorScheme cs)
{
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState)
            pic->m_pplugin->SetColorScheme((PI_ColorScheme)cs);
    }
}

void PlugInManager::NotifyAuiPlugIns(void)
{
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState && (pic->m_cap_flag & USES_AUI_MANAGER))
            pic->m_pplugin->UpdateAuiStatus();
    }
}

int PlugInManager::AddToolbarTool(wxString label, wxBitmap *bitmap, wxBitmap *bmpDisabled, wxItemKind kind,
                                  wxString shortHelp, wxString longHelp, wxObject *clientData, int position,
                                  int tool_sel, opencpn_plugin *pplugin )
{
    PlugInToolbarToolContainer *pttc = new PlugInToolbarToolContainer;
    pttc->label = label;

    if( !bitmap->IsOk() ) {
        ocpnStyle::Style*style = g_StyleManager->GetCurrentStyle();
        pttc->bitmap_day = new wxBitmap( style->GetIcon( _T("default_pi") ));
    } else {
        //  Force a non-reference copy of the bitmap from the PlugIn
        pttc->bitmap_day = new wxBitmap(*bitmap);
        pttc->bitmap_day->UnShare();
    }

    pttc->bitmap_Rollover = new wxBitmap(*pttc->bitmap_day);
    pttc->bitmap_Rollover->UnShare();

    pttc->bitmap_dusk = BuildDimmedToolBitmap(pttc->bitmap_day, 128);
    pttc->bitmap_night = BuildDimmedToolBitmap(pttc->bitmap_day, 32);
    
    pttc->kind = kind;
    pttc->shortHelp = shortHelp;
    pttc->longHelp = longHelp;
    pttc->clientData = clientData;
    pttc->position = position;
    pttc->m_pplugin = pplugin;
    pttc->tool_sel = tool_sel;
    pttc->b_viz = true;
    pttc->b_toggle = false;
    pttc->id = m_plugin_tool_id_next;



    m_PlugInToolbarTools.Add(pttc);

    m_plugin_tool_id_next++;

    return pttc->id;
}

void PlugInManager::RemoveToolbarTool(int tool_id)
{
    for(unsigned int i=0; i < m_PlugInToolbarTools.GetCount(); i++)
    {
        PlugInToolbarToolContainer *pttc = m_PlugInToolbarTools.Item(i);
        {
            if(pttc->id == tool_id)
            {
                m_PlugInToolbarTools.Remove(pttc);
                delete pttc;
                break;
            }
        }
    }

    pParent->RequestNewToolbar();
}

void PlugInManager::SetToolbarToolViz(int item, bool viz)
{
    for(unsigned int i=0; i < m_PlugInToolbarTools.GetCount(); i++)
    {
        PlugInToolbarToolContainer *pttc = m_PlugInToolbarTools.Item(i);
        {
            if(pttc->id == item)
            {
                pttc->b_viz = viz;
                
                //      Apply the change      
                pParent->RequestNewToolbar();
                
                break;
            }
        }
    }
}

void PlugInManager::SetToolbarItemState(int item, bool toggle)
{
    for(unsigned int i=0; i < m_PlugInToolbarTools.GetCount(); i++)
    {
        PlugInToolbarToolContainer *pttc = m_PlugInToolbarTools.Item(i);
        {
            if(pttc->id == item)
            {
                pttc->b_toggle = toggle;
                pParent->SetToolbarItemState(item, toggle);
                break;
            }
        }
    }
}

void PlugInManager::SetToolbarItemBitmaps(int item, wxBitmap *bitmap, wxBitmap *bmpRollover)
{
    for(unsigned int i=0; i < m_PlugInToolbarTools.GetCount(); i++)
    {
        PlugInToolbarToolContainer *pttc = m_PlugInToolbarTools.Item(i);
        {
            if(pttc->id == item)
            {
                delete pttc->bitmap_day;
                delete pttc->bitmap_dusk;
                delete pttc->bitmap_night;
                delete pttc->bitmap_Rollover;

                if( !bitmap->IsOk() ) {
                    ocpnStyle::Style*style = g_StyleManager->GetCurrentStyle();
                    pttc->bitmap_day = new wxBitmap( style->GetIcon( _T("default_pi") ));
                } else {
                    //  Force a non-reference copy of the bitmap from the PlugIn
                    pttc->bitmap_day = new wxBitmap(*bitmap);
                    pttc->bitmap_day->UnShare();
                }

                if( !bmpRollover->IsOk() ) {
                    ocpnStyle::Style*style = g_StyleManager->GetCurrentStyle();
                    pttc->bitmap_Rollover = new wxBitmap( style->GetIcon( _T("default_pi") ));
                } else {
                    //  Force a non-reference copy of the bitmap from the PlugIn
                    pttc->bitmap_Rollover = new wxBitmap(*bmpRollover);
                    pttc->bitmap_Rollover->UnShare();
                }
                
                pttc->bitmap_dusk = BuildDimmedToolBitmap(pttc->bitmap_day, 128);
                pttc->bitmap_night = BuildDimmedToolBitmap(pttc->bitmap_day, 32);

                pParent->SetToolbarItemBitmaps(item, pttc->bitmap_day, pttc->bitmap_Rollover);
                break;
            }
        }
    }

}

opencpn_plugin *PlugInManager::FindToolOwner(const int id)
{
    for(unsigned int i = 0 ; i < m_PlugInToolbarTools.GetCount() ; i++) {
        PlugInToolbarToolContainer *pc = m_PlugInToolbarTools.Item(i);
        if(id == pc->id)
            return pc->m_pplugin;
    }

    return NULL;
}

wxString PlugInManager::GetToolOwnerCommonName(const int id)
{
    opencpn_plugin *ppi = FindToolOwner(id);
    if(ppi) {
        for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++) {
            PlugInContainer *pic = plugin_array.Item(i);
            if(pic && (pic->m_pplugin == ppi)) return pic->m_common_name;
        }
    }

    return wxEmptyString;
}




wxString PlugInManager::GetLastError()
{
    return m_last_error_string;
}

wxBitmap *PlugInManager::BuildDimmedToolBitmap(wxBitmap *pbmp_normal, unsigned char dim_ratio)
{
    wxImage img_dup = pbmp_normal->ConvertToImage();

    if( !img_dup.IsOk() ) return NULL;

    if(dim_ratio < 200)
    {
        //  Create a dimmed version of the image/bitmap
        int gimg_width = img_dup.GetWidth();
        int gimg_height = img_dup.GetHeight();

        double factor = (double)(dim_ratio) / 256.0;

        for(int iy=0 ; iy < gimg_height ; iy++)
        {
            for(int ix=0 ; ix < gimg_width ; ix++)
            {
                if(!img_dup.IsTransparent(ix, iy))
                {
                    wxImage::RGBValue rgb(img_dup.GetRed(ix, iy), img_dup.GetGreen(ix, iy), img_dup.GetBlue(ix, iy));
                    wxImage::HSVValue hsv = wxImage::RGBtoHSV(rgb);
                    hsv.value = hsv.value * factor;
                    wxImage::RGBValue nrgb = wxImage::HSVtoRGB(hsv);
                    img_dup.SetRGB(ix, iy, nrgb.red, nrgb.green, nrgb.blue);
                }
            }
        }
    }

    //  Make a bitmap
    wxBitmap *ptoolBarBitmap;

#ifdef __WXMSW__
    wxBitmap tbmp(img_dup.GetWidth(),img_dup.GetHeight(),-1);
    wxMemoryDC dwxdc;
    dwxdc.SelectObject(tbmp);

    ptoolBarBitmap = new wxBitmap(img_dup, (wxDC &)dwxdc);
#else
    ptoolBarBitmap = new wxBitmap(img_dup);
#endif

    // store it
    return ptoolBarBitmap;
}


wxArrayString PlugInManager::GetPlugInChartClassNameArray(void)
{
    wxArrayString array;
    for(unsigned int i = 0 ; i < plugin_array.GetCount() ; i++)
    {
        PlugInContainer *pic = plugin_array.Item(i);
        if(pic->m_bEnabled && pic->m_bInitState &&
            ((pic->m_cap_flag & INSTALLS_PLUGIN_CHART) || (pic->m_cap_flag & INSTALLS_PLUGIN_CHART_GL)) )
        {
            wxArrayString carray = pic->m_pplugin->GetDynamicChartClassNameArray();

            for(unsigned int j = 0 ; j < carray.GetCount() ; j++)
                array.Add(carray.Item(j));

        }
    }

    //    Scrub the list for duplicates
    //    Corrects a flaw in BSB4 and NVC PlugIns
    unsigned int j=0;
    while(j < array.GetCount())
    {
        wxString test = array.Item(j);
        unsigned int k = j+1;
        while(k < array.GetCount())
        {
            if(test == array.Item(k))
            {
                array.RemoveAt(k);
                j = -1;
                break;
            }
            else
                k++;
        }

        j++;
    }


    return array;
}



//----------------------------------------------------------------------------------------------------------
//    The PlugIn CallBack API Implementation
//    The definitions of this API are found in ocpn_plugin.h
//----------------------------------------------------------------------------------------------------------


int InsertPlugInTool(wxString label, wxBitmap *bitmap, wxBitmap *bmpDisabled, wxItemKind kind,
                     wxString shortHelp, wxString longHelp, wxObject *clientData, int position,
                     int tool_sel, opencpn_plugin *pplugin)
{
    if(s_ppim)
        return s_ppim->AddToolbarTool(label, bitmap, bmpDisabled, kind,
                                      shortHelp, longHelp, clientData, position,
                                      tool_sel, pplugin );
    else
        return -1;
}


void  RemovePlugInTool(int tool_id)
{
    if(s_ppim)
        s_ppim->RemoveToolbarTool(tool_id);
}

void SetToolbarToolViz(int item, bool viz)
{
    if(s_ppim)
        s_ppim->SetToolbarToolViz(item, viz);
}

void SetToolbarItemState(int item, bool toggle)
{
    if(s_ppim)
        s_ppim->SetToolbarItemState(item, toggle);
}

void SetToolbarToolBitmaps(int item, wxBitmap *bitmap, wxBitmap *bmprollover)
{
    if(s_ppim)
        s_ppim->SetToolbarItemBitmaps(item, bitmap, bmprollover);
}

int AddCanvasContextMenuItem(wxMenuItem *pitem, opencpn_plugin *pplugin )
{
    if(s_ppim)
        return s_ppim->AddCanvasContextMenuItem(pitem, pplugin );
    else
        return -1;
}


void SetCanvasContextMenuItemViz(int item, bool viz)
{
    if(s_ppim)
        s_ppim->SetCanvasContextMenuItemViz(item, viz);
}

void SetCanvasContextMenuItemGrey(int item, bool grey)
{
    if(s_ppim)
        s_ppim->SetCanvasContextMenuItemGrey(item, grey);
}


void RemoveCanvasContextMenuItem(int item)
{
    if(s_ppim)
        s_ppim->RemoveCanvasContextMenuItem(item);
}




wxFileConfig *GetOCPNConfigObject(void)
{
    if(s_ppim)
        return pConfig;         // return the global application config object
    else
        return NULL;
}

wxWindow *GetOCPNCanvasWindow()
{
    wxWindow *pret = NULL;
    if(s_ppim)
    {
        MyFrame *pFrame = s_ppim->GetParentFrame();
        pret = (wxWindow *)pFrame->GetCanvasWindow();
    }
    return pret;
}

void RequestRefresh(wxWindow *win)
{
    if(win)
        win->Refresh();
}

void GetCanvasPixLL(PlugIn_ViewPort *vp, wxPoint *pp, double lat, double lon)
{
    //    Make enough of an application viewport to run its method....
    ViewPort ocpn_vp;
    ocpn_vp.clat = vp->clat;
    ocpn_vp.clon = vp->clon;
    ocpn_vp.m_projection_type = vp->m_projection_type;
    ocpn_vp.view_scale_ppm = vp->view_scale_ppm;
    ocpn_vp.skew = vp->skew;
    ocpn_vp.rotation = vp->rotation;
    ocpn_vp.pix_width = vp->pix_width;
    ocpn_vp.pix_height = vp->pix_height;

    wxPoint ret = ocpn_vp.GetPixFromLL(lat, lon);
    pp->x = ret.x;
    pp->y = ret.y;
}

void GetDoubleCanvasPixLL(PlugIn_ViewPort *vp, wxPoint2DDouble *pp, double lat, double lon)
{
    //    Make enough of an application viewport to run its method....
    ViewPort ocpn_vp;
    ocpn_vp.clat = vp->clat;
    ocpn_vp.clon = vp->clon;
    ocpn_vp.m_projection_type = vp->m_projection_type;
    ocpn_vp.view_scale_ppm = vp->view_scale_ppm;
    ocpn_vp.skew = vp->skew;
    ocpn_vp.rotation = vp->rotation;
    ocpn_vp.pix_width = vp->pix_width;
    ocpn_vp.pix_height = vp->pix_height;

    *pp = ocpn_vp.GetDoublePixFromLL(lat, lon);
}

void GetCanvasLLPix( PlugIn_ViewPort *vp, wxPoint p, double *plat, double *plon)
{
    //    Make enough of an application viewport to run its method....
    ViewPort ocpn_vp;
    ocpn_vp.clat = vp->clat;
    ocpn_vp.clon = vp->clon;
    ocpn_vp.m_projection_type = vp->m_projection_type;
    ocpn_vp.view_scale_ppm = vp->view_scale_ppm;
    ocpn_vp.skew = vp->skew;
    ocpn_vp.rotation = vp->rotation;
    ocpn_vp.pix_width = vp->pix_width;
    ocpn_vp.pix_height = vp->pix_height;

    return ocpn_vp.GetLLFromPix( p, plat, plon);
}

bool GetGlobalColor(wxString colorName, wxColour *pcolour)
{
    wxColour c = GetGlobalColor(colorName);
    *pcolour = c;

    return true;
}

wxFont *OCPNGetFont(wxString TextElement, int default_size)
{
    return FontMgr::Get().GetFont(TextElement, default_size);
}

wxString *GetpSharedDataLocation(void)
{
    return g_Platform->GetSharedDataDirPtr();
}

wxString *GetpPrivateApplicationDataLocation(void)
{
    return g_Platform->GetPrivateDataDirPtr();
}



ArrayOfPlugIn_AIS_Targets *GetAISTargetArray(void)
{
    if ( !g_pAIS )
        return NULL;


    ArrayOfPlugIn_AIS_Targets *pret = new ArrayOfPlugIn_AIS_Targets;

    //      Iterate over the AIS Target Hashmap
    AIS_Target_Hash::iterator it;

    AIS_Target_Hash *current_targets = g_pAIS->GetTargetList();

    for ( it = ( *current_targets ).begin(); it != ( *current_targets ).end(); ++it )
    {
        AIS_Target_Data *td = it->second;
        PlugIn_AIS_Target *ptarget = Create_PI_AIS_Target(td);
        pret->Add(ptarget);
    }

//  Test one alarm target
#if 0
    AIS_Target_Data td;
    td.n_alarm_state = AIS_ALARM_SET;
    PlugIn_AIS_Target *ptarget = Create_PI_AIS_Target(&td);
    pret->Add(ptarget);
#endif
    return pret;
}


wxAuiManager *GetFrameAuiManager(void)
{
    return g_pauimgr;
}

bool AddLocaleCatalog( wxString catalog )
{
#if wxUSE_XLOCALE || !wxCHECK_VERSION(3,0,0)
    
    if(plocale_def_lang){
        // Add this catalog to the persistent catalog array
        g_locale_catalog_array.Add(catalog);
        
        //  And then reload all catalogs.
        return ReloadLocale(); // plocale_def_lang->AddCatalog( catalog );
    }
    else
#endif        
        return false;
}

void PushNMEABuffer( wxString buf )
{
    OCPN_DataStreamEvent event( wxEVT_OCPN_DATASTREAM, 0 );
    std::string s = std::string( buf.mb_str() );
    event.SetNMEAString( s );
    event.SetStream( NULL );

    g_pMUX->AddPendingEvent( event );
}

wxXmlDocument GetChartDatabaseEntryXML(int dbIndex, bool b_getGeom)
{

    wxXmlDocument doc = ChartData->GetXMLDescription(dbIndex, b_getGeom);

    return doc;
}

bool UpdateChartDBInplace(wxArrayString dir_array,
                          bool b_force_update,
                          bool b_ProgressDialog)
{
    //    Make an array of CDI
    ArrayOfCDI ChartDirArray;
    for(unsigned int i=0 ; i < dir_array.GetCount(); i++)
    {
        wxString dirname = dir_array.Item(i);
        ChartDirInfo cdi;
        cdi.fullpath = dirname;
        cdi.magic_number = _T("");
        ChartDirArray.Add ( cdi );
    }

    bool b_ret =gFrame->UpdateChartDatabaseInplace(ChartDirArray,
                b_force_update, b_ProgressDialog,
                ChartData->GetDBFileName());

    ViewPort vp;
    gFrame->ChartsRefresh(-1, vp);

    return b_ret;
}

wxArrayString GetChartDBDirArrayString()
{
    return ChartData->GetChartDirArrayString();
}

int AddChartToDBInPlace( wxString &full_path, bool b_RefreshCanvas )
{
    // extract the path from the chart name
    wxFileName fn(full_path);
    wxString fdir = fn.GetPath();
    
    bool bret = false;
    if(ChartData){
        
        bret = ChartData->AddSingleChart( full_path );
        
        if(bret){
            // Save to disk
            pConfig->UpdateChartDirs( ChartData->GetChartDirArray() );
            ChartData->SaveBinary(ChartListFileName);
            

            //  Completely reload the chart database, for a fresh start
            ArrayOfCDI XnewChartDirArray;
            pConfig->LoadChartDirArray( XnewChartDirArray );
            delete ChartData;
            ChartData = new ChartDB();
            ChartData->LoadBinary(ChartListFileName, XnewChartDirArray);

            if(g_boptionsactive){
                g_options->UpdateDisplayedChartDirList(ChartData->GetChartDirArray());
            }
            
            
            if(b_RefreshCanvas || !cc1->GetQuiltMode()) {
                ViewPort vp;
                gFrame->ChartsRefresh(-1, vp);
            }
        }
    }
    return bret;
    
}

int RemoveChartFromDBInPlace( wxString &full_path )
{
    bool bret = false;
    if(ChartData){
        bret = ChartData->RemoveSingleChart( full_path );
        
    // Save to disk
        pConfig->UpdateChartDirs( ChartData->GetChartDirArray() );
        ChartData->SaveBinary(ChartListFileName);
    
    
    //  Completely reload the chart database, for a fresh start
        ArrayOfCDI XnewChartDirArray;
        pConfig->LoadChartDirArray( XnewChartDirArray );
        delete ChartData;
        ChartData = new ChartDB();
        ChartData->LoadBinary(ChartListFileName, XnewChartDirArray);
    
        if(g_boptionsactive){
            g_options->UpdateDisplayedChartDirList(ChartData->GetChartDirArray());
        }
        
        ViewPort vp;
        gFrame->ChartsRefresh(-1, vp);
    }
    
    return bret;
}


void SendPluginMessage( wxString message_id, wxString message_body )
{
    s_ppim->SendMessageToAllPlugins(message_id, message_body);

    //  We will send an event to the main application frame (gFrame)
    //  for informational purposes.
    //  Of course, gFrame is encouraged to use any or all the
    //  data flying by if judged useful and dependable....

    OCPN_MsgEvent Nevent(wxEVT_OCPN_MSG, 0);
    Nevent.SetID(message_id);
    Nevent.SetJSONText(message_body);
    gFrame->GetEventHandler()->AddPendingEvent( Nevent );

}

void DimeWindow(wxWindow *win)
{
    DimeControl(win);
}

void JumpToPosition(double lat, double lon, double scale)
{
    gFrame->JumpToPosition(lat, lon, scale);
}

/* API 1.9 */
wxScrolledWindow *AddOptionsPage( OptionsParentPI parent, wxString title )
{
    if (! g_pOptions) return NULL;

    size_t parentid;
    switch (parent) {
    case PI_OPTIONS_PARENT_DISPLAY:
        parentid = g_pOptions->m_pageDisplay;
    break;
    case PI_OPTIONS_PARENT_CONNECTIONS:
        parentid = g_pOptions->m_pageConnections;
    break;
    case PI_OPTIONS_PARENT_CHARTS:
        parentid = g_pOptions->m_pageCharts;
    break;
    case PI_OPTIONS_PARENT_SHIPS:
        parentid = g_pOptions->m_pageShips;
    break;
    case PI_OPTIONS_PARENT_UI:
        parentid = g_pOptions->m_pageUI;
    break;
    case PI_OPTIONS_PARENT_PLUGINS:
        parentid = g_pOptions->m_pagePlugins;
    break;
    default:
        wxLogMessage( _T("Error in PluginManager::AddOptionsPage: Unknown parent") );
        return NULL;
    break;
    }

    return g_pOptions->AddPage( parentid, title );
}

bool DeleteOptionsPage( wxScrolledWindow* page )
{
    if (! g_pOptions) return false;
    return g_pOptions->DeletePage( page );
}

bool DecodeSingleVDOMessage( const wxString& str, PlugIn_Position_Fix_Ex *pos, wxString *accumulator )
{
    if(!pos)
        return false;

    GenericPosDatEx gpd;
    AIS_Error nerr = AIS_GENERIC_ERROR;
    if(g_pAIS)
        nerr = g_pAIS->DecodeSingleVDO(str, &gpd, accumulator);
    if(nerr == AIS_NoError){
        pos->Lat = gpd.kLat;
        pos->Lon = gpd.kLon;
        pos->Cog = gpd.kCog;
        pos->Sog = gpd.kSog;
        pos->Hdt = gpd.kHdt;

        //  Fill in the dummy values
        pos->FixTime = 0;
        pos->Hdm = 1000;
        pos->Var = 1000;
        pos->nSats = 0;

        return true;
    }

    return false;
}

int GetChartbarHeight( void )
{
    if(g_bShowChartBar)
        return g_Piano->GetHeight();
    return 0;
}


bool GetRoutepointGPX( RoutePoint *pRoutePoint, char *buffer, unsigned int buffer_length)
{
    bool ret = false;
    
    NavObjectCollection1 *pgpx = new NavObjectCollection1;
    pgpx->AddGPXWaypoint( pRoutePoint);
    wxString gpxfilename = wxFileName::CreateTempFileName(wxT("gpx"));
    pgpx->SaveFile(gpxfilename);
    delete pgpx;
    
    wxFFile gpxfile( gpxfilename );
    wxString s;
    if( gpxfile.ReadAll( &s ) ) {
        if(s.Length() < buffer_length) {
            strncpy(buffer, (const char*)s.mb_str(wxConvUTF8), buffer_length -1);
            ret = true;
        }
    }

    gpxfile.Close();
    ::wxRemoveFile(gpxfilename);

    return ret;
}

bool GetActiveRoutepointGPX( char *buffer, unsigned int buffer_length )
{
    if( g_pRouteMan->IsAnyRouteActive() )
        return GetRoutepointGPX( g_pRouteMan->GetpActivePoint(), buffer, buffer_length);
    else
        return false;
}

void PositionBearingDistanceMercator_Plugin(double lat, double lon,
                                            double brg, double dist,
                                            double *dlat, double *dlon)
{
    PositionBearingDistanceMercator(lat, lon, brg, dist, dlat, dlon);
}

void DistanceBearingMercator_Plugin(double lat0, double lon0, double lat1, double lon1, double *brg, double *dist)
{
    DistanceBearingMercator( lat0, lon0, lat1, lon1, brg, dist);
}

double DistGreatCircle_Plugin(double slat, double slon, double dlat, double dlon)
{
    return DistGreatCircle(slat, slon, dlat, dlon);
}

void toTM_Plugin(float lat, float lon, float lat0, float lon0, double *x, double *y)
{
    toTM(lat, lon, lat0, lon0, x, y);
}

void fromTM_Plugin(double x, double y, double lat0, double lon0, double *lat, double *lon)
{
    fromTM(x, y, lat0, lon0, lat, lon);
}

void toSM_Plugin(double lat, double lon, double lat0, double lon0, double *x, double *y)
{
    toSM(lat, lon, lat0, lon0, x, y);
}

void fromSM_Plugin(double x, double y, double lat0, double lon0, double *lat, double *lon)
{
    fromSM(x, y, lat0, lon0, lat, lon);
}

void toSM_ECC_Plugin(double lat, double lon, double lat0, double lon0, double *x, double *y)
{
    toSM_ECC(lat, lon, lat0, lon0, x, y);
}

void fromSM_ECC_Plugin(double x, double y, double lat0, double lon0, double *lat, double *lon)
{
    fromSM_ECC(x, y, lat0, lon0, lat, lon);
}

double toUsrDistance_Plugin( double nm_distance, int unit )
{
    return toUsrDistance( nm_distance, unit );
}

double fromUsrDistance_Plugin( double usr_distance, int unit )
{
    return fromUsrDistance( usr_distance, unit );
}

double toUsrSpeed_Plugin( double kts_speed, int unit )
{
    return toUsrSpeed( kts_speed, unit );
}

double fromUsrSpeed_Plugin( double usr_speed, int unit )
{
    return fromUsrSpeed( usr_speed, unit );
}

wxString getUsrDistanceUnit_Plugin( int unit )
{
    return getUsrDistanceUnit( unit );
}

wxString getUsrSpeedUnit_Plugin( int unit )
{
    return getUsrSpeedUnit( unit );
}

bool PlugIn_GSHHS_CrossesLand(double lat1, double lon1, double lat2, double lon2)
{
    static bool loaded = false;
    if(!loaded) {
        gshhsCrossesLandInit();
        loaded = true;
    }

    return gshhsCrossesLand(lat1, lon1, lat2, lon2);
}


void PlugInPlaySound( wxString &sound_file )
{
    if(g_pi_manager) {
        g_pi_manager->m_plugin_sound.Stop();
        g_pi_manager->m_plugin_sound.UnLoad();

        g_pi_manager->m_plugin_sound.Create( sound_file );

        if( g_pi_manager->m_plugin_sound.IsOk() )
            g_pi_manager->m_plugin_sound.Play();
    }
}

// API 1.10 Route and Waypoint Support
//wxBitmap *FindSystemWaypointIcon( wxString& icon_name );

//      PlugInWaypoint implementation
PlugIn_Waypoint::PlugIn_Waypoint()
{
    m_HyperlinkList = NULL;
}

PlugIn_Waypoint::PlugIn_Waypoint(double lat, double lon,
                const wxString& icon_ident, const wxString& wp_name,
                const wxString& GUID )
{
    wxDateTime now = wxDateTime::Now();
    m_CreateTime = now.ToUTC();
    m_HyperlinkList = NULL;

    m_lat = lat;
    m_lon = lon;
    m_IconName = icon_ident;
    m_MarkName = wp_name;
    m_GUID = GUID;
}

PlugIn_Waypoint::~PlugIn_Waypoint()
{
}

//      PlugInRoute implementation
PlugIn_Route::PlugIn_Route(void )
{
    pWaypointList = new Plugin_WaypointList;
}

PlugIn_Route::~PlugIn_Route(void )
{
    pWaypointList->DeleteContents( false );            // do not delete Waypoints
    pWaypointList->Clear();

    delete pWaypointList;
}

//      PlugInTrack implementation
PlugIn_Track::PlugIn_Track(void )
{
    pWaypointList = new Plugin_WaypointList;
}

PlugIn_Track::~PlugIn_Track(void )
{
    pWaypointList->DeleteContents( false );            // do not delete Waypoints
    pWaypointList->Clear();

    delete pWaypointList;
}



wxString GetNewGUID( void )
{
    return GpxDocument::GetUUID();
}

bool AddCustomWaypointIcon( wxBitmap *pimage, wxString key, wxString description )
{
    pWayPointMan->ProcessIcon( *pimage, key, description );
    return true;
}


bool AddSingleWaypoint( PlugIn_Waypoint *pwaypoint, bool b_permanent)
{
    //  Validate the waypoint parameters a little bit

    //  GUID
    //  Make sure that this GUID is indeed unique in the Routepoint list
    bool b_unique = true;
    wxRoutePointListNode *prpnode = pWayPointMan->GetWaypointList()->GetFirst();
    while( prpnode ) {
        RoutePoint *prp = prpnode->GetData();

        if( prp->m_GUID == pwaypoint->m_GUID ) {
            b_unique = false;
            break;
        }
        prpnode = prpnode->GetNext(); //RoutePoint
    }

    if( !b_unique )
        return false;

    RoutePoint *pWP = new RoutePoint( pwaypoint->m_lat, pwaypoint->m_lon,
                                      pwaypoint->m_IconName, pwaypoint->m_MarkName,
                                      pwaypoint->m_GUID );

    pWP->m_bIsolatedMark = true;                      // This is an isolated mark


    //  Transcribe (clone) the html HyperLink List, if present
    if( pwaypoint->m_HyperlinkList ) {
        if( pwaypoint->m_HyperlinkList->GetCount() > 0 ) {
            wxPlugin_HyperlinkListNode *linknode = pwaypoint->m_HyperlinkList->GetFirst();
            while( linknode ) {
                Plugin_Hyperlink *link = linknode->GetData();

                Hyperlink* h = new Hyperlink();
                h->DescrText = link->DescrText;
                h->Link = link->Link;
                h->LType = link->Type;
            
                pWP->m_HyperlinkList->Append( h );

                linknode = linknode->GetNext();
            }
        }
    }

    pWP->m_MarkDescription = pwaypoint->m_MarkDescription;
    pWP->m_btemp = (b_permanent == false);

    pSelect->AddSelectableRoutePoint( pwaypoint->m_lat, pwaypoint->m_lon, pWP );
    if(b_permanent)
        pConfig->AddNewWayPoint( pWP, -1 );

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
        pRouteManagerDialog->UpdateWptListCtrl();

    return true;
}

bool DeleteSingleWaypoint( wxString &GUID )
{
    //  Find the RoutePoint
    bool b_found = false;
    RoutePoint *prp = pWayPointMan->FindRoutePointByGUID( GUID );

    if(prp)
        b_found = true;

    if( b_found ) {
        pWayPointMan->DestroyWaypoint( prp );
        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
            pRouteManagerDialog->UpdateWptListCtrl();
    }

    return b_found;
}


bool UpdateSingleWaypoint( PlugIn_Waypoint *pwaypoint )
{
    //  Find the RoutePoint
    bool b_found = false;
    RoutePoint *prp = pWayPointMan->FindRoutePointByGUID( pwaypoint->m_GUID );

    if(prp)
        b_found = true;

    if( b_found ) {
        double lat_save = prp->m_lat;
        double lon_save = prp->m_lon;

        prp->m_lat = pwaypoint->m_lat;
        prp->m_lon = pwaypoint->m_lon;
        prp->SetIconName( pwaypoint->m_IconName );
        prp->SetName( pwaypoint->m_MarkName );
        prp->m_MarkDescription = pwaypoint->m_MarkDescription;

        //  Transcribe (clone) the html HyperLink List, if present

        if( pwaypoint->m_HyperlinkList ) {
            prp->m_HyperlinkList->Clear();
            if( pwaypoint->m_HyperlinkList->GetCount() > 0 ) {
                wxPlugin_HyperlinkListNode *linknode = pwaypoint->m_HyperlinkList->GetFirst();
                while( linknode ) {
                    Plugin_Hyperlink *link = linknode->GetData();

                    Hyperlink* h = new Hyperlink();
                    h->DescrText = link->DescrText;
                    h->Link = link->Link;
                    h->LType = link->Type;
                    
                    prp->m_HyperlinkList->Append( h );

                    linknode = linknode->GetNext();
                }
            }
        }

        SelectItem *pFind = pSelect->FindSelection( lat_save, lon_save, SELTYPE_ROUTEPOINT );
        if( pFind ) {
            pFind->m_slat = pwaypoint->m_lat;             // update the SelectList entry
            pFind->m_slon = pwaypoint->m_lon;
        }

        if(!prp->m_btemp)
            pConfig->UpdateWayPoint( prp );

        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
            pRouteManagerDialog->UpdateWptListCtrl();
    }

    return b_found;
}

bool GetSingleWaypoint( wxString &GUID, PlugIn_Waypoint *pwaypoint )
{
    //  Find the RoutePoint
    bool b_found = false;
    RoutePoint *prp = pWayPointMan->FindRoutePointByGUID( GUID );

    if(!prp)
        return false;

    pwaypoint->m_lat = prp->m_lat;
    pwaypoint->m_lon = prp->m_lon;
    pwaypoint->m_IconName = prp->GetIconName();
    pwaypoint->m_MarkName = prp->GetName(  );
    pwaypoint->m_MarkDescription = prp->m_MarkDescription;

    //  Transcribe (clone) the html HyperLink List, if present

    if( prp->m_HyperlinkList ) {
        delete pwaypoint->m_HyperlinkList;
        pwaypoint->m_HyperlinkList = NULL;

        if( prp->m_HyperlinkList->GetCount() > 0 ) {
            pwaypoint->m_HyperlinkList = new Plugin_HyperlinkList;

            wxHyperlinkListNode *linknode = prp->m_HyperlinkList->GetFirst();
            while( linknode ) {
                Hyperlink *link = linknode->GetData();
                
                Plugin_Hyperlink* h = new Plugin_Hyperlink();
                h->DescrText = link->DescrText;
                h->Link = link->Link;
                h->Type = link->LType;
                    
                pwaypoint->m_HyperlinkList->Append( h );
                
                linknode = linknode->GetNext();
            }
        }
    }

    return true;
}

bool AddPlugInRoute( PlugIn_Route *proute, bool b_permanent )
{
    Route *route = new Route();

    PlugIn_Waypoint *pwp;
    RoutePoint *pWP_src;
    int ip = 0;

    wxPlugin_WaypointListNode *pwpnode = proute->pWaypointList->GetFirst();
    while( pwpnode ) {
        pwp = pwpnode->GetData();

        RoutePoint *pWP = new RoutePoint( pwp->m_lat, pwp->m_lon,
                                          pwp->m_IconName, pwp->m_MarkName,
                                          pwp->m_GUID );

        //  Transcribe (clone) the html HyperLink List, if present
        if( pwp->m_HyperlinkList ) {
            if( pwp->m_HyperlinkList->GetCount() > 0 ) {
                wxPlugin_HyperlinkListNode *linknode = pwp->m_HyperlinkList->GetFirst();
                while( linknode ) {
                    Plugin_Hyperlink *link = linknode->GetData();

                    Hyperlink* h = new Hyperlink();
                    h->DescrText = link->DescrText;
                    h->Link = link->Link;
                    h->LType = link->Type;
                    
                    pWP->m_HyperlinkList->Append( h );

                    linknode = linknode->GetNext();
                }
            }
        }

        pWP->m_MarkDescription = pwp->m_MarkDescription;
        pWP->m_bShowName = false;
        pWP->SetCreateTime(pwp->m_CreateTime);
        
        route->AddPoint( pWP );


        pSelect->AddSelectableRoutePoint( pWP->m_lat, pWP->m_lon, pWP );

        if(ip > 0)
            pSelect->AddSelectableRouteSegment( pWP_src->m_lat, pWP_src->m_lon, pWP->m_lat,
                                            pWP->m_lon, pWP_src, pWP, route );
        ip++;
        pWP_src = pWP;

        pwpnode = pwpnode->GetNext(); //PlugInWaypoint
    }

    route->m_RouteNameString = proute->m_NameString;
    route->m_RouteStartString = proute->m_StartString;
    route->m_RouteEndString = proute->m_EndString;
    route->m_GUID = proute->m_GUID;
    route->m_btemp = (b_permanent == false);

    pRouteList->Append( route );

    if(b_permanent)
        pConfig->AddNewRoute( route );

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
        pRouteManagerDialog->UpdateRouteListCtrl();

    return true;
}



bool DeletePlugInRoute( wxString& GUID )
{
    bool b_found = false;

    //  Find the Route
    Route *pRoute = g_pRouteMan->FindRouteByGUID( GUID );
    if(pRoute) {
        g_pRouteMan->DeleteRoute( pRoute );
        b_found = true;
    }
    return b_found;
}

bool UpdatePlugInRoute ( PlugIn_Route *proute )
{
    bool b_found = false;

    //  Find the Route
    Route *pRoute = g_pRouteMan->FindRouteByGUID( proute->m_GUID );
    if(pRoute)
        b_found = true;

    if(b_found) {
        bool b_permanent = (pRoute->m_btemp == false);
        g_pRouteMan->DeleteRoute( pRoute );

        b_found = AddPlugInRoute( proute, b_permanent );
    }

    return b_found;
}


bool AddPlugInTrack( PlugIn_Track *ptrack, bool b_permanent )
{
    Track *track = new Track();

    PlugIn_Waypoint *pwp;
    RoutePoint *pWP_src;
    int ip = 0;

    wxPlugin_WaypointListNode *pwpnode = ptrack->pWaypointList->GetFirst();
    while( pwpnode ) {
        pwp = pwpnode->GetData();

        RoutePoint *pWP = new RoutePoint( pwp->m_lat, pwp->m_lon,
                                          pwp->m_IconName, pwp->m_MarkName,
                                          pwp->m_GUID );


        pWP->m_MarkDescription = pwp->m_MarkDescription;
        pWP->m_bShowName = false;
        pWP->SetCreateTime( pwp->m_CreateTime );
        
        track->AddPoint( pWP );

        pSelect->AddSelectableRoutePoint( pWP->m_lat, pWP->m_lon, pWP );

        if(ip > 0)
            pSelect->AddSelectableRouteSegment( pWP_src->m_lat, pWP_src->m_lon, pWP->m_lat,
                                                pWP->m_lon, pWP_src, pWP, track );
        ip++;
        pWP_src = pWP;

        pwpnode = pwpnode->GetNext(); //PlugInWaypoint
    }

    track->m_RouteNameString = ptrack->m_NameString;
    track->m_RouteStartString = ptrack->m_StartString;
    track->m_RouteEndString = ptrack->m_EndString;
    track->m_GUID = ptrack->m_GUID;
    track->m_btemp = (b_permanent == false);

    pRouteList->Append( track );

    if(b_permanent)
        pConfig->AddNewRoute( track );

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
        pRouteManagerDialog->UpdateTrkListCtrl();

    return true;
}



bool DeletePluginTrack( wxString& GUID )
{
    bool b_found = false;

    //  Find the Route
    Route *pRoute = g_pRouteMan->FindRouteByGUID( GUID );
    if(pRoute) {
        g_pRouteMan->DeleteTrack( (Track *)pRoute );
        b_found = true;
    }

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
        pRouteManagerDialog->UpdateRouteListCtrl();

    return b_found;
 }

bool UpdatePlugInTrack ( PlugIn_Track *ptrack )
{
    bool b_found = false;

    //  Find the Track
    Route *pRoute = g_pRouteMan->FindRouteByGUID( ptrack->m_GUID );
    if(pRoute)
        b_found = true;

    if(b_found) {
        bool b_permanent = (pRoute->m_btemp == false);
        g_pRouteMan->DeleteTrack( (Track *)pRoute );

        b_found = AddPlugInTrack( ptrack, b_permanent );
    }

    return b_found;
}

void PlugInMultMatrixViewport ( PlugIn_ViewPort *vp )
{
#ifdef ocpnUSE_GL
    wxPoint point;
    GetCanvasPixLL(vp, &point, 0, 0);
    glTranslatef(point.x, point.y, 0);
    glScalef(vp->view_scale_ppm, vp->view_scale_ppm, 1);
    glRotatef(vp->rotation, 0, 0, 1);
#endif
}

void PlugInNormalizeViewport ( PlugIn_ViewPort *vp )
{
#ifdef ocpnUSE_GL
    vp->clat = vp->clon = 0;
    vp->view_scale_ppm = 1;
    vp->pix_width = vp->pix_height = 0;
    vp->rotation = vp->skew = 0;
#endif
}


//-----------------------------------------------------------------------------------------
//    The opencpn_plugin base class implementation
//-----------------------------------------------------------------------------------------

opencpn_plugin::~opencpn_plugin()
{}

int opencpn_plugin::Init(void)
{
    return 0;
}


bool opencpn_plugin::DeInit(void)
{
    return true;
}

int opencpn_plugin::GetAPIVersionMajor()
{
    return 1;
}

int opencpn_plugin::GetAPIVersionMinor()
{
    return 2;
}

int opencpn_plugin::GetPlugInVersionMajor()
{
    return 1;
}

int opencpn_plugin::GetPlugInVersionMinor()
{
    return 0;
}

wxBitmap *opencpn_plugin::GetPlugInBitmap()
{
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    return new wxBitmap(style->GetIcon( _T("default_pi") ) );
}

wxString opencpn_plugin::GetCommonName()
{
    return _T("BaseClassCommonName");
}

wxString opencpn_plugin::GetShortDescription()
{
    return _T("OpenCPN PlugIn Base Class");
}

wxString opencpn_plugin::GetLongDescription()
{
    return _T("OpenCPN PlugIn Base Class\n\
PlugInManager created this base class");
}



void opencpn_plugin::SetPositionFix(PlugIn_Position_Fix &pfix)
{}

void opencpn_plugin::SetNMEASentence(wxString &sentence)
{}

void opencpn_plugin::SetAISSentence(wxString &sentence)
{}

int opencpn_plugin::GetToolbarToolCount(void)
{
    return 0;
}

int opencpn_plugin::GetToolboxPanelCount(void)
{
    return 0;
}

void opencpn_plugin::SetupToolboxPanel(int page_sel, wxNotebook* pnotebook)
{}

void opencpn_plugin::OnCloseToolboxPanel(int page_sel, int ok_apply_cancel)
{}

void opencpn_plugin::ShowPreferencesDialog( wxWindow* parent )
{}

void opencpn_plugin::OnToolbarToolCallback(int id)
{}

void opencpn_plugin::OnContextMenuItemCallback(int id)
{}

bool opencpn_plugin::RenderOverlay(wxMemoryDC *dc, PlugIn_ViewPort *vp)
{
    return false;
}

void opencpn_plugin::SetCursorLatLon(double lat, double lon)
{}

void opencpn_plugin::SetCurrentViewPort(PlugIn_ViewPort &vp)
{}

void opencpn_plugin::SetDefaults(void)
{}

void opencpn_plugin::ProcessParentResize(int x, int y)
{}

void opencpn_plugin::SetColorScheme(PI_ColorScheme cs)
{}

void opencpn_plugin::UpdateAuiStatus(void)
{}


wxArrayString opencpn_plugin::GetDynamicChartClassNameArray()
{
    wxArrayString array;
    return array;
}


//    Opencpn_Plugin_16 Implementation
opencpn_plugin_16::opencpn_plugin_16(void *pmgr)
    : opencpn_plugin(pmgr)
{
}

opencpn_plugin_16::~opencpn_plugin_16(void)
{}

bool opencpn_plugin_16::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp)
{
    return false;
}

void opencpn_plugin_16::SetPluginMessage(wxString &message_id, wxString &message_body)
{}

//    Opencpn_Plugin_17 Implementation
opencpn_plugin_17::opencpn_plugin_17(void *pmgr)
    : opencpn_plugin(pmgr)
{
}

opencpn_plugin_17::~opencpn_plugin_17(void)
{}


bool opencpn_plugin_17::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp)
{
    return false;
}

bool opencpn_plugin_17::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp)
{
    return false;
}

void opencpn_plugin_17::SetPluginMessage(wxString &message_id, wxString &message_body)
{}


//    Opencpn_Plugin_18 Implementation
opencpn_plugin_18::opencpn_plugin_18(void *pmgr)
    : opencpn_plugin(pmgr)
{
}

opencpn_plugin_18::~opencpn_plugin_18(void)
{}


bool opencpn_plugin_18::RenderOverlay(wxDC &dc, PlugIn_ViewPort *vp)
{
    return false;
}

bool opencpn_plugin_18::RenderGLOverlay(wxGLContext *pcontext, PlugIn_ViewPort *vp)
{
    return false;
}

void opencpn_plugin_18::SetPluginMessage(wxString &message_id, wxString &message_body)
{}

void opencpn_plugin_18::SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix)
{}


//    Opencpn_Plugin_19 Implementation
opencpn_plugin_19::opencpn_plugin_19(void *pmgr)
    : opencpn_plugin_18(pmgr)
{
}

opencpn_plugin_19::~opencpn_plugin_19(void)
{
}

void opencpn_plugin_19::OnSetupOptions(void)
{
}

//    Opencpn_Plugin_110 Implementation
opencpn_plugin_110::opencpn_plugin_110(void *pmgr)
: opencpn_plugin_19(pmgr)
{
}

opencpn_plugin_110::~opencpn_plugin_110(void)
{
}

void opencpn_plugin_110::LateInit(void)
{
}

//    Opencpn_Plugin_111 Implementation
opencpn_plugin_111::opencpn_plugin_111(void *pmgr)
: opencpn_plugin_110(pmgr)
{
}

opencpn_plugin_111::~opencpn_plugin_111(void)
{
}


//    Opencpn_Plugin_112 Implementation
opencpn_plugin_112::opencpn_plugin_112(void *pmgr)
: opencpn_plugin_111(pmgr)
{
}

opencpn_plugin_112::~opencpn_plugin_112(void)
{
}

bool opencpn_plugin_112::MouseEventHook( wxMouseEvent &event )
{
    return false;
}

void opencpn_plugin_112::SendVectorChartObjectInfo(wxString &chart, wxString &feature, wxString &objname, double lat, double lon, double scale, int nativescale)
{
}

//    Opencpn_Plugin_113 Implementation
opencpn_plugin_113::opencpn_plugin_113(void *pmgr)
: opencpn_plugin_112(pmgr)
{
}

opencpn_plugin_113::~opencpn_plugin_113(void)
{
}

bool opencpn_plugin_113::KeyboardEventHook( wxKeyEvent &event )
{
    return false;
}

void opencpn_plugin_113::OnToolbarToolDownCallback(int id) {}
void opencpn_plugin_113::OnToolbarToolUpCallback(int id) {}

//          Helper and interface classes

//-------------------------------------------------------------------------------
//    PlugIn_AIS_Target Implementation
//-------------------------------------------------------------------------------

PlugIn_AIS_Target *Create_PI_AIS_Target(AIS_Target_Data *ptarget)
{
    PlugIn_AIS_Target *pret = new PlugIn_AIS_Target;

    pret->MMSI =            ptarget->MMSI;
    pret->Class =           ptarget->Class;
    pret->NavStatus =       ptarget->NavStatus;
    pret->SOG =             ptarget->SOG;
    pret->COG =             ptarget->COG;
    pret->HDG =             ptarget->HDG;
    pret->Lon =             ptarget->Lon;
    pret->Lat =             ptarget->Lat;
    pret->ROTAIS =          ptarget->ROTAIS;
    pret->ShipType =        ptarget->ShipType;
    pret->IMO =             ptarget->IMO;

    pret->Range_NM =        ptarget->Range_NM;
    pret->Brg =             ptarget->Brg;

    //      Per target collision parameters
    pret->bCPA_Valid =      ptarget->bCPA_Valid;
    pret->TCPA =            ptarget->TCPA;                     // Minutes
    pret->CPA =             ptarget->CPA;                      // Nautical Miles

    pret->alarm_state =     (plugin_ais_alarm_type)ptarget->n_alert_state;

    strncpy(pret->CallSign, ptarget->CallSign, sizeof(ptarget->CallSign));
    strncpy(pret->ShipName, ptarget->ShipName, sizeof(ptarget->ShipName));

    return pret;
}

//-------------------------------------------------------------------------------
//    PluginListPanel & PluginPanel Implementation
//-------------------------------------------------------------------------------

PluginListPanel::PluginListPanel( wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, ArrayOfPlugIns *pPluginArray )
//      :wxPanel( parent, id, pos, size, wxSUNKEN_BORDER|wxTAB_TRAVERSAL )
    :wxScrolledWindow( parent, id, pos, size, wxTAB_TRAVERSAL|wxVSCROLL )

{
    Hide();
    m_pPluginArray = pPluginArray;
    m_PluginSelected = NULL;

    m_pitemBoxSizer01 = new wxBoxSizer( wxVERTICAL );
    SetSizer( m_pitemBoxSizer01 );

    int max_dy = 0;

    for( unsigned int i = 0; i < pPluginArray->GetCount() ; i++ )
    {
        PluginPanel *pPluginPanel = new PluginPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, pPluginArray->Item( pPluginArray->GetCount() - i -1 ) );
        m_pitemBoxSizer01->Add( pPluginPanel, 0, wxEXPAND|wxALL, 0 );
        m_PluginItems.Add( pPluginPanel );

        wxStaticLine* itemStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
        m_pitemBoxSizer01->Add( itemStaticLine, 0, wxEXPAND|wxALL, 0 );

        //    When a child Panel is selected, its size grows to include "Preferences" and Enable" buttons.
        //    As a consequence, the vertical size of the ListPanel grows as well.
        //    Calculate and add a spacer to bottom of ListPanel so that initial ListPanel
        //    minimum size calculations account for selected Panel size growth.
        //    Sadly, this does not work right on wxQt.  So, just punt for now...

        pPluginPanel->SetSelected( false );       // start unselected
        m_pitemBoxSizer01->Layout();
        wxSize nsel_size = pPluginPanel->GetSize();

        pPluginPanel->SetSelected( true );        // switch to selected, a bit bigger
        m_pitemBoxSizer01->Layout();
        wxSize sel_size = pPluginPanel->GetSize();

#ifndef __WXQT__        
        pPluginPanel->SetSelected( false );       // reset to unselected
        m_pitemBoxSizer01->Layout();
#endif
        
        int dy = sel_size.y - nsel_size.y;
        dy += 10;                                 // fluff
        max_dy = wxMax(dy, max_dy);
    }

    m_pitemBoxSizer01->AddSpacer(max_dy);
    
    Show();
}

void PluginListPanel::UpdatePluginsOrder()
{
    m_pPluginArray->Clear();
    for( unsigned int i = 0 ; i < m_PluginItems.GetCount() ; i++ )
    {
        m_pPluginArray->Insert(m_PluginItems.Item(i)->GetPluginPtr(), 0);
    }
}

PluginListPanel::~PluginListPanel()
{
}

void PluginListPanel::UpdateSelections()
{
    for(unsigned int i=0 ; i < m_PluginItems.GetCount() ; i++) {
        PluginPanel *pPluginPanel = m_PluginItems.Item(i);
        if( pPluginPanel ){
            pPluginPanel->SetSelected( pPluginPanel->GetSelected() );
        }
    }
}
    
void PluginListPanel::SelectPlugin( PluginPanel *pi )
{
    if (m_PluginSelected == pi)
        return;

    if (m_PluginSelected)
        m_PluginSelected->SetSelected(false);

    m_PluginSelected = pi;
    m_parent->Layout();
    Refresh(false);
}

void PluginListPanel::MoveUp( PluginPanel *pi )
{
    int pos = m_PluginItems.Index( pi );
    if( pos == 0 ) //The first one can't be moved further up
        return;
    m_PluginItems.RemoveAt(pos);
    m_pitemBoxSizer01->Remove( pos * 2 + 1 );
    m_pitemBoxSizer01->Remove( pos * 2 );
    m_PluginItems.Insert( pi, pos - 1 );
    wxStaticLine* itemStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    m_pitemBoxSizer01->Insert( (pos - 1) * 2, itemStaticLine, 0, wxEXPAND|wxALL, 0 );
    m_pitemBoxSizer01->Insert( (pos - 1) * 2, pi, 0, wxEXPAND|wxALL, 0 );

    m_PluginSelected = pi;

    m_parent->Layout();
    Refresh(true);
}

void PluginListPanel::MoveDown( PluginPanel *pi )
{
    int pos = m_PluginItems.Index( pi );
    if( pos == (int)m_PluginItems.Count() - 1 ) //The last one can't be moved further down
        return;
    m_PluginItems.RemoveAt(pos);
    m_pitemBoxSizer01->Remove( pos * 2 + 1 );
    m_pitemBoxSizer01->Remove( pos * 2 );
    m_PluginItems.Insert( pi, pos + 1 );
    wxStaticLine* itemStaticLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    m_pitemBoxSizer01->Insert( (pos + 1) * 2 - 1, itemStaticLine, 0, wxEXPAND|wxALL, 0 );
    m_pitemBoxSizer01->Insert( (pos + 1) * 2, pi, 0, wxEXPAND|wxALL, 0 );

    m_PluginSelected = pi;

    m_parent->Layout();
    Refresh(false);
}

PluginPanel::PluginPanel(PluginListPanel *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, PlugInContainer *p_plugin)
    :wxPanel(parent, id, pos, size, wxBORDER_NONE)
{
    m_PluginListPanel = parent;
    m_pPlugin = p_plugin;
    m_bSelected = false;

    wxBoxSizer* itemBoxSizer01 = new wxBoxSizer(wxHORIZONTAL);
    SetSizer(itemBoxSizer01);
    Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(PluginPanel::OnPluginSelected), NULL, this);

    wxStaticBitmap *itemStaticBitmap = new wxStaticBitmap( this, wxID_ANY,
                                                           wxBitmap(m_pPlugin->m_bitmap->ConvertToImage().Copy()));
    itemBoxSizer01->Add(itemStaticBitmap, 0, wxEXPAND|wxALL, 5);
    itemStaticBitmap->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler( PluginPanel::OnPluginSelected ), NULL, this);
    wxBoxSizer* itemBoxSizer02 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer01->Add(itemBoxSizer02, 1, wxEXPAND|wxALL, 0);
    wxBoxSizer* itemBoxSizer03 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer02->Add(itemBoxSizer03);
    m_pName = new wxStaticText( this, wxID_ANY, m_pPlugin->m_common_name );
    m_pName->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler( PluginPanel::OnPluginSelected ), NULL, this);
    wxFont font = *wxNORMAL_FONT;
    font.SetWeight(wxFONTWEIGHT_BOLD);
    m_pName->SetFont(font);
    itemBoxSizer03->Add(m_pName, 0, wxEXPAND|wxALL, 5);
    m_pVersion = new wxStaticText( this, wxID_ANY,
                                   wxString::Format(_T("%d.%d"), m_pPlugin->m_version_major, m_pPlugin->m_version_minor) );
    itemBoxSizer03->Add(m_pVersion, 0, wxEXPAND|wxALL, 5);
    m_pVersion->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler( PluginPanel::OnPluginSelected ), NULL, this);
    m_pDescription = new wxStaticText( this, wxID_ANY, m_pPlugin->m_short_description );
    itemBoxSizer02->Add( m_pDescription, 0, wxEXPAND|wxALL, 5 );
    m_pDescription->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler( PluginPanel::OnPluginSelected ), NULL, this);

    m_pButtons = new wxFlexGridSizer(2);
    m_pButtons->AddGrowableCol(1);

//      m_pButtons = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer02->Add( m_pButtons, 1, wxEXPAND|wxALL, 0 );
    m_pButtonPreferences = new wxButton( this, wxID_ANY, _("Preferences"), wxDefaultPosition, wxDefaultSize, 0 );
    m_pButtons->Add( m_pButtonPreferences, 0, wxALIGN_LEFT|wxALL, 2);
    m_pButtonEnable = new wxButton( this, wxID_ANY, _("Disable"), wxDefaultPosition, wxDefaultSize, 0 );
    m_pButtons->Add(m_pButtonEnable, 0, wxALIGN_RIGHT|wxALL, 2);
    m_pButtonPreferences->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PluginPanel::OnPluginPreferences), NULL, this);
    m_pButtonEnable->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PluginPanel::OnPluginEnable), NULL, this);

    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    m_pButtonsUpDown = new wxBoxSizer(wxVERTICAL);
    m_pButtonUp = new wxBitmapButton( this, wxID_ANY, style->GetIcon( _T("up") ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_pButtonsUpDown->Add( m_pButtonUp, 0, wxALIGN_RIGHT|wxALL, 2);
    m_pButtonDown = new wxBitmapButton( this, wxID_ANY, style->GetIcon( _T("down") ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
    m_pButtonsUpDown->Add( m_pButtonDown, 0, wxALIGN_RIGHT|wxALL, 2);
    m_pButtonUp->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PluginPanel::OnPluginUp), NULL, this);
    m_pButtonDown->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(PluginPanel::OnPluginDown), NULL, this);
    itemBoxSizer01->Add(m_pButtonsUpDown, 0, wxALL, 0);
    
    SetSelected( m_bSelected );
}

PluginPanel::~PluginPanel()
{
}

void PluginPanel::OnPluginSelected( wxMouseEvent &event )
{
    SetSelected( true );
    m_PluginListPanel->SelectPlugin( this );
}

void PluginPanel::SetSelected( bool selected )
{
    m_bSelected = selected;
    if (selected)
    {
        SetBackgroundColour(GetGlobalColor(_T("DILG1")));
        m_pDescription->SetLabel( m_pPlugin->m_long_description );
        m_pButtons->Show(true);
        m_pButtonsUpDown->Show(true);
        Layout();
        //FitInside();
    }
    else
    {
        SetBackgroundColour(GetGlobalColor(_T("DILG0")));
        m_pDescription->SetLabel( m_pPlugin->m_short_description );
        m_pButtons->Show(false);
        m_pButtonsUpDown->Show(false);
        Layout();
        //FitInside();
    }
    // StaticText color change upon selection
    SetEnabled( m_pPlugin->m_bEnabled );
}

void PluginPanel::OnPluginPreferences( wxCommandEvent& event )
{
    if (m_pPlugin->m_bEnabled && m_pPlugin->m_bInitState && (m_pPlugin->m_cap_flag & WANTS_PREFERENCES) )
    {
        m_pPlugin->m_pplugin->ShowPreferencesDialog( this );
    }
}

void PluginPanel::OnPluginEnable( wxCommandEvent& event )
{
    SetEnabled(!m_pPlugin->m_bEnabled);
}

void PluginPanel::SetEnabled( bool enabled )
{
    if (m_pPlugin->m_bEnabled != enabled)
    {
        m_pPlugin->m_bEnabled = enabled;
        if(s_ppim)
            s_ppim->UpdatePlugIns();
        NotifySetupOptionsPlugin( m_pPlugin );
    }
    if (!enabled && !m_bSelected)
    {
        m_pName->SetForegroundColour(*wxLIGHT_GREY);
        m_pVersion->SetForegroundColour(*wxLIGHT_GREY);
        m_pDescription->SetForegroundColour(*wxLIGHT_GREY);
        m_pButtonEnable->SetLabel(_("Enable"));
    }
    else
    {
        m_pName->SetForegroundColour(*wxBLACK);
        m_pVersion->SetForegroundColour(*wxBLACK);
        m_pDescription->SetForegroundColour(*wxBLACK);
        if ( enabled )
            m_pButtonEnable->SetLabel(_("Disable"));
        else
            m_pButtonEnable->SetLabel(_("Enable"));
    }
    m_pButtonPreferences->Enable( enabled && (m_pPlugin->m_cap_flag & WANTS_PREFERENCES) );
}

void PluginPanel::OnPluginUp( wxCommandEvent& event )
{
    m_PluginListPanel->MoveUp( this );
}

void PluginPanel::OnPluginDown( wxCommandEvent& event )
{
    m_PluginListPanel->MoveDown( this );
}



// ----------------------------------------------------------------------------
// PlugInChartBase Implmentation
//  This class is the base class for Plug-able chart types
// ----------------------------------------------------------------------------

PlugInChartBase::PlugInChartBase()
{}

PlugInChartBase::~PlugInChartBase()
{}

wxString PlugInChartBase::GetFileSearchMask(void)
{
    return _T("");
}

int PlugInChartBase::Init( const wxString& name, int init_flags )
{
    return 0;
}

//    Accessors

double PlugInChartBase::GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom)
{
    return 1.0;
}

double PlugInChartBase::GetNormalScaleMax(double canvas_scale_factor, int canvas_width)
{
    return 2.0e7;
}

bool PlugInChartBase::GetChartExtent(ExtentPI *pext)
{
    return false;
}


wxBitmap& PlugInChartBase::RenderRegionView(const PlugIn_ViewPort& VPoint,
        const wxRegion &Region)
{
    return wxNullBitmap;
}


bool PlugInChartBase::AdjustVP(PlugIn_ViewPort &vp_last, PlugIn_ViewPort &vp_proposed)
{
    return false;
}

void PlugInChartBase::GetValidCanvasRegion(const PlugIn_ViewPort& VPoint, wxRegion *pValidRegion)
{}

void PlugInChartBase::SetColorScheme(int cs, bool bApplyImmediate)
{}

double PlugInChartBase::GetNearestPreferredScalePPM(double target_scale_ppm)
{
    return 1.0;
}

wxBitmap *PlugInChartBase::GetThumbnail(int tnx, int tny, int cs)
{
    return NULL;
}

void PlugInChartBase::ComputeSourceRectangle(const PlugIn_ViewPort &vp, wxRect *pSourceRect)
{}

double PlugInChartBase::GetRasterScaleFactor()
{
    return 1.0;
}

bool PlugInChartBase::GetChartBits( wxRect& source, unsigned char *pPix, int sub_samp )
{
    return false;
}

int PlugInChartBase::GetSize_X()
{
    return 1;
}

int PlugInChartBase::GetSize_Y()
{
    return 1;
}

void PlugInChartBase::latlong_to_chartpix(double lat, double lon, double &pixx, double &pixy)
{}


// ----------------------------------------------------------------------------
// PlugInChartBaseGL Implementation
//  
// ----------------------------------------------------------------------------

PlugInChartBaseGL::PlugInChartBaseGL()
{}

PlugInChartBaseGL::~PlugInChartBaseGL()
{}

int PlugInChartBaseGL::RenderRegionViewOnGL( const wxGLContext &glc, const PlugIn_ViewPort& VPoint,
                                  const wxRegion &Region, bool b_use_stencil )
{
    return 0;
}

ListOfPI_S57Obj *PlugInChartBaseGL::GetObjRuleListAtLatLon(float lat, float lon, float select_radius,
                                                           PlugIn_ViewPort *VPoint)
{
    return NULL;
}

wxString PlugInChartBaseGL::CreateObjDescriptions( ListOfPI_S57Obj* obj_list )
{
    return _T("");
}

int PlugInChartBaseGL::GetNoCOVREntries()
{
    return 0;
}

int PlugInChartBaseGL::GetNoCOVRTablePoints(int iTable)
{
    return 0;
}

int  PlugInChartBaseGL::GetNoCOVRTablenPoints(int iTable)
{
    return 0;
}

float *PlugInChartBaseGL::GetNoCOVRTableHead(int iTable)
{ 
    return 0;
}


// ----------------------------------------------------------------------------
// ChartPlugInWrapper Implementation
//    This class is a wrapper/interface to PlugIn charts(PlugInChartBase)
// ----------------------------------------------------------------------------


ChartPlugInWrapper::ChartPlugInWrapper()
{}

ChartPlugInWrapper::ChartPlugInWrapper(const wxString &chart_class)
{
    m_ppo = ::wxCreateDynamicObject(chart_class);
    m_ppicb = wxDynamicCast(m_ppo, PlugInChartBase);
}

ChartPlugInWrapper::~ChartPlugInWrapper()
{
    if(m_ppicb)
        delete m_ppicb;
}

wxString ChartPlugInWrapper::GetFileSearchMask(void)
{
    if(m_ppicb)
        return m_ppicb->GetFileSearchMask();
    else
        return _T("");
}

InitReturn ChartPlugInWrapper::Init( const wxString& name, ChartInitFlag init_flags )
{
    if(m_ppicb)
    {
        InitReturn ret_val = (InitReturn)m_ppicb->Init(name, (int)init_flags);

        //    Here we transcribe all the required wrapped member elements up into the chartbase object which is the parent of this class
        if(ret_val == INIT_OK)
        {
            m_FullPath = m_ppicb->GetFullPath();
            m_ChartType = (ChartTypeEnum)m_ppicb->GetChartType();
            m_ChartFamily = (ChartFamilyEnum)m_ppicb->GetChartFamily();
            m_projection = (OcpnProjType)m_ppicb->GetChartProjection();
            m_EdDate = m_ppicb->GetEditionDate();
            m_Name = m_ppicb->GetName();
            m_ID = m_ppicb->GetID();
            m_DepthUnits = m_ppicb->GetDepthUnits();
            m_SoundingsDatum = m_ppicb->GetSoundingsDatum();
            m_datum_str = m_ppicb->GetDatumString();
            m_SE = m_ppicb->GetSE();
            m_EdDate = m_ppicb->GetEditionDate();
            m_ExtraInfo = m_ppicb->GetExtraInfo();
            Chart_Error_Factor = m_ppicb->GetChartErrorFactor();
            m_depth_unit_id = (ChartDepthUnitType)m_ppicb->GetDepthUnitId();
            m_Chart_Skew = m_ppicb->GetChartSkew();
            m_Chart_Scale = m_ppicb->GetNativeScale();

            bReadyToRender = m_ppicb->IsReadyToRender();

        }


        //  PlugIn may invoke wxExecute(), which steals the keyboard focus
        //  So take it back
        if(cc1)
            cc1->SetFocus();
        
        return ret_val;
    }
    else
        return INIT_FAIL_REMOVE;
}


//    Accessors
int ChartPlugInWrapper::GetCOVREntries()
{
    if(m_ppicb)
        return m_ppicb->GetCOVREntries();
    else
        return 0;
}

int ChartPlugInWrapper::GetCOVRTablePoints(int iTable)
{
    if(m_ppicb)
        return m_ppicb->GetCOVRTablePoints(iTable);
    else
        return 0;
}

int  ChartPlugInWrapper::GetCOVRTablenPoints(int iTable)
{
    if(m_ppicb)
        return m_ppicb->GetCOVRTablenPoints(iTable);
    else
        return 0;
}

float *ChartPlugInWrapper::GetCOVRTableHead(int iTable)
{
    if(m_ppicb)
        return m_ppicb->GetCOVRTableHead(iTable);
    else
        return 0;
}

//      TODO
//      PlugIn chart types do not properly support NoCovr Regions
//      Proper fix is to update PlugIn Chart Type API
//      Derive an extended PlugIn chart class from existing class,
//      and use some kind of RTTI to figure out which class to call.
int ChartPlugInWrapper::GetNoCOVREntries()
{
    if(m_ppicb) {
        PlugInChartBaseGL *ppicbgl = dynamic_cast<PlugInChartBaseGL *>(m_ppicb);
        if(ppicbgl){
            return ppicbgl->GetNoCOVREntries();
        }
    }
    return 0;
}

int ChartPlugInWrapper::GetNoCOVRTablePoints(int iTable)
{
    if(m_ppicb) {
        PlugInChartBaseGL *ppicbgl = dynamic_cast<PlugInChartBaseGL *>(m_ppicb);
        if(ppicbgl){
            return ppicbgl->GetNoCOVRTablePoints(iTable);
        }
    }
    return 0;
}

int  ChartPlugInWrapper::GetNoCOVRTablenPoints(int iTable)
{
    if(m_ppicb) {
        PlugInChartBaseGL *ppicbgl = dynamic_cast<PlugInChartBaseGL *>(m_ppicb);
        if(ppicbgl){
            return ppicbgl->GetNoCOVRTablenPoints(iTable);
        }
    }
    return 0;
}

float *ChartPlugInWrapper::GetNoCOVRTableHead(int iTable)
{
    if(m_ppicb) {
        PlugInChartBaseGL *ppicbgl = dynamic_cast<PlugInChartBaseGL *>(m_ppicb);
        if(ppicbgl){
            return ppicbgl->GetNoCOVRTableHead(iTable);
        }
    }
    return 0;
}

bool ChartPlugInWrapper::GetChartExtent(Extent *pext)
{
    if(m_ppicb)
    {
        ExtentPI xpi;
        if(m_ppicb->GetChartExtent(&xpi))
        {
            pext->NLAT = xpi.NLAT;
            pext->SLAT = xpi.SLAT;
            pext->ELON = xpi.ELON;
            pext->WLON = xpi.WLON;

            return true;
        }
        else
            return false;
    }
    else
        return false;
}

ThumbData *ChartPlugInWrapper::GetThumbData(int tnx, int tny, float lat, float lon)
{
    if(m_ppicb)
    {

//    Create the bitmap if needed, doing a deep copy from the Bitmap owned by the PlugIn Chart
        if(!pThumbData->pDIBThumb)
        {
            wxBitmap *pBMPOwnedByChart = m_ppicb->GetThumbnail(tnx, tny, m_global_color_scheme);
            if( pBMPOwnedByChart ) {
                wxImage img = pBMPOwnedByChart->ConvertToImage();
                pThumbData->pDIBThumb = new wxBitmap(img);
            }
            else
                pThumbData->pDIBThumb = NULL;
            
        }

        pThumbData->Thumb_Size_X = tnx;
        pThumbData->Thumb_Size_Y = tny;

        /*
        //    Plot the supplied Lat/Lon on the thumbnail
                    int divx = m_ppicb->Size_X / tnx;
                    int divy = m_ppicb->Size_Y / tny;

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
        */
        pThumbData->ShipX = 0;
        pThumbData->ShipY = 0;

        return pThumbData;
    }
    else
        return NULL;
}

ThumbData *ChartPlugInWrapper::GetThumbData()
{
    return pThumbData;
}

bool ChartPlugInWrapper::UpdateThumbData(double lat, double lon)
{
    return true;
}

double ChartPlugInWrapper::GetNormalScaleMin(double canvas_scale_factor, bool b_allow_overzoom)
{
    if(m_ppicb)
        return m_ppicb->GetNormalScaleMin(canvas_scale_factor, b_allow_overzoom);
    else
        return 1.0;
}

double ChartPlugInWrapper::GetNormalScaleMax(double canvas_scale_factor, int canvas_width)
{
    if(m_ppicb)
        return m_ppicb->GetNormalScaleMax(canvas_scale_factor, canvas_width);
    else
        return 2.0e7;
}

bool ChartPlugInWrapper::RenderRegionViewOnGL(const wxGLContext &glc, const ViewPort& VPoint, const OCPNRegion &Region)
{
#ifdef ocpnUSE_GL
    if(m_ppicb)
    {
        gs_plib_flags = 0;               // reset the CAPs flag
        PlugIn_ViewPort pivp = CreatePlugInViewport( VPoint);
        OCPNRegion rg = Region;
        if(rg.IsOk())
        {
            wxRegion r = rg.ConvertTowxRegion();
            PlugInChartBaseGL *ppicb_gl = dynamic_cast<PlugInChartBaseGL*>(m_ppicb);
            if(ppicb_gl){
                ppicb_gl->RenderRegionViewOnGL( glc, pivp, r, glChartCanvas::s_b_useStencil);
            }
            return true;
        }
    }
    else
        return false;
#endif    
    return true;
}


bool ChartPlugInWrapper::RenderRegionViewOnDC(wxMemoryDC& dc, const ViewPort& VPoint,
        const OCPNRegion &Region)
{
    if(m_ppicb)
    {
        gs_plib_flags = 0;               // reset the CAPs flag
        PlugIn_ViewPort pivp = CreatePlugInViewport( VPoint);
        OCPNRegion rg = Region;
        if(rg.IsOk())
        {
            wxRegion r = rg.ConvertTowxRegion();
            dc.SelectObject(m_ppicb->RenderRegionView( pivp, r));
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

bool ChartPlugInWrapper::AdjustVP(ViewPort &vp_last, ViewPort &vp_proposed)
{
    if(m_ppicb)
    {
        PlugIn_ViewPort pivp_last = CreatePlugInViewport( vp_last);
        PlugIn_ViewPort pivp_proposed = CreatePlugInViewport( vp_proposed);
        return m_ppicb->AdjustVP(pivp_last, pivp_proposed);
    }
    else
        return false;
}

void ChartPlugInWrapper::GetValidCanvasRegion(const ViewPort& VPoint, OCPNRegion *pValidRegion)
{
    if(m_ppicb)
    {
        PlugIn_ViewPort pivp = CreatePlugInViewport( VPoint);
        // currently convert using wxRegion,
        // this should be changed as wxRegion is proven unstable/buggy on various platforms
        wxRegion region;
        m_ppicb->GetValidCanvasRegion(pivp, &region);
        *pValidRegion = OCPNRegion(region);
    }

    return;
}


void ChartPlugInWrapper::SetColorScheme(ColorScheme cs, bool bApplyImmediate)
{
    if(m_ppicb)
        m_ppicb->SetColorScheme(cs, bApplyImmediate);
}


double ChartPlugInWrapper::GetNearestPreferredScalePPM(double target_scale_ppm)
{
    if(m_ppicb)
        return m_ppicb->GetNearestPreferredScalePPM(target_scale_ppm);
    else
        return 1.0;
}


void ChartPlugInWrapper::ComputeSourceRectangle(const ViewPort &VPoint, wxRect *pSourceRect)
{
    if(m_ppicb)
    {
        PlugIn_ViewPort pivp = CreatePlugInViewport( VPoint);
        m_ppicb->ComputeSourceRectangle(pivp, pSourceRect);
    }
}

double ChartPlugInWrapper::GetRasterScaleFactor()
{
    if(m_ppicb)
        return m_ppicb->GetRasterScaleFactor();
    else
        return 1.0;
}

bool ChartPlugInWrapper::GetChartBits( wxRect& source, unsigned char *pPix, int sub_samp )
{
    wxCriticalSectionLocker locker(m_critSect);
    
    if(m_ppicb)

        return m_ppicb->GetChartBits( source, pPix, sub_samp );
    else
        return false;
}

int ChartPlugInWrapper::GetSize_X()
{
    if(m_ppicb)
        return m_ppicb->GetSize_X();
    else
        return 1;
}

int ChartPlugInWrapper::GetSize_Y()
{
    if(m_ppicb)
        return m_ppicb->GetSize_Y();
    else
        return 1;
}

void ChartPlugInWrapper::latlong_to_chartpix(double lat, double lon, double &pixx, double &pixy)
{
    if(m_ppicb)
        m_ppicb->latlong_to_chartpix(lat, lon, pixx, pixy);
}


/* API 1.11  */

/* API 1.11  adds some more common functions to avoid unnecessary code duplication */

wxString toSDMM_PlugIn(int NEflag, double a, bool hi_precision)
{
    return toSDMM(NEflag, a, hi_precision);
}

wxColour GetBaseGlobalColor(wxString colorName)
{
    return GetGlobalColor( colorName );
}


int OCPNMessageBox_PlugIn(wxWindow *parent,
                          const wxString& message,
                          const wxString& caption,
                          int style, int x, int y)
{
    return OCPNMessageBox( parent, message, caption, style, 100, x, y );
}

wxString GetOCPN_ExePath( void )
{
    return g_Platform->GetExePath();
}

wxString *GetpPlugInLocation()
{
    return g_Platform->GetPluginDirPtr();
}

wxString GetWritableDocumentsDir( void )
{
    return g_Platform->GetWritableDocumentsDir();
}


wxString GetPlugInPath(opencpn_plugin *pplugin)
{
    wxString ret_val;
    ArrayOfPlugIns *pi_array = g_pi_manager->GetPlugInArray();
    for(unsigned int i = 0 ; i < pi_array->GetCount() ; i++)
    {
        PlugInContainer *pic = pi_array->Item(i);
        if(pic->m_pplugin == pplugin )
        {
            ret_val =pic->m_plugin_file;
            break;
        }
    }
    
    return ret_val;
}

//      API 1.11 Access to Vector PlugIn charts

ListOfPI_S57Obj *PlugInManager::GetPlugInObjRuleListAtLatLon( ChartPlugInWrapper *target, float zlat, float zlon,
                                                 float SelectRadius, const ViewPort& vp )
{
    if(target) {
        PlugInChartBaseGL *picbgl = dynamic_cast <PlugInChartBaseGL *>(target->GetPlugInChart());
        if(picbgl){
            PlugIn_ViewPort pi_vp = CreatePlugInViewport( vp );
            ListOfPI_S57Obj *piol = picbgl->GetObjRuleListAtLatLon(zlat, zlon, SelectRadius, &pi_vp);

            return piol;
        }
        else
            return NULL;
    }
    else
        return NULL;
}

wxString PlugInManager::CreateObjDescriptions( ChartPlugInWrapper *target, ListOfPI_S57Obj *rule_list )
{
    wxString ret_str;
    if(target) {
        PlugInChartBaseGL *picbgl = dynamic_cast <PlugInChartBaseGL *>(target->GetPlugInChart());
        if(picbgl){
            ret_str = picbgl->CreateObjDescriptions( rule_list );
        }
    }
    
    return ret_str;
}



//      API 1.11 Access to S52 PLIB
wxString PI_GetPLIBColorScheme()
{
    return _T("");           //ps52plib->GetPLIBColorScheme()
}

int PI_GetPLIBDepthUnitInt()
{
    if(ps52plib)
        return ps52plib->m_nDepthUnitDisplay;
    else
        return 0; 
}

int PI_GetPLIBSymbolStyle()
{
    if(ps52plib)
        return ps52plib->m_nSymbolStyle;
    else
        return 0;  
}

int PI_GetPLIBBoundaryStyle()
{
    if(ps52plib)
        return ps52plib->m_nBoundaryStyle;
    else        
        return 0;
}

bool PI_PLIBObjectRenderCheck( PI_S57Obj *pObj, PlugIn_ViewPort *vp )
{ 
    if(ps52plib) {
        //  Create and populate a compatible s57 Object
        S57Obj cobj;
        chart_context ctx;
        CreateCompatibleS57Object( pObj, &cobj, &ctx );

        ViewPort cvp = CreateCompatibleViewport( *vp );
        
        S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;
        
        //  Create and populate a minimally compatible object container
        ObjRazRules rzRules;
        rzRules.obj = &cobj;
        rzRules.LUP = pContext->LUP;
        rzRules.sm_transform_parms = 0;
        rzRules.child = NULL;
        rzRules.next = NULL;
        
        if(pContext->LUP)
            return ps52plib->ObjectRenderCheck( &rzRules, &cvp );
        else
            return false;
    }
    else
        return false;
    
}

int PI_GetPLIBStateHash()
{
    if(ps52plib)
        return ps52plib->GetStateHash();
    else
        return 0;
}

void CreateCompatibleS57Object( PI_S57Obj *pObj, S57Obj *cobj, chart_context *pctx )
{
    strncpy(cobj->FeatureName, pObj->FeatureName, 8);
    cobj->Primitive_type = (GeoPrim_t)pObj->Primitive_type;
    cobj->att_array = pObj->att_array;
    cobj->attVal = pObj->attVal;
    cobj->n_attr = pObj->n_attr;    
    
    cobj->x = pObj->x;
    cobj->y = pObj->y;
    cobj->z = pObj->z;
    cobj->npt = pObj->npt;
    
    cobj->iOBJL = pObj->iOBJL;
    cobj->Index = pObj->Index;
    
    cobj->geoPt = (pt *)pObj->geoPt;
    cobj->geoPtz = pObj->geoPtz;
    cobj->geoPtMulti = pObj->geoPtMulti;
    
    cobj->m_lat = pObj->m_lat;
    cobj->m_lon = pObj->m_lon;
    
    cobj->m_DisplayCat = (DisCat)pObj->m_DisplayCat;
    cobj->x_rate = pObj->x_rate;
    cobj->y_rate = pObj->y_rate;
    cobj->x_origin = pObj->x_origin;
    cobj->y_origin = pObj->y_origin;
    
    cobj->Scamin = pObj->Scamin;
    cobj->nRef = pObj->nRef;
    cobj->bIsAton = pObj->bIsAton;
    cobj->bIsAssociable = pObj->bIsAssociable;
    
    cobj->m_n_lsindex = pObj->m_n_lsindex;
    cobj->m_lsindex_array = pObj->m_lsindex_array;
    cobj->m_n_edge_max_points = pObj->m_n_edge_max_points;
    
    if(gs_plib_flags & PLIB_CAPS_OBJSEGLIST)
        cobj->m_ls_list = (line_segment_element *)pObj->m_ls_list;          // note the cast, assumes in-sync layout
    else   
        cobj->m_ls_list = 0;
        
    if(gs_plib_flags & PLIB_CAPS_OBJCATMUTATE)
        cobj->m_bcategory_mutable = pObj->m_bcategory_mutable;
    else
        cobj->m_bcategory_mutable = true;                       // assume all objects are mutable

    cobj->m_DPRI = -1;                              // default is unassigned, fixed at render time
    if(gs_plib_flags & PLIB_CAPS_OBJCATMUTATE){
        if(pObj->m_DPRI == -1){
            S52PLIB_Context *pCtx = (S52PLIB_Context *)pObj->S52_Context;
            if(pCtx->LUP)
                cobj->m_DPRI = pCtx->LUP->DPRI - '0';
        }
        else
            cobj->m_DPRI = pObj->m_DPRI;
    }
    
        
 
    cobj->pPolyTessGeo = ( PolyTessGeo* )pObj->pPolyTessGeo;
    cobj->m_chart_context = (chart_context *)pObj->m_chart_context;
    cobj->auxParm0 = 0;
    cobj->auxParm1 = 0;
    cobj->auxParm2 = 0;
    cobj->auxParm3 = 0;
    
    S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;
    
    cobj->bBBObj_valid = pContext->bBBObj_valid;
    if( pContext->bBBObj_valid )
        cobj->BBObj = pContext->BBObj;
    
    cobj->CSrules = pContext->CSrules;
    cobj->bCS_Added = pContext->bCS_Added;
    
    cobj->FText = pContext->FText;
    cobj->bFText_Added = pContext->bFText_Added;
    cobj->rText = pContext->rText;
    
    cobj->bIsClone = true;              // Protect cloned object pointers in S57Obj dtor

    if(pctx){
        cobj->m_chart_context = pctx;
        chart_context *ppctx = (chart_context *)pObj->m_chart_context;
        
        if( ppctx ){
            cobj->m_chart_context->m_pvc_hash = ppctx->m_pvc_hash;
            cobj->m_chart_context->m_pve_hash = ppctx->m_pve_hash;
            cobj->m_chart_context->ref_lat = ppctx->ref_lat;
            cobj->m_chart_context->ref_lon = ppctx->ref_lon;
            cobj->m_chart_context->pFloatingATONArray = ppctx->pFloatingATONArray;
            cobj->m_chart_context->pRigidATONArray = ppctx->pRigidATONArray;
            cobj->m_chart_context->safety_contour = ppctx->safety_contour;
            cobj->m_chart_context->vertex_buffer = ppctx->vertex_buffer;
        }
        cobj->m_chart_context->chart = 0;           // note bene, this is always NULL for a PlugIn chart
    }
}


bool PI_PLIBSetContext( PI_S57Obj *pObj )
{
    S52PLIB_Context *ctx;
    if( !pObj->S52_Context ){
        ctx = new S52PLIB_Context;
        pObj->S52_Context = ctx;
    }
    
    ctx = (S52PLIB_Context *)pObj->S52_Context;
        
    S57Obj cobj;
    CreateCompatibleS57Object( pObj, &cobj, NULL );
 
    LUPname LUP_Name = PAPER_CHART;

    //      Force a re-evaluation of CS rules
    ctx->CSrules = NULL;
    ctx->bCS_Added = false;

    //      Clear the rendered text cache
    if( ctx->bFText_Added ) {
        ctx->bFText_Added = false;
        delete ctx->FText;
        ctx->FText = NULL;
    }
    
    //  Reset object selection box
    ctx->bBBObj_valid = true;
    ctx->BBObj.SetMin( pObj->lon_min, pObj->lat_min );
    ctx->BBObj.SetMax( pObj->lon_max, pObj->lat_max );
    
    
        //      This is where Simplified or Paper-Type point features are selected
    switch( cobj.Primitive_type ){
            case GEO_POINT:
            case GEO_META:
            case GEO_PRIM:
                
                if( PAPER_CHART == ps52plib->m_nSymbolStyle )
                    LUP_Name = PAPER_CHART;
                else
                    LUP_Name = SIMPLIFIED;
                
                break;
                
            case GEO_LINE:
                LUP_Name = LINES;
                break;
                
            case GEO_AREA:
                if( PLAIN_BOUNDARIES == ps52plib->m_nBoundaryStyle )
                    LUP_Name = PLAIN_BOUNDARIES;
                else
                    LUP_Name = SYMBOLIZED_BOUNDARIES;
                
                break;
    }
        
    LUPrec *lup = ps52plib->S52_LUPLookup( LUP_Name, cobj.FeatureName, &cobj );
    ctx->LUP = lup;
        
        //              Convert LUP to rules set
    ps52plib->_LUP2rules( lup, &cobj );
    
    ctx->MPSRulesList = NULL;
    
    return true;
}
    
void PI_UpdateContext(PI_S57Obj *pObj)
{
    S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;
    if(pContext){
        pContext->bBBObj_valid = true;
        pContext->BBObj.SetMin( pObj->lon_min, pObj->lat_min );
        pContext->BBObj.SetMax( pObj->lon_max, pObj->lat_max );
    }
}

    
void UpdatePIObjectPlibContext( PI_S57Obj *pObj, S57Obj *cobj, ObjRazRules *rzRules )
{
    //  Update the PLIB context after the render operation
    S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;
    
    pContext->CSrules = cobj->CSrules;
    pContext->bCS_Added = cobj->bCS_Added;
    
    pContext->FText = cobj->FText;
    pContext->bFText_Added = cobj->bFText_Added;
    pContext->rText = cobj->rText;
    
    if(cobj->bBBObj_valid)
        pContext->BBObj = cobj->BBObj;
    pContext->bBBObj_valid = cobj->bBBObj_valid;

    //  Render operation may have promoted the object's display category (e.g.WRECKS)
    pObj->m_DisplayCat = (PI_DisCat)cobj->m_DisplayCat;
    
    if(gs_plib_flags & PLIB_CAPS_OBJCATMUTATE)
        pObj->m_DPRI = cobj->m_DPRI;
    
    pContext->ChildRazRules = rzRules->child;
    pContext->MPSRulesList = rzRules->mps;
    
}

bool PI_GetObjectRenderBox( PI_S57Obj *pObj, double *lat_min, double *lat_max, double *lon_min, double *lon_max)
{
    S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;
    if(pContext){
        if(lat_min) *lat_min = pContext->BBObj.GetMinY();
        if(lat_max) *lat_max = pContext->BBObj.GetMaxY();
        if(lon_min) *lon_min = pContext->BBObj.GetMinX();
        if(lon_max) *lon_max = pContext->BBObj.GetMaxX();
        return pContext->bBBObj_valid;
    }
    else
        return false;
}
    
PI_LUPname PI_GetObjectLUPName( PI_S57Obj *pObj )
{
    S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;
    if( pContext ) {
        LUPrec *lup = pContext->LUP;
        if( lup )
            return (PI_LUPname)(lup->TNAM);
    }
    return (PI_LUPname)(-1);
    
}

PI_DisPrio PI_GetObjectDisplayPriority( PI_S57Obj *pObj )
{
    S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;
    if( pContext ) {
        LUPrec *lup = pContext->LUP;
        if( lup )
            return (PI_DisPrio)(lup->DPRI);
    }
    
    return (PI_DisPrio)(-1);
        
}

PI_DisCat PI_GetObjectDisplayCategory( PI_S57Obj *pObj )
{
    S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;
    if( pContext ) {
        LUPrec *lup = pContext->LUP;
        if( lup )
            return (PI_DisCat)(lup->DISC);
    }
    return (PI_DisCat)(-1);
    
}
double PI_GetPLIBMarinerSafetyContour()
{
    return S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR);
}


void PI_PLIBSetLineFeaturePriority( PI_S57Obj *pObj, int prio )
{
    //  Create and populate a compatible s57 Object
    S57Obj cobj;
    chart_context ctx;
    CreateCompatibleS57Object( pObj, &cobj, &ctx );
        
    S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;
    
    //  Create and populate a minimally compatible object container
    ObjRazRules rzRules;
    rzRules.obj = &cobj;
    rzRules.LUP = pContext->LUP;
    rzRules.sm_transform_parms = 0;
    rzRules.child = NULL;
    rzRules.next = NULL;
    rzRules.mps = pContext->MPSRulesList;
    
    if(pContext->LUP){
        ps52plib->SetLineFeaturePriority( &rzRules, prio );

    //  Update the PLIB context after the render operation
        UpdatePIObjectPlibContext( pObj, &cobj, &rzRules );
    }

}

void PI_PLIBPrepareForNewRender( void )
{
    if(ps52plib){
        ps52plib->PrepareForRender();
        ps52plib->ClearTextList();
        
        if(gs_plib_flags & PLIB_CAPS_LINE_BUFFER)
            ps52plib->EnableGLLS(true);    // Newer PlugIns can use GLLS
        else
            ps52plib->EnableGLLS(false);   // Older cannot
    }
}

void PI_PLIBSetRenderCaps( unsigned int flags )
{
    gs_plib_flags = flags;
}

void PI_PLIBFreeContext( void *pContext )
{

    S52PLIB_Context *pctx = (S52PLIB_Context *)pContext;
    
    if( pctx->ChildRazRules ){
        ObjRazRules *ctop = pctx->ChildRazRules;
        while( ctop ) {
            delete ctop->obj;
            
            if( ps52plib )
                ps52plib->DestroyLUP( ctop->LUP );
            delete ctop->LUP;
            
            ObjRazRules *cnxx = ctop->next;
            delete ctop;
            ctop = cnxx;
        }
    }

    if(pctx->MPSRulesList){
        
        if( ps52plib && pctx->MPSRulesList->cs_rules ){
            for(unsigned int i=0 ; i < pctx->MPSRulesList->cs_rules->GetCount() ; i++){
                Rules *top = pctx->MPSRulesList->cs_rules->Item(i);
                ps52plib->DestroyRulesChain( top );
            }
            delete pctx->MPSRulesList->cs_rules; 
        }
        free( pctx->MPSRulesList );
        
    }
    
    delete pctx->FText;
    
    delete pctx;
}

int PI_PLIBRenderObjectToDC( wxDC *pdc, PI_S57Obj *pObj, PlugIn_ViewPort *vp )
{
    //  Create and populate a compatible s57 Object
    S57Obj cobj;
    chart_context ctx;
    CreateCompatibleS57Object( pObj, &cobj, &ctx );
    
    S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;
    
    //  Set up object SM rendering constants
    sm_parms transform;
    toSM( vp->clat, vp->clon, pObj->chart_ref_lat, pObj->chart_ref_lon, &transform.easting_vp_center, &transform.northing_vp_center );
    
    //  Create and populate a minimally compatible object container
    ObjRazRules rzRules;
    rzRules.obj = &cobj;
    rzRules.LUP = pContext->LUP;
    rzRules.sm_transform_parms = &transform;
    rzRules.child = pContext->ChildRazRules;
    rzRules.next = NULL;
    rzRules.mps = pContext->MPSRulesList;
    
    if(pContext->LUP){
        ViewPort cvp = CreateCompatibleViewport( *vp );
    
    //  Do the render
        ps52plib->RenderObjectToDC( pdc, &rzRules, &cvp );
    
    //  Update the PLIB context after the render operation
        UpdatePIObjectPlibContext( pObj, &cobj, &rzRules );
    }

    return 1;
}

int PI_PLIBRenderAreaToDC( wxDC *pdc, PI_S57Obj *pObj, PlugIn_ViewPort *vp, wxRect rect, unsigned char *pixbuf )
{
    //  Create a compatible render canvas
    render_canvas_parms pb_spec;
    
    pb_spec.depth = BPP;
    pb_spec.pb_pitch = ( ( rect.width * pb_spec.depth / 8 ) );
    pb_spec.lclip = rect.x;
    pb_spec.rclip = rect.x + rect.width - 1;
    pb_spec.pix_buff = pixbuf;          // the passed buffer
    pb_spec.width = rect.width;
    pb_spec.height = rect.height;
    pb_spec.x = rect.x;
    pb_spec.y = rect.y;
#ifdef ocpnUSE_ocpnBitmap
    pb_spec.b_revrgb = true;
#else
    pb_spec.b_revrgb = false;
#endif
    
    pb_spec.b_revrgb = false;
 
    //  Create and populate a compatible s57 Object
    S57Obj cobj;
    chart_context ctx;
    CreateCompatibleS57Object( pObj, &cobj, &ctx );

    S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;

    //  Set up object SM rendering constants
    sm_parms transform;
    toSM( vp->clat, vp->clon, pObj->chart_ref_lat, pObj->chart_ref_lon, &transform.easting_vp_center, &transform.northing_vp_center );
    
    //  Create and populate a minimally compatible object container
    ObjRazRules rzRules;
    rzRules.obj = &cobj;
    rzRules.LUP = pContext->LUP;
    rzRules.sm_transform_parms = &transform;
    rzRules.child = pContext->ChildRazRules;
    rzRules.next = NULL;
    rzRules.mps = pContext->MPSRulesList;
    
    ViewPort cvp = CreateCompatibleViewport( *vp );

    //  If the PlugIn does not support it nativiely, build a fully described Geomoetry
    if( !(gs_plib_flags & PLIB_CAPS_SINGLEGEO_BUFFER) ){
        if(!pObj->geoPtMulti){          // do this only once
            PolyTessGeo *tess = (PolyTessGeo *)pObj->pPolyTessGeo;
        
            if(!tess)
                return 1;                       // bail on empty data
                
            PolyTriGroup *ptg = new PolyTriGroup;
            ptg->tri_prim_head = tess->Get_PolyTriGroup_head()->tri_prim_head; //tph;
            ptg->bsingle_alloc = false;
            ptg->data_type = DATA_TYPE_DOUBLE;
            tess->Set_PolyTriGroup_head(ptg);
        
            double *pd = (double *)malloc(sizeof(double));
            pObj->geoPtMulti = pd;  //Hack hack
        
        }
    }
    
    if(pContext->LUP){
    //  Do the render
        ps52plib->RenderAreaToDC( pdc, &rzRules, &cvp, &pb_spec );

    //  Update the PLIB context after the render operation
        UpdatePIObjectPlibContext( pObj, &cobj, &rzRules );
    }
    
    return 1;
}

int PI_PLIBRenderAreaToGL( const wxGLContext &glcc, PI_S57Obj *pObj, PlugIn_ViewPort *vp, wxRect &render_rect )
{
#ifdef ocpnUSE_GL
    //  Create and populate a compatible s57 Object
    S57Obj cobj;
    chart_context ctx;
    CreateCompatibleS57Object( pObj, &cobj, &ctx );

//    chart_context *pct = (chart_context *)pObj->m_chart_context;

    //  If the PlugIn does not support it nativiely, build a fully described Geomoetry
    
    if( !(gs_plib_flags & PLIB_CAPS_SINGLEGEO_BUFFER) ){
       if(!pObj->geoPtMulti ){                          // only do this once
            PolyTessGeo *tess = (PolyTessGeo *)pObj->pPolyTessGeo;
        
            if(!tess)
                return 1;                       // bail on empty data
                
            PolyTriGroup *ptg = new PolyTriGroup;       // this will leak a little, but is POD
            ptg->tri_prim_head = tess->Get_PolyTriGroup_head()->tri_prim_head; 
            ptg->bsingle_alloc = false;
            ptg->data_type = DATA_TYPE_DOUBLE;
            tess->Set_PolyTriGroup_head(ptg);

            //  Mark this object using geoPtMulti
            //  The malloc will get free'ed when the object is deleted.
            double *pd = (double *)malloc(sizeof(double));
            pObj->geoPtMulti = pd;  //Hack hack
        }            
        cobj.auxParm0 = -6;         // signal that this object render cannot use VBO
        cobj.auxParm1 = -1;         // signal that this object render cannot have single buffer conversion done
    }            
    else {              // it is a newer PLugIn, so can do single buffer conversion and VBOs
        cobj.auxParm0 = -5;         // signal that this object render must use a temporary VBO
    }
    

    S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;
    
    //  Set up object SM rendering constants
    sm_parms transform;
    toSM( vp->clat, vp->clon, pObj->chart_ref_lat, pObj->chart_ref_lon, &transform.easting_vp_center, &transform.northing_vp_center );
    
    //  Create and populate a minimally compatible object container
    ObjRazRules rzRules;
    rzRules.obj = &cobj;
    rzRules.LUP = pContext->LUP;
    rzRules.sm_transform_parms = &transform;
    rzRules.child = pContext->ChildRazRules;
    rzRules.next = NULL;
    rzRules.mps = pContext->MPSRulesList;
    
    if(pContext->LUP){
        ViewPort cvp = CreateCompatibleViewport( *vp );
    
    //  Do the render
        ps52plib->RenderAreaToGL( glcc, &rzRules, &cvp, render_rect );
    
    
    //  Update the PLIB context after the render operation
        UpdatePIObjectPlibContext( pObj, &cobj, &rzRules );
    }
    
#endif    
    return 1;
    
}

int PI_PLIBRenderObjectToGL( const wxGLContext &glcc, PI_S57Obj *pObj,
                                      PlugIn_ViewPort *vp, wxRect &render_rect )
{
    //  Create and populate a compatible s57 Object
    S57Obj cobj;
    chart_context ctx;
    CreateCompatibleS57Object( pObj, &cobj, &ctx );
    
    S52PLIB_Context *pContext = (S52PLIB_Context *)pObj->S52_Context;
    
    //  Set up object SM rendering constants
    sm_parms transform;
    toSM( vp->clat, vp->clon, pObj->chart_ref_lat, pObj->chart_ref_lon, &transform.easting_vp_center, &transform.northing_vp_center );
    
    //  Create and populate a minimally compatible object container
    ObjRazRules rzRules;
    rzRules.obj = &cobj;
    rzRules.LUP = pContext->LUP;
    rzRules.sm_transform_parms = &transform;
    rzRules.child = pContext->ChildRazRules;
    rzRules.next = NULL;
    rzRules.mps = pContext->MPSRulesList;
    
    if(pContext->LUP){
        ViewPort cvp = CreateCompatibleViewport( *vp );
    
    //  Do the render
        ps52plib->RenderObjectToGL( glcc, &rzRules, &cvp, render_rect );
    
    //  Update the PLIB context after the render operation
        UpdatePIObjectPlibContext( pObj, &cobj, &rzRules );
    }
    
    return 1;
    
}

/* API 1.13  */

/* API 1.13  adds some more common functions to avoid unnecessary code duplication */

double fromDMM_Plugin( wxString sdms )
{
    return fromDMM( sdms );
}

void SetCanvasRotation(double rotation)
{
    cc1->DoRotateCanvas( rotation );
}

// Play a sound to a given device
bool PlugInPlaySoundEx( wxString &sound_file, int deviceIndex )
{
    if(g_pi_manager) {
        g_pi_manager->m_plugin_sound.Stop();
        g_pi_manager->m_plugin_sound.UnLoad();

        g_pi_manager->m_plugin_sound.Create( sound_file, deviceIndex );

        if( g_pi_manager->m_plugin_sound.IsOk() )
            return g_pi_manager->m_plugin_sound.Play();
    }

    return false;
}

bool CheckEdgePan_PlugIn( int x, int y, bool dragging, int margin, int delta )
{
    return cc1->CheckEdgePan( x, y, dragging, margin, delta );
}

wxBitmap GetIcon_PlugIn(const wxString & name)
{
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
    return style->GetIcon( name );
}

void SetCursor_PlugIn( wxCursor *pCursor )
{
    cc1->pPlugIn_Cursor = pCursor;
}

void AddChartDirectory( wxString &path )
{
    if( g_options )
    {
        g_options->AddChartDir( path );
    }
}

void ForceChartDBUpdate()
{
    if( g_options )
    {
        g_options->pScanCheckBox->SetValue(true);
    }
}


int PlatformDirSelectorDialog( wxWindow *parent, wxString *file_spec, wxString Title, wxString initDir)
{
    return g_Platform->DoDirSelectorDialog( parent, file_spec, Title, initDir);
}

int PlatformFileSelectorDialog( wxWindow *parent, wxString *file_spec, wxString Title, wxString initDir,
                                                wxString suggestedName, wxString wildcard)
{
    return g_Platform->DoFileSelectorDialog( parent, file_spec, Title, initDir,
                                            suggestedName, wildcard);
}




//      http File Download Support

//      OCPN_downloadEvent Implementation

//DEFINE_EVENT_TYPE(wxEVT_DOWNLOAD_EVENT)

OCPN_downloadEvent::OCPN_downloadEvent(wxEventType commandType, int id)
:wxEvent(id, commandType)
{
    m_stat = OCPN_DL_UNKNOWN;
    m_condition = OCPN_DL_EVENT_TYPE_UNKNOWN;
    m_b_complete = false;
}

OCPN_downloadEvent::~OCPN_downloadEvent()
{
}

wxEvent* OCPN_downloadEvent::Clone() const
{
    OCPN_downloadEvent *newevent=new OCPN_downloadEvent(*this);
    newevent->m_stat=this->m_stat;
    newevent->m_condition=this->m_condition;

    newevent->m_totalBytes=this->m_totalBytes;
    newevent->m_sofarBytes=this->m_sofarBytes;
    newevent->m_b_complete=this->m_b_complete;
    
    return newevent;
}

const wxEventType wxEVT_DOWNLOAD_EVENT = wxNewEventType();





_OCPN_DLStatus g_download_status;
_OCPN_DLCondition g_download_condition;

#define DL_EVENT_TIMER 4388

class PI_DLEvtHandler : public wxEvtHandler
{
public:
    PI_DLEvtHandler();
    ~PI_DLEvtHandler();
    
    void onDLEvent( OCPN_downloadEvent &event);
    void setBackgroundMode( long ID, wxEvtHandler *handler );
    void clearBackgroundMode();
    void onTimerEvent(wxTimerEvent &event);
    
    long m_id;
    wxTimer m_eventTimer;
    wxEvtHandler *m_download_evHandler;
    
    long m_sofarBytes;
    long m_totalBytes;
    
    
};




PI_DLEvtHandler::PI_DLEvtHandler()
{
    g_download_status = OCPN_DL_UNKNOWN;
    g_download_condition = OCPN_DL_EVENT_TYPE_UNKNOWN;
    
    m_download_evHandler = NULL;
    m_id = -1;
    m_sofarBytes = 0;
    m_totalBytes = 0;
    
    
}

PI_DLEvtHandler::~PI_DLEvtHandler()
{
    m_eventTimer.Stop();
    Disconnect(wxEVT_TIMER, (wxObjectEventFunction)(wxEventFunction)&PI_DLEvtHandler::onTimerEvent);
    
}


void PI_DLEvtHandler::onDLEvent( OCPN_downloadEvent &event)
{
//    qDebug() << "Got Event " << (int)event.getDLEventStatus() << (int)event.getDLEventCondition();
    g_download_status = event.getDLEventStatus();
    g_download_condition = event.getDLEventCondition();

    // This is an END event, happening at the end of BACKGROUND file download
    if(m_download_evHandler && ( OCPN_DL_EVENT_TYPE_END == event.getDLEventCondition()) ){
        OCPN_downloadEvent ev(wxEVT_DOWNLOAD_EVENT, 0);
        ev.setComplete(true);
        ev.setTransferred(m_sofarBytes);
        ev.setTotal(m_totalBytes);
    
        ev.setDLEventStatus( event.getDLEventStatus());
        ev.setDLEventCondition( OCPN_DL_EVENT_TYPE_END );
    
        m_download_evHandler->AddPendingEvent(ev);
        m_eventTimer.Stop();
#ifdef __OCPN__ANDROID__        
        finishAndroidFileDownload();
#endif        
    }
    
    event.Skip();
}

void PI_DLEvtHandler::setBackgroundMode( long ID, wxEvtHandler *handler)
{
    m_id = ID;
    m_download_evHandler = handler;
    
    m_eventTimer.SetOwner( this, DL_EVENT_TIMER );
    
    Connect(wxEVT_TIMER, (wxObjectEventFunction)(wxEventFunction)&PI_DLEvtHandler::onTimerEvent);
    m_eventTimer.Start(1000, wxTIMER_CONTINUOUS);
    
}

void PI_DLEvtHandler::clearBackgroundMode()
{
    m_download_evHandler = NULL;
    m_eventTimer.Stop();
}
    
    
    
void PI_DLEvtHandler::onTimerEvent(wxTimerEvent &event)
{
#ifdef __OCPN__ANDROID__    
    //   Query the download status, and post to the original requestor

    wxString sstat;
    int stat = queryAndroidFileDownload( m_id, &sstat );
    
    long sofarBytes = 0;
    long totalBytes = -1;
    long state = -1;
    
    wxStringTokenizer tk(sstat, _T(";"));
    if( tk.HasMoreTokens() ){
        wxString token = tk.GetNextToken();
        token.ToLong(&state);
        token = tk.GetNextToken();
        token.ToLong(&sofarBytes);
        token = tk.GetNextToken();
        token.ToLong(&totalBytes);
    }

    qDebug() << state << sofarBytes << totalBytes;
    
    m_sofarBytes = sofarBytes;
    m_totalBytes = totalBytes;
    
    bool b_complete = false;
    if( (state == 16) || (state == 8) )
        b_complete = true;

    OCPN_downloadEvent ev(wxEVT_DOWNLOAD_EVENT, 0);
    ev.setComplete(b_complete);
    ev.setTransferred(sofarBytes);
    ev.setTotal(totalBytes);

    ev.setDLEventStatus( OCPN_DL_UNKNOWN);
    ev.setDLEventCondition( OCPN_DL_EVENT_TYPE_PROGRESS );
    
    //2;0;148686
    
    if(m_download_evHandler){
        qDebug() << "Sending event on timer...";
        m_download_evHandler->AddPendingEvent(ev);
    }
    
    
#endif    
}



PI_DLEvtHandler *g_piEventHandler;




//  Blocking download of single file
_OCPN_DLStatus OCPN_downloadFile( const wxString& url, const wxString &outputFile, 
                       const wxString &title, const wxString &message, 
                       const wxBitmap& bitmap,
                       wxWindow *parent, long style, int timeout_secs)
{
    
#ifdef __OCPN__ANDROID__

    wxString msg = _T("Downloading file synchronously: ");
    msg += url;  msg += _T(" to: ");  msg += outputFile;
    wxLogMessage(msg);
    
    //  Create a temporary event handler to receive status events
    g_piEventHandler = new PI_DLEvtHandler;

    //  Create a connection for the expected events
    g_piEventHandler->Connect(wxEVT_DOWNLOAD_EVENT, (wxObjectEventFunction)(wxEventFunction)&PI_DLEvtHandler::onDLEvent);
     
    long dl_ID = -1;
    
    int res = startAndroidFileDownload( url, outputFile, g_piEventHandler, &dl_ID );
    //  Started OK?
    if(res){
        finishAndroidFileDownload();
        g_piEventHandler->Disconnect(wxEVT_DOWNLOAD_EVENT, (wxObjectEventFunction)(wxEventFunction)&PI_DLEvtHandler::onDLEvent);
        delete g_piEventHandler;
        return OCPN_DL_FAILED;
    }
        
    
    wxDateTime dl_start_time = wxDateTime::Now();
    
    //  Spin, waiting for timeout or event from downstream, and checking status
    while(1){
        wxTimeSpan dt = wxDateTime::Now() - dl_start_time;
        qDebug() << "Spin.." << dt.GetSeconds().GetLo();
        
        if(dt.GetSeconds() > timeout_secs){
            qDebug() << "USER_TIMOUT";
            finishAndroidFileDownload();
            g_piEventHandler->Disconnect(wxEVT_DOWNLOAD_EVENT, (wxObjectEventFunction)(wxEventFunction)&PI_DLEvtHandler::onDLEvent);
            delete g_piEventHandler;
            return (OCPN_DL_USER_TIMEOUT);
        }
        
        if(g_download_condition != OCPN_DL_EVENT_TYPE_UNKNOWN){
            if(OCPN_DL_EVENT_TYPE_END == g_download_condition){
                _OCPN_DLStatus ss = g_download_status;
                qDebug() << "DL_END" << (int)ss;
                finishAndroidFileDownload();
                g_piEventHandler->Disconnect(wxEVT_DOWNLOAD_EVENT, (wxObjectEventFunction)(wxEventFunction)&PI_DLEvtHandler::onDLEvent);
                delete g_piEventHandler;
                return ss;              // The actual return code
            }
        }
        
        wxString sstat;
        int stat = queryAndroidFileDownload( dl_ID, &sstat );
        
        wxSleep(1);
        wxSafeYield();
    }
#endif
    
    return OCPN_DL_FAILED;
}            


//  Non-Blocking download of single file
_OCPN_DLStatus OCPN_downloadFileBackground( const wxString& url, const wxString &outputFile,
                                                            wxEvtHandler *handler, long *handle)
{
#ifdef __OCPN__ANDROID__
    wxString msg = _T("Downloading file asynchronously: ");
    msg += url;  msg += _T(" to: ");  msg += outputFile;
    wxLogMessage(msg);
    
    //  Create a temporary event handler to receive status events
    g_piEventHandler = new PI_DLEvtHandler;
    
    //  Create a connection for the expected events
    g_piEventHandler->Connect(wxEVT_DOWNLOAD_EVENT, (wxObjectEventFunction)(wxEventFunction)&PI_DLEvtHandler::onDLEvent);
    
    
    long dl_ID = -1;
    
    int res = startAndroidFileDownload( url, outputFile, g_piEventHandler, &dl_ID );
    //  Started OK?
    if(res){
        finishAndroidFileDownload();
        g_piEventHandler->Disconnect(wxEVT_DOWNLOAD_EVENT, (wxObjectEventFunction)(wxEventFunction)&PI_DLEvtHandler::onDLEvent);
        delete g_piEventHandler;
        return OCPN_DL_FAILED;
    }
    
    //  configure the local event handler for background transfer
    g_piEventHandler->setBackgroundMode(dl_ID, handler);
    
    if(handle)
        *handle = dl_ID;
    
    return OCPN_DL_STARTED;
#endif
    
    return OCPN_DL_FAILED;
    
}

void OCPN_cancelDownloadFileBackground( long handle )
{
#ifdef __OCPN__ANDROID__
    cancelAndroidFileDownload( handle );
    finishAndroidFileDownload();
    if(g_piEventHandler)
        g_piEventHandler->clearBackgroundMode();
    
#endif
}

