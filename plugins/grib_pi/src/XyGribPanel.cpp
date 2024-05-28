///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version 4.1.0-0-g733bf3d)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "XyGribPanel.h"

///////////////////////////////////////////////////////////////////////////

XyGribPanel::XyGribPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name ) : wxPanel( parent, id, pos, size, style, name )
{
	wxGridBagSizer* m_separator_line;
	m_separator_line = new wxGridBagSizer( 0, 0 );
	m_separator_line->SetFlexibleDirection( wxBOTH );
	m_separator_line->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_wavemodel_label = new wxStaticText( this, wxID_ANY, _("Wave model"), wxDefaultPosition, wxDefaultSize, 0 );
	m_wavemodel_label->Wrap( -1 );
	m_separator_line->Add( m_wavemodel_label, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );

	m_atmmodel_label = new wxStaticText( this, wxID_ANY, _("Weather model"), wxDefaultPosition, wxDefaultSize, 0 );
	m_atmmodel_label->Wrap( -1 );
	m_separator_line->Add( m_atmmodel_label, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );

	m_weathermodel = new wxStaticText( this, wxID_ANY, _("Resolution"), wxDefaultPosition, wxDefaultSize, 0 );
	m_weathermodel->Wrap( -1 );
	m_separator_line->Add( m_weathermodel, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );

	m_duration_label = new wxStaticText( this, wxID_ANY, _("Days"), wxDefaultPosition, wxDefaultSize, 0 );
	m_duration_label->Wrap( -1 );
	m_separator_line->Add( m_duration_label, wxGBPosition( 3, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );

	m_run_label = new wxStaticText( this, wxID_ANY, _("Run"), wxDefaultPosition, wxDefaultSize, 0 );
	m_run_label->Wrap( -1 );
	m_separator_line->Add( m_run_label, wxGBPosition( 4, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );

	m_interval_label = new wxStaticText( this, wxID_ANY, _("Interval"), wxDefaultPosition, wxDefaultSize, 0 );
	m_interval_label->Wrap( -1 );
	m_separator_line->Add( m_interval_label, wxGBPosition( 5, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );

	wxArrayString m_atmmodel_choiceChoices;
	m_atmmodel_choice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_atmmodel_choiceChoices, 0 );
	m_atmmodel_choice->SetSelection( 0 );
	m_separator_line->Add( m_atmmodel_choice, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 5 );

	wxArrayString m_wavemodel_choiceChoices;
	m_wavemodel_choice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_wavemodel_choiceChoices, 0 );
	m_wavemodel_choice->SetSelection( 0 );
	m_separator_line->Add( m_wavemodel_choice, wxGBPosition( 1, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 5 );

	wxArrayString m_interval_choiceChoices;
	m_interval_choice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_interval_choiceChoices, 0 );
	m_interval_choice->SetSelection( 0 );
	m_separator_line->Add( m_interval_choice, wxGBPosition( 5, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 5 );

	wxArrayString m_resolution_choiceChoices;
	m_resolution_choice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_resolution_choiceChoices, 0 );
	m_resolution_choice->SetSelection( 0 );
	m_separator_line->Add( m_resolution_choice, wxGBPosition( 2, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 5 );

	wxArrayString m_duration_choiceChoices;
	m_duration_choice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_duration_choiceChoices, 0 );
	m_duration_choice->SetSelection( 0 );
	m_separator_line->Add( m_duration_choice, wxGBPosition( 3, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 5 );

	wxArrayString m_run_choiceChoices;
	m_run_choice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_run_choiceChoices, 0 );
	m_run_choice->SetSelection( 0 );
	m_separator_line->Add( m_run_choice, wxGBPosition( 4, 1 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 5 );

	m_dataconfig_nbook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_surfacetab_panel = new wxPanel( m_dataconfig_nbook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxGridSizer* m_surfacetab_sizer;
	m_surfacetab_sizer = new wxGridSizer( 0, 2, 0, 0 );

	m_wind_cbox = new wxCheckBox( m_surfacetab_panel, wxID_ANY, _("Wind (10m)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_wind_cbox->SetValue(true);
	m_surfacetab_sizer->Add( m_wind_cbox, 0, wxALL, 5 );

	m_gust_cbox = new wxCheckBox( m_surfacetab_panel, wxID_ANY, _("Wind gust (surface)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_gust_cbox->SetValue(true);
	m_surfacetab_sizer->Add( m_gust_cbox, 0, wxALL, 5 );

	m_pressure_cbox = new wxCheckBox( m_surfacetab_panel, wxID_ANY, _("Pressure (MSL)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_surfacetab_sizer->Add( m_pressure_cbox, 0, wxALL, 5 );

	m_temperature_cbox = new wxCheckBox( m_surfacetab_panel, wxID_ANY, _("Temperature (2m)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_surfacetab_sizer->Add( m_temperature_cbox, 0, wxALL, 5 );

	m_cape_cbox = new wxCheckBox( m_surfacetab_panel, wxID_ANY, _("CAPE (surface)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_surfacetab_sizer->Add( m_cape_cbox, 0, wxALL, 5 );

	m_reflectivity_cbox = new wxCheckBox( m_surfacetab_panel, wxID_ANY, _("Reflectivity (atmosphere)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_surfacetab_sizer->Add( m_reflectivity_cbox, 0, wxALL, 5 );

	m_cloudcover_cbox = new wxCheckBox( m_surfacetab_panel, wxID_ANY, _("Cloud cover (total)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_surfacetab_sizer->Add( m_cloudcover_cbox, 0, wxALL, 5 );

	m_precipitation_cbox = new wxCheckBox( m_surfacetab_panel, wxID_ANY, _("Total precipitation"), wxDefaultPosition, wxDefaultSize, 0 );
	m_surfacetab_sizer->Add( m_precipitation_cbox, 0, wxALL, 5 );

	m_waveheight_cbox = new wxCheckBox( m_surfacetab_panel, wxID_ANY, _("Wave Significant height"), wxDefaultPosition, wxDefaultSize, 0 );
	m_waveheight_cbox->SetValue(true);
	m_surfacetab_sizer->Add( m_waveheight_cbox, 0, wxALL, 5 );

	m_windwave_cbox = new wxCheckBox( m_surfacetab_panel, wxID_ANY, _("Wind waves"), wxDefaultPosition, wxDefaultSize, 0 );
	m_surfacetab_sizer->Add( m_windwave_cbox, 0, wxALL, 5 );


	m_surfacetab_panel->SetSizer( m_surfacetab_sizer );
	m_surfacetab_panel->Layout();
	m_surfacetab_sizer->Fit( m_surfacetab_panel );
	m_dataconfig_nbook->AddPage( m_surfacetab_panel, _("Parameters"), false );

	m_separator_line->Add( m_dataconfig_nbook, wxGBPosition( 0, 2 ), wxGBSpan( 6, 1 ), wxALL|wxEXPAND, 5 );

	m_separator_1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	m_separator_line->Add( m_separator_1, wxGBPosition( 7, 0 ), wxGBSpan( 1, 3 ), wxEXPAND | wxALL, 5 );

	m_download_button = new wxButton( this, wxID_ANY, _("Download"), wxDefaultPosition, wxDefaultSize, 0 );
	m_separator_line->Add( m_download_button, wxGBPosition( 9, 0 ), wxGBSpan( 1, 1 ), wxALL|wxEXPAND, 5 );

	m_progress_gauge = new wxGauge( this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
	m_progress_gauge->SetValue( 0 );
	m_separator_line->Add( m_progress_gauge, wxGBPosition( 9, 1 ), wxGBSpan( 1, 2 ), wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5 );

	m_status_label = new wxStaticText( this, wxID_ANY, _("Status :"), wxDefaultPosition, wxDefaultSize, 0 );
	m_status_label->Wrap( -1 );
	m_separator_line->Add( m_status_label, wxGBPosition( 8, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxALL, 5 );

	m_status_text = new wxStaticText( this, wxID_ANY, _("Ready"), wxDefaultPosition, wxDefaultSize, 0 );
	m_status_text->Wrap( -1 );
	m_separator_line->Add( m_status_text, wxGBPosition( 8, 1 ), wxGBSpan( 1, 2 ), wxALL, 5 );

	m_sizeestimate_label = new wxStaticText( this, wxID_ANY, _("Estimated size :"), wxDefaultPosition, wxDefaultSize, 0 );
	m_sizeestimate_label->Wrap( -1 );
	m_separator_line->Add( m_sizeestimate_label, wxGBPosition( 6, 1 ), wxGBSpan( 1, 1 ), wxALIGN_RIGHT|wxALL, 5 );

	m_sizeestimate_text = new wxStaticText( this, wxID_ANY, _("Unknown"), wxDefaultPosition, wxDefaultSize, 0 );
	m_sizeestimate_text->Wrap( -1 );
	m_separator_line->Add( m_sizeestimate_text, wxGBPosition( 6, 2 ), wxGBSpan( 1, 1 ), wxALL, 5 );


	this->SetSizer( m_separator_line );
	this->Layout();
	m_separator_line->Fit( this );
}

XyGribPanel::~XyGribPanel()
{
}
