/******************************************************************************
 * $Id: chartdldr_pi.h,v 1.0 2011/02/26 01:54:37 nohal Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Chart Downloader Plugin
 * Author:   Pavel Kalian
 *
 ***************************************************************************
 *   Copyright (C) 2011 by Pavel Kalian   *
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
 */

#ifndef _CHARTDLDRPI_H_
#define _CHARTDLDRPI_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <wx/fileconf.h>
#include <wx/tokenzr.h>
#include <wx/event.h>

#include <wx/imaglist.h>

#include <map>

#include "version.h"

#define     MY_API_VERSION_MAJOR    1
#define     MY_API_VERSION_MINOR    13

#define USERDATA "{USERDATA}"

#include "ocpn_plugin.h"

#include "chartdldrgui.h"
#include "chartcatalog.h"

#define UPDATE_DATA_FILENAME "chartdldr_pi.dat"

// forward declarations
class ChartSource;
class ChartDldrPanelImpl;
class ChartDldrGuiAddSourceDlg;
class ChartDldrPrefsDlgImpl;

WX_DEFINE_ARRAY_PTR(ChartSource *, wxArrayOfChartSources);
WX_DECLARE_OBJARRAY(wxDateTime, wxArrayOfDateTime);

#define NEW_LIST

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------

class chartdldr_pi : public opencpn_plugin_113
{
public:
    chartdldr_pi( void *ppimgr );

//    The required PlugIn Methods
    int             Init( void );
    bool            DeInit( void );

    int             GetAPIVersionMajor();
    int             GetAPIVersionMinor();
    int             GetPlugInVersionMajor();
    int             GetPlugInVersionMinor();
    wxBitmap       *GetPlugInBitmap();
    wxString        GetCommonName();
    wxString        GetShortDescription();
    wxString        GetLongDescription();

    void            OnSetupOptions( void );
    void            OnCloseToolboxPanel( int page_sel, int ok_apply_cancel );

//    The required override PlugIn Methods
    void            ShowPreferencesDialog( wxWindow* parent );

//    Other public methods
    bool            SaveConfig(void);
    bool            ProcessFile(const wxString& aFile, const wxString& aTargetDir, bool aStripPath = true, wxDateTime aMTime = wxDateTime::Now());
    bool            ExtractZipFiles(const wxString& aZipFile, const wxString& aTargetDir, bool aStripPath = true, wxDateTime aMTime = wxDateTime::Now(), bool aRemoveZip = false);
#ifdef DLDR_USE_LIBARCHIVE
    bool            ExtractLibArchiveFiles(const wxString& aArchiveFile, const wxString& aTargetDir, bool aStripPath = true, wxDateTime aMTime = wxDateTime::Now(), bool aRemoveArchive = false);
#endif
#if defined(CHARTDLDR_RAR_UNARR) || !defined(DLDR_USE_LIBARCHIVE)
    bool            ExtractUnarrFiles(const wxString& aRarFile, const wxString& aTargetDir, bool aStripPath = true, wxDateTime aMTime = wxDateTime::Now(), bool aRemoveRar = false);
#endif

    void            UpdatePrefs(ChartDldrPrefsDlgImpl *dialog);
    
//    Public properties
    wxArrayOfChartSources *m_pChartSources;
    wxWindow       *m_parent_window;
    ChartCatalog   *m_pChartCatalog;
    ChartSource    *m_pChartSource;
    void            SetSourceId(int id) { m_selected_source = id; }
    int             GetSourceId() { return m_selected_source; }
    wxString        GetBaseChartDir() { return m_base_chart_dir; }
    bool            m_preselect_new;
    bool            m_preselect_updated;
    bool            m_allow_bulk_update;

private:
    wxFileConfig   *m_pconfig;
    wxScrolledWindow *m_pOptionsPage;
    bool            LoadConfig(void);

    int             m_leftclick_tool_id;

    wxString        m_schartdldr_sources;
    int             m_selected_source;

    ChartDldrPanelImpl *m_dldrpanel;
    wxString        m_base_chart_dir;
};

class ChartSource : public wxTreeItemData
{
public:
    ChartSource( wxString name, wxString url, wxString localdir );
    ~ChartSource();
    
    wxString        GetName() { return m_name; }
    wxString        GetUrl() { return m_url; }
    wxString        GetDir() { return m_dir; }
    void            SetDir( wxString dir ) { m_dir = dir; }
    void            SetName(    wxString name ) { m_name = name; }
    void            SetUrl( wxString url ) { m_url = url; }
    bool            ExistsLocaly(   wxString chart_number, wxString filename );
    bool            IsNewerThanLocal( wxString chart_number, wxString filename, wxDateTime validDate );
    void            UpdateLocalFiles() { GetLocalFiles(); }

    bool            UpdateDataExists();
    void            LoadUpdateData();
    void            SaveUpdateData();
    void            ChartUpdated( wxString chart_number, time_t timestamp );
private:
    wxArrayString   m_localfiles;
    wxArrayOfDateTime m_localdt;
    void            GetLocalFiles();
    wxString        m_name;
    wxString        m_url;
    wxString        m_dir;
    std::map<std::string, time_t> m_update_data;
};

