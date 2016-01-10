///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun  6 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __CHARTDLDRGUI_H__
#define __CHARTDLDRGUI_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/treectrl.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/notebook.h>
#include <wx/statbox.h>
#include <wx/filepicker.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/combobox.h>
#include "checkedlistctrl.h"
#include <wx/checkbox.h>
#include <wx/statline.h>
#include <wx/dcmemory.h>

#include "ocpn_plugin.h"

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class AddSourceDlg
///////////////////////////////////////////////////////////////////////////////
class AddSourceDlg : public wxDialog
{
	private:

	protected:
		wxPanel* m_panelPredefined;
		wxPanel* m_panelCustom;
		wxStaticText* m_stName;
		wxStaticText* m_stUrl;
		wxStdDialogButtonSizer* m_sdbSizerBtns;
		wxButton* m_sdbSizerBtnsOK;
		wxButton* m_sdbSizerBtnsCancel;

		// Virtual event handlers, overide them in your derived class
		virtual void OnSourceSelected( wxTreeEvent& event ) { event.Skip(); }
		virtual void OnOkClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancelClick( wxCommandEvent& event ) { event.Skip(); }
                void OnDirSelClick( wxCommandEvent& event );
                void applyStyle();
                
	public:
		wxNotebook* m_nbChoice;
		wxTreeCtrl* m_treeCtrlPredefSrcs;
		wxTextCtrl* m_tSourceName;
		wxTextCtrl* m_tChartSourceUrl;
		//wxDirPickerCtrl* m_dpChartDirectory;
                wxTextCtrl *m_tcChartDirectory;
                wxButton *m_buttonChartDirectory;
                wxScrolledWindow *m_sourceswin;
                
		AddSourceDlg( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("New chart source"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxDEFAULT_DIALOG_STYLE );
		~AddSourceDlg();

};

///////////////////////////////////////////////////////////////////////////////
/// Class ChartDldrPanel
///////////////////////////////////////////////////////////////////////////////
class ChartDldrPanel : public wxPanel
{
	private:

	protected:
		wxListCtrl* m_lbChartSources;
		wxButton* m_bAddSource;
		wxButton* m_bDeleteSource;
		wxButton* m_bEditSource;
		wxButton* m_bUpdateChartList;
		wxButton* m_bUpdateAllCharts;
		wxStaticText* m_stCatalogInfo;
		wxButton* m_bHelp;
		wxButton* m_bDnldCharts;
		wxButton* m_bShowLocal;

		// Virtual event handlers, overide them in your derived class
		virtual void OnPaint( wxPaintEvent& event ) { event.Skip(); }
		virtual void OnLeftDClick( wxMouseEvent& event ) { event.Skip(); }
		virtual void SelectSource( wxListEvent& event ) { event.Skip(); }
		virtual void AddSource( wxCommandEvent& event ) { event.Skip(); }
		virtual void DeleteSource( wxCommandEvent& event ) { event.Skip(); }
		virtual void EditSource( wxCommandEvent& event ) { event.Skip(); }
		virtual void UpdateChartList( wxCommandEvent& event ) { event.Skip(); }
		virtual void UpdateAllCharts( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnContextMenu( wxMouseEvent& event ) { event.Skip(); }
		virtual void DoHelp( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnDownloadCharts( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnShowLocalDir( wxCommandEvent& event ) { event.Skip(); }


	public:
		wxCheckedListCtrl *m_clCharts;

		ChartDldrPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL );
		~ChartDldrPanel();
        ChartDldrPanel() { }
};

///////////////////////////////////////////////////////////////////////////////
/// Class ChartDldrPrefsDlg
///////////////////////////////////////////////////////////////////////////////
class ChartDldrPrefsDlg : public wxDialog
{
	private:

	protected:
		//wxDirPickerCtrl* m_dpDefaultDir;
		wxStaticText* m_stPreselect;
		wxCheckBox* m_cbSelectUpdated;
		wxCheckBox* m_cbSelectNew;
		wxStaticLine* m_staticline1;
		wxCheckBox* m_cbBulkUpdate;
		wxStdDialogButtonSizer* m_sdbSizerBtns;
		wxButton* m_sdbSizerBtnsOK;
		wxButton* m_sdbSizerBtnsCancel;
                wxButton* m_buttonChartDirectory;
                wxTextCtrl* m_tcDefaultDir;
                
                void OnDirSelClick( wxCommandEvent& event );
                
                
		// Virtual event handlers, overide them in your derived class
		virtual void OnCancelClick( wxCommandEvent& event ) ;
		virtual void OnOkClick( wxCommandEvent& event );


	public:

		ChartDldrPrefsDlg( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Chart Downloader Preferences"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 462,331 ), long style = wxDEFAULT_DIALOG_STYLE );
		~ChartDldrPrefsDlg();

};

#endif //__CHARTDLDRGUI_H__
