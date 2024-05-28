///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.0.0-0-g0efcecf0)
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
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/checklst.h>
#include <wx/button.h>
#include <wx/listbox.h>
#include <wx/html/htmlwin.h>
#include <wx/wizard.h>
#include <wx/dynarray.h>
WX_DEFINE_ARRAY_PTR( wxWizardPageSimple*, WizardPages );

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class FirstUseWiz
///////////////////////////////////////////////////////////////////////////////
class FirstUseWiz : public wxWizard
{
	private:

	protected:
		wxStaticText* m_stDepth;
		wxChoice* m_cDepth;
		wxStaticText* m_stDistance;
		wxChoice* m_cDistance;
    wxStaticText* m_stSpeed;
    wxChoice* m_cSpeed;
    wxStaticText* m_stWind;
    wxChoice* m_cWind;
		wxStaticText* m_stPosition;
		wxChoice* m_cPosition;
		wxCheckBox* m_cbShowTrue;
		wxCheckBox* m_cbShowMagnetic;
		wxRichTextCtrl* m_rtLangUnitInfo;
		wxStaticText* m_stSources;
		wxCheckListBox* m_clSources;
		wxButton* m_btnRescanSources;
		wxRichTextCtrl* m_rtConnectionInfo;
		wxStaticText* m_stAddCharts;
		wxListBox* m_lbChartsDirs;
		wxButton* m_btnAddChartDir;
		wxRichTextCtrl* m_rtChartDirInfo;
		wxHtmlWindow* m_htmlWinFinish;

		// Virtual event handlers, override them in your derived class
		virtual void OnInitDialog( wxInitDialogEvent& event ) { event.Skip(); }
		virtual void OnWizardCancel( wxWizardEvent& event ) { event.Skip(); }
		virtual void OnWizardFinished( wxWizardEvent& event ) { event.Skip(); }
		virtual void OnWizardHelp( wxWizardEvent& event ) { event.Skip(); }
		virtual void OnWizardPageChanged( wxWizardEvent& event ) { event.Skip(); }
		virtual void OnWizardPageChanging( wxWizardEvent& event ) { event.Skip(); }
		virtual void OnWizardPageShown( wxWizardEvent& event ) { event.Skip(); }
		virtual void m_btnRescanSourcesOnButtonClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void m_btnAddChartDirOnButtonClick( wxCommandEvent& event ) { event.Skip(); }


	public:

		FirstUseWiz( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("OpenCPN Initial Configuration"), const wxBitmap& bitmap = wxNullBitmap, const wxPoint& pos = wxDefaultPosition, long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxSTAY_ON_TOP );
		WizardPages m_pages;

		~FirstUseWiz();

};

