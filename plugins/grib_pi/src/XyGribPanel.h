///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.1.0-0-g733bf3d)
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
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/notebook.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <wx/gauge.h>
#include <wx/gbsizer.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class XyGribPanel
///////////////////////////////////////////////////////////////////////////////
class XyGribPanel : public wxPanel
{
	private:

	protected:
		wxStaticText* m_wavemodel_label;
		wxStaticText* m_atmmodel_label;
		wxStaticText* m_weathermodel;
		wxStaticText* m_duration_label;
		wxStaticText* m_run_label;
		wxStaticText* m_interval_label;
		wxNotebook* m_dataconfig_nbook;
		wxPanel* m_surfacetab_panel;
		wxStaticLine* m_separator_1;
		wxStaticText* m_status_label;
		wxStaticText* m_sizeestimate_label;

	public:
		wxChoice* m_atmmodel_choice;
		wxChoice* m_wavemodel_choice;
		wxChoice* m_interval_choice;
		wxChoice* m_resolution_choice;
		wxChoice* m_duration_choice;
		wxChoice* m_run_choice;
		wxCheckBox* m_wind_cbox;
		wxCheckBox* m_gust_cbox;
		wxCheckBox* m_pressure_cbox;
		wxCheckBox* m_temperature_cbox;
		wxCheckBox* m_cape_cbox;
		wxCheckBox* m_reflectivity_cbox;
		wxCheckBox* m_cloudcover_cbox;
		wxCheckBox* m_precipitation_cbox;
		wxCheckBox* m_waveheight_cbox;
		wxCheckBox* m_windwave_cbox;
		wxButton* m_download_button;
		wxGauge* m_progress_gauge;
		wxStaticText* m_status_text;
		wxStaticText* m_sizeestimate_text;

		XyGribPanel( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1,-1 ), long style = wxTAB_TRAVERSAL, const wxString& name = wxEmptyString );

		~XyGribPanel();

};

