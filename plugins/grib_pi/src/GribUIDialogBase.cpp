///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "GribUIDialogBase.h"

///////////////////////////////////////////////////////////////////////////

GRIBUIDialogBase::GRIBUIDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer1->AddGrowableRow( 1 );
	fgSizer1->SetFlexibleDirection( wxVERTICAL );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	wxFlexGridSizer* fgSizer51;
	fgSizer51 = new wxFlexGridSizer( 1, 9, 0, 0 );
	fgSizer51->AddGrowableCol( 1 );
	fgSizer51->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer51->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_NONE );
	
	m_bpPrev = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpPrev->SetToolTip( _("Previous") );
	
	fgSizer51->Add( m_bpPrev, 0, wxALL, 1 );
	
	m_cRecordForecast = new wxComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 ); 
	fgSizer51->Add( m_cRecordForecast, 0, wxALL|wxEXPAND, 1 );
	
	m_bpNext = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpNext->SetToolTip( _("Next") );
	
	fgSizer51->Add( m_bpNext, 0, wxALL, 1 );
	
	m_bpNow = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpNow->SetToolTip( _("Now") );

    fgSizer51->Add( m_bpNow, 0, wxALL, 1 );

    m_bpPlay = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpPlay->SetToolTip( _("Play") );
	
	fgSizer51->Add( m_bpPlay, 0, wxALL, 1 );

    m_sTimeline = new wxSlider( this, ID_TIMELINE, 1, 0, 10, wxDefaultPosition, wxSize( 80,-1 ), wxSL_HORIZONTAL );
	fgSizer51->Add( m_sTimeline, 0, wxEXPAND, 1 );

	m_bpOpenFile = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpOpenFile->SetToolTip( _("Open File") );
	
	fgSizer51->Add( m_bpOpenFile, 0, wxALL, 1 );
	
	m_bpSettings = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpSettings->SetToolTip( _("Settings") );
	
	fgSizer51->Add( m_bpSettings, 0, wxALL, 1 );
	
	m_bpRequest = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpRequest->SetToolTip( _("Request") );
	
	fgSizer51->Add( m_bpRequest, 0, wxALL, 1 );
		
	fgSizer1->Add( fgSizer51, 1, wxEXPAND, 1 );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("GRIB Data at Cursor") ), wxVERTICAL );
	
	m_fgTrackingControls = new wxFlexGridSizer( 0, 6, 0, 0 );
	m_fgTrackingControls->SetFlexibleDirection( wxBOTH );
	m_fgTrackingControls->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_cbWind = new wxCheckBox( this, ID_CB_WIND_SPEED, _("Wind"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbWind, 0, wxALL, 5 );
	
	m_tcWindSpeed = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_tcWindSpeed->SetMaxLength( 3 ); 
	m_fgTrackingControls->Add( m_tcWindSpeed, 0, wxALL, 1 );
	
	m_tcWindDirection = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_tcWindDirection->SetMaxLength( 3 ); 
	m_fgTrackingControls->Add( m_tcWindDirection, 0, wxALL, 1 );
	
	m_cbWindScat = new wxCheckBox( this, wxID_ANY, _("Wind Scat"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbWindScat, 0, wxALL, 1 );
	
	m_tcWindScatSpeed = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_fgTrackingControls->Add( m_tcWindScatSpeed, 0, wxALL, 1 );
	
	m_tcWindScatDirection = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), 0 );
	m_fgTrackingControls->Add( m_tcWindScatDirection, 0, wxALL, 1 );
	
	m_cbWave = new wxCheckBox( this, ID_CB_SIG_WAVE_HEIGHT, _("Wave"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbWave, 0, wxALL, 1 );
	
	m_tcWaveHeight = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_tcWaveHeight->SetMaxLength( 5 ); 
	m_fgTrackingControls->Add( m_tcWaveHeight, 0, wxALL, 1 );
	
	m_tcWaveDirection = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_tcWaveDirection->SetMaxLength( 3 ); 
	m_fgTrackingControls->Add( m_tcWaveDirection, 0, wxALL, 5 );
	
	m_cbCurrent = new wxCheckBox( this, ID_CB_CURRENT_VELOCITY, _("Current"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbCurrent, 0, wxALL, 1 );
	
	m_tcCurrentVelocity = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_tcCurrentVelocity->SetMaxLength( 5 ); 
	m_fgTrackingControls->Add( m_tcCurrentVelocity, 0, wxALL, 1 );
	
	m_tcCurrentDirection = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_tcCurrentDirection->SetMaxLength( 5 ); 
	m_fgTrackingControls->Add( m_tcCurrentDirection, 0, wxALL, 1 );
	
	m_cbWindGust = new wxCheckBox( this, wxID_ANY, _("Wind Gust"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbWindGust, 0, wxALL, 1 );
	
	m_tcWindGust = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_fgTrackingControls->Add( m_tcWindGust, 0, wxALL, 1 );
	
	
	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 1 );
	
	m_cbPressure = new wxCheckBox( this, ID_CB_PRESSURE, _("Pressure"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbPressure, 0, wxALL, 1 );
	
	m_tcPressure = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_tcPressure->SetMaxLength( 5 ); 
	m_fgTrackingControls->Add( m_tcPressure, 0, wxALL, 1 );
	
	
	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 1 );
	
	m_cbPrecipitation = new wxCheckBox( this, wxID_ANY, _("Precipitation"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbPrecipitation, 0, wxALL, 1 );
	
	m_tcPrecipitation = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_fgTrackingControls->Add( m_tcPrecipitation, 0, wxALL, 1 );
	
	
	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 1 );
	
	m_cbCloud = new wxCheckBox( this, wxID_ANY, _("Cloud"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbCloud, 0, wxALL, 1 );
	
	m_tcCloud = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_fgTrackingControls->Add( m_tcCloud, 0, wxALL, 1 );
	
	
	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 1 );
	
	m_cbAirTemperature = new wxCheckBox( this, wxID_ANY, _("Air Temp"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbAirTemperature, 0, wxALL, 1 );
	
	m_tcAirTemperature = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_fgTrackingControls->Add( m_tcAirTemperature, 0, wxALL, 1 );
	
	
	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_cbSeaTemperature = new wxCheckBox( this, ID_CB_SEA_TEMPERATURE, _("Sea Temp"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbSeaTemperature, 0, wxALL, 1 );
	
	m_tcSeaTemperature = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 60,-1 ), wxTE_READONLY );
	m_tcSeaTemperature->SetMaxLength( 5 ); 
	m_fgTrackingControls->Add( m_tcSeaTemperature, 0, wxALL, 1 );
	
	
	sbSizer2->Add( m_fgTrackingControls, 1, wxEXPAND, 1 );
	
	
	fgSizer1->Add( sbSizer2, 1, wxEXPAND, 1 );	
	
	this->SetSizer( fgSizer1 );
	this->Layout();
	fgSizer1->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GRIBUIDialogBase::OnClose ) );
	this->Connect( wxEVT_SIZE, wxSizeEventHandler( GRIBUIDialogBase::OnSize ) );
	m_bpPrev->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnPrev ), NULL, this );
	m_cRecordForecast->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( GRIBUIDialogBase::OnRecordForecast ), NULL, this );
	m_bpNext->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnNext ), NULL, this );
	m_bpNow->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnNow ), NULL, this );
	m_bpOpenFile->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnOpenFile ), NULL, this );
	m_bpSettings->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnSettings ), NULL, this );
	m_bpRequest->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnRequest ), NULL, this );
    m_bpPlay->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnPlayStop ), NULL, this );
	m_sTimeline->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GRIBUIDialogBase::OnTimeline ), NULL, this );
	m_sTimeline->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GRIBUIDialogBase::OnTimeline ), NULL, this );
	m_sTimeline->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GRIBUIDialogBase::OnTimeline ), NULL, this );
	m_sTimeline->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GRIBUIDialogBase::OnTimeline ), NULL, this );
	m_sTimeline->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GRIBUIDialogBase::OnTimeline ), NULL, this );
	m_sTimeline->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GRIBUIDialogBase::OnTimeline ), NULL, this );
	m_sTimeline->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GRIBUIDialogBase::OnTimeline ), NULL, this );
	m_sTimeline->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GRIBUIDialogBase::OnTimeline ), NULL, this );
	m_sTimeline->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GRIBUIDialogBase::OnTimeline ), NULL, this );
	m_cbWind->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbWindScat->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbWave->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbCurrent->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbWindGust->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbPressure->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbPrecipitation->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbCloud->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbAirTemperature->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbSeaTemperature->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
}

