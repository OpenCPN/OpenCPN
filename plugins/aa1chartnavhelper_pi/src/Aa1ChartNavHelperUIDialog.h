///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef AA1CHARTNAVHELPERUIDIALOG_H__
#define AA1CHARTNAVHELPERUIDIALOG_H__

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
#include <wx/statbox.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/radiobox.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>

///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class Aa1ChartNavHelperUIDialogBase
///////////////////////////////////////////////////////////////////////////////
class Aa1ChartNavHelperUIDialogBase : public wxDialog 
{
	private:
	
	protected:
		// Virtual event handlers, overide them in your derived class
		virtual void PlotSettings( wxCommandEvent& event ) { event.Skip(); }
		virtual void EnableCompassChanged( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		wxBoxSizer* bSframe;
		wxStaticBoxSizer* sbPlot;
		wxButton* m_bPlotSettings;
		wxCheckBox* m_cbEnableCompass;
		
		Aa1ChartNavHelperUIDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY,
                        const wxString& title = _("Chart Nav Helper"), const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxSize( 250,485 ), long style = wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxTAB_TRAVERSAL ); 
		~Aa1ChartNavHelperUIDialogBase();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class Aa1ChartNavHelperPrefsDialog
///////////////////////////////////////////////////////////////////////////////
class Aa1ChartNavHelperPrefsDialog : public wxDialog 
{
	private:
	
	protected:
		wxStdDialogButtonSizer* m_sdbSizer1;
		wxButton* m_sdbSizer1OK;
		wxButton* m_sdbSizer1Cancel;
	
	public:
        wxCheckBox* m_cbShowLiveToolbarIcon;
        wxCheckBox* m_cbShowToolbarIcon;
        wxSlider* m_sOpacity;
		
		Aa1ChartNavHelperPrefsDialog( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Chart Nav Helper Preferences"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxCAPTION|wxDEFAULT_DIALOG_STYLE );
		~Aa1ChartNavHelperPrefsDialog();
	
};

///////////////////////////////////////////////////////////////////////////////
/// Class Aa1ChartNavHelperPlotSettingsDialogBase
///////////////////////////////////////////////////////////////////////////////
class Aa1ChartNavHelperPlotSettingsDialogBase : public wxDialog 
{
	private:
	
	protected:
		wxButton* m_bAbout;
		wxButton* m_bSave;
		wxButton* m_bCancel;
		
		// Virtual event handlers, overide them in your derived class
		virtual void About( wxCommandEvent& event ) { event.Skip(); }
		virtual void Save( wxCommandEvent& event ) { event.Skip(); }
		virtual void Cancel( wxCommandEvent& event ) { event.Skip(); }
		
	
	public:
		wxStaticText* m_staticTextValYear;
		wxSpinCtrl*   m_scCtrlValYearSpacing;

		wxCheckBox* m_checkBoxUseDefaultVar;
		wxTextCtrl* m_textDefaultVar;
		wxTextCtrl* m_textDefaultVarYearDelta;
		wxStaticText* m_staticTextDefaultVar1;
		wxStaticText* m_staticTextDefaultVar2;
		wxStaticText* m_staticTextDefaultVar3;

		Aa1ChartNavHelperPlotSettingsDialogBase( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Chart Nav Helper Setting"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 375,270 ), long style = wxDEFAULT_DIALOG_STYLE );
		~Aa1ChartNavHelperPlotSettingsDialogBase();
	
};

#endif //AA1CHARTNAVHELPERUIDIALOG_H__