/** Implementing ChartDldrPanel */
class ChartDldrPanelImpl : public ChartDldrPanel
{
friend class chartdldr_pi;
private:
    bool            DownloadChart( wxString url, wxString file, wxString title );
    bool            downloadInProgress;
    int             to_download;

    int             updatingAll;
    bool            cancelled;
    bool            DownloadIsCancel;
    chartdldr_pi   *pPlugIn;
    bool            m_populated;

    void            OnPopupClick( wxCommandEvent &evt );
    int             GetSelectedCatalog();
    void            AppendCatalog(ChartSource *cs);
    void            DoEditSource();

    bool            m_bTransferComplete;
    bool            m_bTransferSuccess;
    wxString        m_totalsize;
    wxString        m_transferredsize;
    int		    m_failed_downloads;
    int             m_downloading;

    void            DisableForDownload( bool enabled );
    bool            m_bconnected;

protected:
    // Handlers for ChartDldrPanel events.
    void            SetSource( int id );
	void            SelectSource( wxListEvent& event );
	void            AddSource( wxCommandEvent& event );
	void            DeleteSource( wxCommandEvent& event );
	void            EditSource( wxCommandEvent& event );
	void            UpdateChartList( wxCommandEvent& event );
	void            OnDownloadCharts( wxCommandEvent& event );
	void            DownloadCharts( );
	void            DoHelp( wxCommandEvent& event )
      {
          #ifdef __WXMSW__
          wxLaunchDefaultBrowser( _T("file:///") + *GetpSharedDataLocation() + _T("plugins/chartdldr_pi/data/doc/index.html") );
          #else
          wxLaunchDefaultBrowser( _T("file://") + *GetpSharedDataLocation() + _T("plugins/chartdldr_pi/data/doc/index.html") );
          #endif
      }
    void            UpdateAllCharts( wxCommandEvent& event );
	void            OnShowLocalDir( wxCommandEvent& event );
    void            OnPaint( wxPaintEvent& event );
    void            OnLeftDClick( wxMouseEvent& event );

    void            CleanForm();
    void            FillFromFile( wxString url, wxString dir, bool selnew = false, bool selupd = false );

    void            OnContextMenu( wxMouseEvent& event );
    void            SetBulkUpdate( bool bulk_update );

    int             GetChartCount();
    int             GetCheckedChartCount();
    bool            isChartChecked( int i );
    void            CheckAllCharts( bool value );
    void            InvertCheckAllCharts( );
    
public:
    //ChartDldrPanelImpl() { m_bconnected = false; DownloadIsCancel = false; }
    ~ChartDldrPanelImpl();
    ChartDldrPanelImpl( chartdldr_pi* plugin = NULL, wxWindow* parent = NULL, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER );
    void            SelectCatalog( int item );
    void            onDLEvent(OCPN_downloadEvent &ev);
    void            CancelDownload() { Disconnect(wxEVT_DOWNLOAD_EVENT, (wxObjectEventFunction)(wxEventFunction)&ChartDldrPanelImpl::onDLEvent); cancelled = true; m_bconnected = false;}
    
private:
    DECLARE_DYNAMIC_CLASS( ChartDldrPanelImpl )
    DECLARE_EVENT_TABLE()
};

class ChartDldrGuiAddSourceDlg : public AddSourceDlg
{
protected:
    void            OnChangeType( wxCommandEvent& event );
	void            OnSourceSelected( wxTreeEvent& event );
	void            OnOkClick( wxCommandEvent& event );
    void            OnCancelClick( wxCommandEvent& event );
        
    bool            LoadSources();
    bool            LoadSections( const wxTreeItemId &root, pugi::xml_node &node );
    bool            LoadSection( const wxTreeItemId &root, pugi::xml_node &node );
    bool            LoadCatalogs( const wxTreeItemId &root, pugi::xml_node &node );
    bool            LoadCatalog( const wxTreeItemId &root, pugi::xml_node &node );

public:
    ChartDldrGuiAddSourceDlg( wxWindow* parent );
	~ChartDldrGuiAddSourceDlg();
	void            SetBasePath( const wxString path ) { m_base_path = path; }
    void            SetSourceEdit( ChartSource* cs );

private:
    bool            ValidateUrl(const wxString Url, bool catalog_xml = true);
    wxString        FixPath(wxString path);
    wxString        m_base_path;
    wxString        m_last_path;
    wxImageList    *p_iconList;
#ifdef __OCPN__ANDROID__    
    wxImageList    *p_buttonIconList;
#endif /* __OCPN__ANDROID__ */
};

class ChartDldrPrefsDlgImpl : public ChartDldrPrefsDlg
{
protected:
    void            OnOkClick( wxCommandEvent& event );

public:
    ChartDldrPrefsDlgImpl( wxWindow* parent );
	~ChartDldrPrefsDlgImpl();
	wxString        GetPath() { return m_tcDefaultDir->GetValue(); }
	void            SetPath( const wxString path );
	void            GetPreferences( bool &preselect_new, bool &preselect_updated, bool &bulk_update );
	void            SetPreferences( bool preselect_new, bool preselect_updated, bool bulk_update );
};

#endif