GRIBUIDialogBase::~GRIBUIDialogBase()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GRIBUIDialogBase::OnClose ) );
	this->Disconnect( wxEVT_SIZE, wxSizeEventHandler( GRIBUIDialogBase::OnSize ) );
	m_bpPrev->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnPrev ), NULL, this );
	m_cRecordForecast->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( GRIBUIDialogBase::OnRecordForecast ), NULL, this );
	m_bpNext->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnNext ), NULL, this );
	m_bpNow->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnNow ), NULL, this );
	m_bpOpenFile->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnOpenFile ), NULL, this );
	m_bpSettings->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnSettings ), NULL, this );
	m_bpRequest->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnRequest ), NULL, this );
    m_bpPlay->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnPlayStop ), NULL, this );
	m_sTimeline->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GRIBUIDialogBase::OnTimeline ), NULL, this );
	m_sTimeline->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GRIBUIDialogBase::OnTimeline ), NULL, this );
	m_sTimeline->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GRIBUIDialogBase::OnTimeline ), NULL, this );
	m_sTimeline->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GRIBUIDialogBase::OnTimeline ), NULL, this );
	m_sTimeline->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GRIBUIDialogBase::OnTimeline ), NULL, this );
	m_sTimeline->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GRIBUIDialogBase::OnTimeline ), NULL, this );
	m_sTimeline->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GRIBUIDialogBase::OnTimeline ), NULL, this );
	m_sTimeline->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GRIBUIDialogBase::OnTimeline ), NULL, this );
	m_sTimeline->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GRIBUIDialogBase::OnTimeline ), NULL, this );
	m_cbWind->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbWindScat->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbWave->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbCurrent->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbWindGust->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbPressure->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbPrecipitation->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbCloud->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbAirTemperature->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbSeaTemperature->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	
}

