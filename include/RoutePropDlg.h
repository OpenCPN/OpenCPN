///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/timectrl.h>
#include <wx/choice.h>
#include <wx/wrapsizer.h>
#include <wx/dataview.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/statbox.h>
#include <wx/notebook.h>
#include <wx/button.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class RoutePropDlg
///////////////////////////////////////////////////////////////////////////////
class RoutePropDlg : public wxFrame
{
	private:

	protected:
		wxNotebook* m_ntbRteProp;
		wxPanel* m_pnlBasic;
		wxStaticText* m_stName;
		wxTextCtrl* m_tcName;
		wxStaticText* m_stFrom;
		wxTextCtrl* m_tcFrom;
		wxStaticText* m_stTo;
		wxTextCtrl* m_tcTo;
		wxStaticText* m_stDistTotal;
		wxTextCtrl* m_tcDistance;
		wxStaticText* m_stPlanSpeed;
		wxTextCtrl* m_tcPlanSpeed;
		wxStaticText* m_stEnroute;
		wxTextCtrl* m_tcEnroute;
		wxStaticText* m_stDeparture;
		wxDatePickerCtrl* m_dpDepartureDate;
		wxTimePickerCtrl* m_tpDepartureTime;
		wxStaticText* m_stTimeZone;
		wxChoice* m_choiceTimezone;
		wxDataViewListCtrl* m_dvlcWaypoints;
		wxDataViewColumn* m_dataViewListColumnLeg;
		wxDataViewColumn* m_dataViewListColumnToWpt;
		wxDataViewColumn* m_dataViewListColumnDistance;
		wxDataViewColumn* m_dataViewListColumnBearing;
		wxDataViewColumn* m_dataViewListColumnLat;
		wxDataViewColumn* m_dataViewListColumnLon;
		wxDataViewColumn* m_dataViewListColumnETE;
		wxDataViewColumn* m_dataViewListColumnETA;
		wxDataViewColumn* m_dataViewListColumnSpeed;
		wxDataViewColumn* m_dataViewListColumnNTE;
		wxDataViewColumn* m_dataViewListColumnDesc;
		wxDataViewColumn* m_dataViewListColumnCourse;
		wxDataViewColumn* m_dataViewListColumnEmpty;
		wxPanel* m_pnlAdvanced;
		wxStaticText* m_stDescription;
		wxTextCtrl* m_textCtrl9;
		wxButton* m_btnPrint;
		wxButton* m_btnExtend;
		wxButton* m_btnSplit;
		wxStdDialogButtonSizer* m_sdbSizerBtns;
		wxButton* m_sdbSizerBtnsOK;
		wxButton* m_sdbSizerBtnsCancel;

		// Virtual event handlers, overide them in your derived class
		virtual void RoutePropDlgOnClose( wxCloseEvent& event ) { event.Skip(); }
		virtual void RoutePropDlgOnSize( wxSizeEvent& event ) { event.Skip(); }
		virtual void RoutePropDlgOnNotebookPageChanged( wxNotebookEvent& event ) { event.Skip(); }
		virtual void PlanSpeedOnText( wxCommandEvent& event ) { event.Skip(); }
		virtual void PlanSpeedOnTextEnter( wxCommandEvent& event ) { event.Skip(); }
		virtual void DepartureDateOnDateChanged( wxDateEvent& event ) { event.Skip(); }
		virtual void DepartureTimeOnTimeChanged( wxDateEvent& event ) { event.Skip(); }
		virtual void TimezoneOnChoice( wxCommandEvent& event ) { event.Skip(); }
		virtual void WaypointsOnDataViewListCtrlItemEditingDone( wxDataViewEvent& event ) { event.Skip(); }
		virtual void WaypointsOnDataViewListCtrlItemEditingStarted( wxDataViewEvent& event ) { event.Skip(); }
		virtual void WaypointsOnDataViewListCtrlItemStartEditing( wxDataViewEvent& event ) { event.Skip(); }
		virtual void WaypointsOnDataViewListCtrlItemValueChanged( wxDataViewEvent& event ) { event.Skip(); }
		virtual void PrintOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void ExtendOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void SplitOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void BtnsOnCancelButtonClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void BtnsOnOKButtonClick( wxCommandEvent& event ) { event.Skip(); }


	public:

		RoutePropDlg( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Route Properties"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 710,414 ), long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL );

		~RoutePropDlg();

};