GribSettingsDialogBase::GribSettingsDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxSize( -1,-1 ) );
	
	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("playback") ), wxVERTICAL );
	
	wxGridSizer* gSizer2;
	gSizer2 = new wxGridSizer( 0, 2, 0, 0 );
	
	m_cInterpolate = new wxCheckBox( this, wxID_ANY, _("Interpolate between gribs"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cInterpolate->SetValue(true); 
	gSizer2->Add( m_cInterpolate, 0, 0, 5 );
	
	m_cLoopMode = new wxCheckBox( this, wxID_ANY, _("Loop Mode"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer2->Add( m_cLoopMode, 0, 0, 5 );
	
	m_staticText5 = new wxStaticText( this, wxID_ANY, _("Slices per Update"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText5->Wrap( -1 );
	gSizer2->Add( m_staticText5, 0, 0, 5 );
	
	m_sSlicesPerUpdate = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 100, 2 );
	gSizer2->Add( m_sSlicesPerUpdate, 0, wxALL, 5 );
	
	m_staticText9 = new wxStaticText( this, wxID_ANY, _("Updates per Second"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText9->Wrap( -1 );
	gSizer2->Add( m_staticText9, 0, wxALL, 5 );
	
	m_sUpdatesPerSecond = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 60, 4 );
	gSizer2->Add( m_sUpdatesPerSecond, 0, wxALL, 5 );
	
	m_staticText4 = new wxStaticText( this, wxID_ANY, _("Slices per hour"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	gSizer2->Add( m_staticText4, 0, wxALL, 5 );
	
	m_sHourDivider = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 20, 2 );
	gSizer2->Add( m_sHourDivider, 0, wxALL, 5 );
	
	
	sbSizer4->Add( gSizer2, 1, 0, 5 );
	
	
	fgSizer4->Add( sbSizer4, 1, 0, 5 );
	
	wxStaticBoxSizer* sbSizer5;
	sbSizer5 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Display") ), wxVERTICAL );
	
	wxGridSizer* gSizer3;
	gSizer3 = new wxGridSizer( 0, 3, 0, 0 );
	
	wxArrayString m_cDataTypeChoices;
	m_cDataType = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cDataTypeChoices, 0 );
	m_cDataType->SetSelection( 0 );
	gSizer3->Add( m_cDataType, 0, wxALL, 5 );
	
	m_staticText12 = new wxStaticText( this, wxID_ANY, _("Units"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	gSizer3->Add( m_staticText12, 0, wxALL, 5 );
	
	wxArrayString m_cDataUnitsChoices;
	m_cDataUnits = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cDataUnitsChoices, 0 );
	m_cDataUnits->SetSelection( 0 );
	gSizer3->Add( m_cDataUnits, 0, wxALL, 5 );
	
	m_cbBarbedArrows = new wxCheckBox( this, wxID_ANY, _("Barbed Arrows"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer3->Add( m_cbBarbedArrows, 0, wxALL, 5 );
	
	m_staticText10 = new wxStaticText( this, wxID_ANY, _("Range"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10->Wrap( -1 );
	gSizer3->Add( m_staticText10, 0, wxALL, 5 );
	
	m_sBarbedRange = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0 );
	gSizer3->Add( m_sBarbedRange, 0, wxALL, 5 );
	
	m_cbIsoBars = new wxCheckBox( this, wxID_ANY, _("Iso Bars"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer3->Add( m_cbIsoBars, 0, wxALL, 5 );
	
	m_staticText6 = new wxStaticText( this, wxID_ANY, _("Spacing"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->Wrap( -1 );
	gSizer3->Add( m_staticText6, 0, wxALL, 5 );
	
	m_sIsoBarSpacing = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 1000, 0 );
	gSizer3->Add( m_sIsoBarSpacing, 0, wxALL, 5 );
	
	m_cbDirectionArrows = new wxCheckBox( this, wxID_ANY, _("Direction Arrows"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer3->Add( m_cbDirectionArrows, 0, wxALL, 5 );
	
	m_staticText51 = new wxStaticText( this, wxID_ANY, _("Size"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText51->Wrap( -1 );
	gSizer3->Add( m_staticText51, 0, wxALL, 5 );
	
	m_sDirectionArrowSize = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 10, 0 );
	gSizer3->Add( m_sDirectionArrowSize, 0, wxALL, 5 );
	
	m_cbOverlayMap = new wxCheckBox( this, wxID_ANY, _("OverlayMap"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer3->Add( m_cbOverlayMap, 0, wxALL, 5 );
	
	m_staticText7 = new wxStaticText( this, wxID_ANY, _("Colors"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText7->Wrap( -1 );
	gSizer3->Add( m_staticText7, 0, wxALL, 5 );
	
	wxString m_cOverlayColorsChoices[] = { _("Current"), _("Generic"), _("Quickscat"), _("SeaTemp"), _("Rain"), _("Cloud") };
	int m_cOverlayColorsNChoices = sizeof( m_cOverlayColorsChoices ) / sizeof( wxString );
	m_cOverlayColors = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cOverlayColorsNChoices, m_cOverlayColorsChoices, 0 );
	m_cOverlayColors->SetSelection( 0 );
	gSizer3->Add( m_cOverlayColors, 0, wxALL, 5 );
	
	m_cbNumbers = new wxCheckBox( this, wxID_ANY, _("Numbers"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer3->Add( m_cbNumbers, 0, wxALL, 5 );
	
	m_staticText8 = new wxStaticText( this, wxID_ANY, _("Spacing"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText8->Wrap( -1 );
	gSizer3->Add( m_staticText8, 0, wxALL, 5 );
	
	m_sNumbersSpacing = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 50 );
	gSizer3->Add( m_sNumbersSpacing, 0, wxALL, 5 );
	
	
	sbSizer5->Add( gSizer3, 1, wxEXPAND, 5 );
	
	
	fgSizer4->Add( sbSizer5, 1, wxFIXED_MINSIZE, 5 );
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton( this, wxID_OK );
	m_sdbSizer1->AddButton( m_sdbSizer1OK );
	m_sdbSizer1Apply = new wxButton( this, wxID_APPLY );
	m_sdbSizer1->AddButton( m_sdbSizer1Apply );
	m_sdbSizer1Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer1->AddButton( m_sdbSizer1Cancel );
	m_sdbSizer1->Realize();
	
	fgSizer4->Add( m_sdbSizer1, 1, 0, 5 );
	
	
	this->SetSizer( fgSizer4 );
	this->Layout();
	fgSizer4->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_cDataType->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribSettingsDialogBase::OnDataTypeChoice ), NULL, this );
	m_sdbSizer1Apply->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnApply ), NULL, this );
}

GribSettingsDialogBase::~GribSettingsDialogBase()
{
	// Disconnect Events
	m_cDataType->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribSettingsDialogBase::OnDataTypeChoice ), NULL, this );
	m_sdbSizer1Apply->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnApply ), NULL, this );
	
}

GribPreferencesDialogBase::GribPreferencesDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxFlexGridSizer* fgSizer8;
	fgSizer8 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer8->SetFlexibleDirection( wxBOTH );
	fgSizer8->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_cbUseHiDef = new wxCheckBox( this, wxID_ANY, _("Use High Definition Graphics"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer6->Add( m_cbUseHiDef, 0, wxALL, 5 );
	
	m_cbUseGradualColors = new wxCheckBox( this, wxID_ANY, _("Use Gradual Colors"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer6->Add( m_cbUseGradualColors, 0, wxALL, 5 );
	
	m_cbCopyFirstCumulativeRecord = new wxCheckBox( this, wxID_ANY, _("Copy First Cumulative Missing Record"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer6->Add( m_cbCopyFirstCumulativeRecord, 0, wxALL, 5 );
	
	m_cbCopyMissingWaveRecord = new wxCheckBox( this, wxID_ANY, _("Copy Missing Wave Records"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer6->Add( m_cbCopyMissingWaveRecord, 0, wxALL, 5 );

    const wxString options[] = { _("Load the More Recent File in Directory"), _("Load the Last Open File") };
    m_rbStartOptions = new wxRadioBox( this, wxID_ANY, _("Load File Options"), wxDefaultPosition, wxDefaultSize, 2, options, 2, wxRA_SPECIFY_ROWS );
    fgSizer6->Add( m_rbStartOptions, 0, wxALL|wxEXPAND, 5 );

    const wxString format[] = { _("Local Time"), _("UTC") };
    m_rbTimeFormat = new wxRadioBox( this, wxID_ANY, _("Time Options"), wxDefaultPosition, wxDefaultSize, 2, format, 2, wxRA_SPECIFY_ROWS );
    fgSizer6->Add( m_rbTimeFormat, 0, wxALL|wxEXPAND, 5 );

	fgSizer8->Add( fgSizer6, 1, wxEXPAND, 5 );
	
	m_sdbSizer2 = new wxStdDialogButtonSizer();
	m_sdbSizer2OK = new wxButton( this, wxID_OK );
	m_sdbSizer2->AddButton( m_sdbSizer2OK );
	m_sdbSizer2Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer2->AddButton( m_sdbSizer2Cancel );
	m_sdbSizer2->Realize();
	
	fgSizer8->Add( m_sdbSizer2, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( fgSizer8 );
	this->Layout();
	fgSizer8->Fit( this );
	
	this->Centre( wxBOTH );
}

GribPreferencesDialogBase::~GribPreferencesDialogBase()
{
}


//-------------------------------------------------------------------------------------------------------
//Request Preferences Dialog implementation
//---------------------------------------------------------------------------------------------------------
GribRequestSettingBase::GribRequestSettingBase( wxWindow* parent, wxWindowID id,
    const wxString& title, const wxPoint& pos, const wxSize& size, long style) 
    : wxDialog( parent, id, title, pos, size, style)
{
    int border_size = 5;

    wxBoxSizer* bsRequest1 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(bsRequest1);

     // head request Setting
    wxStaticBoxSizer* sbsRequest1;
	sbsRequest1 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Profil") ), wxVERTICAL );
	bsRequest1->Add( sbsRequest1, 0, wxGROW|wxALL, border_size );

    wxFlexGridSizer *m_pTopSizer = new wxFlexGridSizer(4);
    sbsRequest1->Add( m_pTopSizer, 0, wxALL | wxEXPAND, border_size );

    wxStaticText* m_mailto_text = new wxStaticText( this, wxID_ANY, _("Mail To "), wxDefaultPosition, wxSize(-1, -1) );
    m_pTopSizer->Add( m_mailto_text, 0, wxALIGN_LEFT | wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, border_size );

    const wxString dest[] = { _T("Saildocs") };
    m_pMailTo = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 1, dest, wxALIGN_LEFT );
    m_pTopSizer->Add( m_pMailTo, 0, wxTOP, border_size );
    m_pMailTo->SetSelection( 0 );

    m_pTopSizer->Add( 0, 0, 0, 0 );
    m_pTopSizer->Add( 0, 0, 0, 0 );

    wxStaticText* m_model_text = new wxStaticText( this, wxID_ANY, _("Forecast Model "), wxDefaultPosition, wxSize(-1, -1) );
    m_pTopSizer->Add( m_model_text, 0, wxALIGN_LEFT | wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, border_size );

    const wxString model[] = { wxT("NOAA_GFS"), wxT("NOAA_COAMPS") ,wxT("NOAA_RTOFS") };
    m_pModel = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, model, wxALIGN_LEFT );
    m_pTopSizer->Add( m_pModel, 0, wxTOP, border_size );
  
    wxStaticText* m_Resolution_text = new wxStaticText( this, wxID_ANY, _("Resolution"), wxDefaultPosition, wxSize(-1, -1) );
    m_pTopSizer->Add( m_Resolution_text, 0, wxALIGN_LEFT | wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, border_size );

    m_pResolution = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, resolution0, wxALIGN_LEFT );
 
    m_pTopSizer->Add( m_pResolution , 0, wxTOP, border_size );

    wxStaticText* m_Interval_text = new wxStaticText( this, wxID_ANY, _("Interval"), wxDefaultPosition, wxSize(-1, -1) );
    m_pTopSizer->Add( m_Interval_text, 0, wxALIGN_LEFT | wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, border_size );

    const wxString interval[] = { _("3 h"), _("6 h"), _("12 h") };
    m_pInterval = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, interval, wxALIGN_LEFT );
    m_pTopSizer->Add( m_pInterval, 0, wxTOP, border_size );

    wxStaticText* m_Range_text = new wxStaticText( this, wxID_ANY, _T("Time Range"), wxDefaultPosition, wxSize(-1, -1) );
    m_pTopSizer->Add( m_Range_text, 0, wxALIGN_LEFT | wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, border_size );

    const wxString range[] = { _("4 days"), _("6 days"), _("8 days") };
    m_pTimeRange = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, range, wxALIGN_LEFT );
    m_pTopSizer->Add( m_pTimeRange, 0, wxTOP, border_size );
  
    // data request Setting
    wxStaticBoxSizer* sbsRequest2;
	sbsRequest2 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Data available in the selected forecast model") ), wxVERTICAL );
	bsRequest1->Add( sbsRequest2, 0, wxGROW|wxALL, border_size );

    wxFlexGridSizer *pTopSizer2 = new wxFlexGridSizer( 2 );
    sbsRequest2->Add( pTopSizer2, 0, wxALL | wxEXPAND, border_size );

    m_pWind = new wxCheckBox( this, -1, _("Wind"));
    pTopSizer2->Add( m_pWind, 1, wxALIGN_LEFT|wxALL, border_size );

    m_pPress = new wxCheckBox( this, -1, _("Pressure"));
    pTopSizer2->Add( m_pPress, 1, wxALIGN_LEFT|wxALL, border_size );
  
    m_pWaves = new wxCheckBox( this, -1, _("Waves"));
    pTopSizer2->Add( m_pWaves, 1, wxALIGN_LEFT|wxALL, border_size );
    
    m_pRainfall = new wxCheckBox( this, -1, _("Rainfall"));
    pTopSizer2->Add( m_pRainfall, 1, wxALIGN_LEFT|wxALL, border_size );
   
    m_pCloudCover = new wxCheckBox( this, -1, _("Clouds Cover"));
    pTopSizer2->Add( m_pCloudCover, 1, wxALIGN_LEFT|wxALL, border_size );
    
    m_pAirTemp = new wxCheckBox( this, -1, _("Air Temperature(2m)"));
    pTopSizer2->Add( m_pAirTemp, 1, wxALIGN_LEFT|wxALL, border_size );
    
    m_pSeaTemp = new wxCheckBox( this, -1, _("Sea Temperature(surf.)"));
    pTopSizer2->Add( m_pSeaTemp, 1, wxALIGN_LEFT|wxALL, border_size );

    m_pCurrent = new wxCheckBox( this, -1, _("Current(Surf.)"));
    pTopSizer2->Add( m_pCurrent, 1, wxALIGN_LEFT|wxALL, border_size );
   
    wxStaticBoxSizer* sbsRequest3;
	sbsRequest3 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Mail") ), wxVERTICAL );
	bsRequest1->Add( sbsRequest3, 0, wxGROW|wxALL, border_size );

    m_MailImage = new wxStaticText( this, wxID_ANY, _(""), wxDefaultPosition, wxSize(-1, -1) );
    sbsRequest3->Add( m_MailImage, 0, wxALIGN_LEFT | wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, border_size );

    wxStdDialogButtonSizer* DialogButtonSizer = new wxStdDialogButtonSizer();
    m_bSave = new wxButton( this, wxID_ANY, _("Save") );
    DialogButtonSizer->SetNegativeButton( m_bSave );
    m_bSend = new wxButton( this, wxID_ANY, _("Send") );
    DialogButtonSizer->SetAffirmativeButton( m_bSend );
    wxButton *m_Cancel = new wxButton( this, wxID_CANCEL );
    DialogButtonSizer->AddButton( m_Cancel );
    DialogButtonSizer->Realize();

    bsRequest1->Add(DialogButtonSizer, 0, wxALIGN_RIGHT|wxALL, border_size);

    Fit();

    // Connect Events
    m_pModel->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(GribRequestSettingBase::OnModelChange), NULL, this );
    m_pResolution->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pInterval->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pTimeRange->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pWind->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pPress->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pWaves->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pRainfall->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pCloudCover->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pAirTemp->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pSeaTemp->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pCurrent->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_bSend->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnSendMaiL), NULL, this );
    m_bSave->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnSaveRequest), NULL, this );
 }

GribRequestSettingBase::~GribRequestSettingBase()
{
	// Disconnect Events
	m_pModel->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(GribRequestSettingBase::OnModelChange), NULL, this );
	m_pResolution->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pInterval->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pTimeRange->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pWind->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pPress->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pWaves->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pRainfall->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pCloudCover->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pAirTemp->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pSeaTemp->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_pCurrent->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnAnyChange), NULL, this );
    m_bSend->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnSendMaiL), NULL, this );
    m_bSave->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(GribRequestSettingBase::OnSaveRequest), NULL, this );
}