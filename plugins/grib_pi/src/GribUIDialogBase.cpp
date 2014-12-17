///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun  6 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "GribUIDialogBase.h"

///////////////////////////////////////////////////////////////////////////

GRIBUIDialogBase::GRIBUIDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	this->SetFont( wxFont( 9, 70, 90, 90, false, wxEmptyString ) );
	
	m_fgTrackingDisplay = new wxFlexGridSizer( 0, 1, 0, 0 );
	m_fgTrackingDisplay->AddGrowableRow( 1 );
	m_fgTrackingDisplay->SetFlexibleDirection( wxVERTICAL );
	m_fgTrackingDisplay->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	wxFlexGridSizer* fgSizer51;
	fgSizer51 = new wxFlexGridSizer( 1, 10, 0, 0 );
	fgSizer51->AddGrowableCol( 1 );
	fgSizer51->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer51->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_NONE );
	
	m_bpPrev = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpPrev->SetToolTip( _("Previous") );
	
	fgSizer51->Add( m_bpPrev, 0, wxALL, 1 );
	
	wxArrayString m_cRecordForecastChoices;
	m_cRecordForecast = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cRecordForecastChoices, 0 );
	m_cRecordForecast->SetSelection( 0 );
	fgSizer51->Add( m_cRecordForecast, 0, wxALL|wxEXPAND, 5 );
	
	m_bpNext = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpNext->SetToolTip( _("Next") );
	
	fgSizer51->Add( m_bpNext, 0, wxALL, 1 );
	
	m_bpNow = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpNow->SetToolTip( _("Now") );
	
	fgSizer51->Add( m_bpNow, 0, wxALL, 1 );
	
	m_bpZoomToCenter = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpZoomToCenter->SetToolTip( _("Zoom To Center") );
	
	fgSizer51->Add( m_bpZoomToCenter, 0, wxALL, 1 );
	
	m_bpPlay = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpPlay->SetToolTip( _("Play") );
	
	fgSizer51->Add( m_bpPlay, 0, wxALL, 1 );
	
	m_sTimeline = new wxSlider( this, ID_TIMELINE, 1, 0, 10, wxDefaultPosition, wxSize( 90,-1 ), wxSL_HORIZONTAL );
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
	
	
	m_fgTrackingDisplay->Add( fgSizer51, 1, wxEXPAND, 1 );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("GRIB Data at Cursor") ), wxVERTICAL );
	
	m_fgTrackingControls = new wxFlexGridSizer( 0, 6, 0, 0 );
	m_fgTrackingControls->SetFlexibleDirection( wxBOTH );
	m_fgTrackingControls->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_fcAltitude = new wxFlexGridSizer( 0, 2, 0, 0 );
	m_fcAltitude->SetFlexibleDirection( wxBOTH );
	m_fcAltitude->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxArrayString m_cbAltitudeChoices;
	m_cbAltitude = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( -1,-1 ), m_cbAltitudeChoices, 0 );
	m_cbAltitude->SetSelection( 0 );
	m_cbAltitude->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );
	
	m_fcAltitude->Add( m_cbAltitude, 0, wxLEFT, 3 );
	
	m_cbWind = new wxCheckBox( this, ID_CB_WIND, _("Wind"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fcAltitude->Add( m_cbWind, 0, wxLEFT|wxTOP, 2 );
	
	
	m_fgTrackingControls->Add( m_fcAltitude, 1, wxEXPAND, 5 );
	
	m_tcWindSpeed = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), wxTE_READONLY );
	m_fgTrackingControls->Add( m_tcWindSpeed, 0, wxALL, 1 );
	
	m_tcWindDirection = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 45,-1 ), wxTE_READONLY );
	m_fgTrackingControls->Add( m_tcWindDirection, 0, wxALL, 1 );
	
	m_cbWave = new wxCheckBox( this, ID_CB_WAVES, _("Waves"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbWave, 0, wxALL, 5 );
	
	m_tcWaveHeight = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), wxTE_READONLY );
	m_tcWaveHeight->SetToolTip( _("Significant Wave Height") );
	
	m_fgTrackingControls->Add( m_tcWaveHeight, 0, wxALL, 1 );
	
	m_tcWaveDirection = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 45,-1 ), wxTE_READONLY );
	m_tcWaveDirection->SetToolTip( _("Waves Direction") );
	
	m_fgTrackingControls->Add( m_tcWaveDirection, 0, wxALL, 1 );
	
	m_cbCurrent = new wxCheckBox( this, ID_CB_CURRENT, _("Current"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbCurrent, 0, wxALL, 5 );
	
	m_tcCurrentVelocity = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), wxTE_READONLY );
	m_tcCurrentVelocity->SetToolTip( _("Surface Current Speed") );
	
	m_fgTrackingControls->Add( m_tcCurrentVelocity, 0, wxALL, 1 );
	
	m_tcCurrentDirection = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 45,-1 ), wxTE_READONLY );
	m_tcCurrentDirection->SetToolTip( _("Surface Current Direction") );
	
	m_fgTrackingControls->Add( m_tcCurrentDirection, 0, wxALL, 1 );
	
	m_cbWindGust = new wxCheckBox( this, ID_CB_WIND_GUSTS, _("Wind Gust"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbWindGust, 0, wxALL, 5 );
	
	m_tcWindGust = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), wxTE_READONLY );
	m_fgTrackingControls->Add( m_tcWindGust, 0, wxALL, 1 );
	
	
	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 1 );
	
	m_cbPressure = new wxCheckBox( this, ID_CB_PRESSURE, _("Pressure"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbPressure, 0, wxALL, 5 );
	
	m_tcPressure = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), wxTE_READONLY );
	m_tcPressure->SetToolTip( _("Mean Sea Level Pressure (MSLP)") );
	
	m_fgTrackingControls->Add( m_tcPressure, 0, wxALL, 1 );
	
	
	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 1 );
	
	m_cbPrecipitation = new wxCheckBox( this, ID_CB_RAINFALL, _("Rainfall"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbPrecipitation, 0, wxALL, 5 );
	
	m_tcPrecipitation = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), wxTE_READONLY );
	m_tcPrecipitation->SetToolTip( _("Cumulative Rainfall per Hour") );
	
	m_fgTrackingControls->Add( m_tcPrecipitation, 0, wxALL, 1 );
	
	
	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 1 );
	
	m_cbCloud = new wxCheckBox( this, ID_CB_CLOUD_COVER, _("Cloud Cover"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbCloud, 0, wxALL, 5 );
	
	m_tcCloud = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), wxTE_READONLY );
	m_tcCloud->SetToolTip( _("Cloud Cover") );
	
	m_fgTrackingControls->Add( m_tcCloud, 0, wxALL, 1 );
	
	
	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 1 );
	
	m_cbAirTemperature = new wxCheckBox( this, ID_CB_AIR_TEMP, _("Air Temp"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbAirTemperature, 0, wxALL, 5 );
	
	m_tcAirTemperature = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), wxTE_READONLY );
	m_fgTrackingControls->Add( m_tcAirTemperature, 0, wxALL, 1 );
	
	
	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 1 );
	
	m_cbSeaTemperature = new wxCheckBox( this, ID_CB_SEA_TEMP, _("Sea Temp"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbSeaTemperature, 0, wxALL, 5 );
	
	m_tcSeaTemperature = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), wxTE_READONLY );
	m_tcSeaTemperature->SetToolTip( _("Sea or Air Temperature at Surface Level") );
	
	m_fgTrackingControls->Add( m_tcSeaTemperature, 0, wxALL, 1 );
	
	
	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_cbCAPE = new wxCheckBox( this, ID_CB_CAPE, _("CAPE"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbCAPE, 0, wxALL, 5 );
	
	m_tcCAPE = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxPoint( -1,-1 ), wxSize( 70,-1 ), wxTE_READONLY );
	m_tcCAPE->SetToolTip( _("Convective Available Potential Energy") );
	
	m_fgTrackingControls->Add( m_tcCAPE, 0, wxALL, 1 );
	
	
	sbSizer2->Add( m_fgTrackingControls, 1, wxEXPAND, 1 );
	
	
	m_fgTrackingDisplay->Add( sbSizer2, 1, wxFIXED_MINSIZE, 1 );
	
	wxFlexGridSizer* fgSizer30;
	fgSizer30 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer30->SetFlexibleDirection( wxBOTH );
	fgSizer30->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer31;
	fgSizer31 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer31->SetFlexibleDirection( wxBOTH );
	fgSizer31->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_stAltitudeText = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_stAltitudeText->Wrap( -1 );
	fgSizer31->Add( m_stAltitudeText, 0, wxEXPAND|wxLEFT|wxRIGHT, 5 );
	
	
	fgSizer30->Add( fgSizer31, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer27;
	fgSizer27 = new wxFlexGridSizer( 0, 8, 0, 0 );
	fgSizer27->SetFlexibleDirection( wxBOTH );
	fgSizer27->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticText* m_staticText35;
	m_staticText35 = new wxStaticText( this, wxID_ANY, _("Altitude"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText35->Wrap( -1 );
	fgSizer27->Add( m_staticText35, 0, wxALL, 5 );
	
	m_tcAltitude = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxTE_READONLY );
	m_tcAltitude->SetMinSize( wxSize( 70,-1 ) );
	
	fgSizer27->Add( m_tcAltitude, 0, wxALL, 5 );
	
	wxStaticText* m_staticText36;
	m_staticText36 = new wxStaticText( this, wxID_ANY, _("Temperature"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText36->Wrap( -1 );
	fgSizer27->Add( m_staticText36, 0, wxALL, 5 );
	
	m_tcTemp = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_tcTemp->SetMinSize( wxSize( 70,-1 ) );
	
	fgSizer27->Add( m_tcTemp, 0, wxALL, 5 );
	
	wxStaticText* m_staticText37;
	m_staticText37 = new wxStaticText( this, wxID_ANY, _("Rel. Humid."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText37->Wrap( -1 );
	fgSizer27->Add( m_staticText37, 0, wxALL, 5 );
	
	m_tcRelHumid = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_tcRelHumid->SetMinSize( wxSize( 70,-1 ) );
	
	fgSizer27->Add( m_tcRelHumid, 0, wxALL, 5 );
	
	
	fgSizer30->Add( fgSizer27, 1, wxEXPAND, 5 );
	
	
	m_fgTrackingDisplay->Add( fgSizer30, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( m_fgTrackingDisplay );
	this->Layout();
	m_fgTrackingDisplay->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GRIBUIDialogBase::OnClose ) );
	this->Connect( wxEVT_SIZE, wxSizeEventHandler( GRIBUIDialogBase::OnSize ) );
	m_bpPrev->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnPrev ), NULL, this );
	m_cRecordForecast->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GRIBUIDialogBase::OnRecordForecast ), NULL, this );
	m_bpNext->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnNext ), NULL, this );
	m_bpNow->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnNow ), NULL, this );
	m_bpZoomToCenter->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnZoomToCenterClick ), NULL, this );
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
	m_bpOpenFile->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnOpenFile ), NULL, this );
	m_bpSettings->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnSettings ), NULL, this );
	m_bpRequest->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnRequest ), NULL, this );
	m_cbAltitude->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GRIBUIDialogBase::OnAltitudeChange ), NULL, this );
	m_cbWind->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbWind->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
	m_cbWave->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbWave->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
	m_cbCurrent->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbCurrent->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
	m_cbWindGust->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbWindGust->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
	m_cbPressure->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbPressure->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
	m_cbPrecipitation->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbPrecipitation->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
	m_cbCloud->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbCloud->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
	m_cbAirTemperature->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbAirTemperature->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
	m_cbSeaTemperature->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbSeaTemperature->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
	m_cbCAPE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbCAPE->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
}

GRIBUIDialogBase::~GRIBUIDialogBase()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GRIBUIDialogBase::OnClose ) );
	this->Disconnect( wxEVT_SIZE, wxSizeEventHandler( GRIBUIDialogBase::OnSize ) );
	m_bpPrev->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnPrev ), NULL, this );
	m_cRecordForecast->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GRIBUIDialogBase::OnRecordForecast ), NULL, this );
	m_bpNext->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnNext ), NULL, this );
	m_bpNow->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnNow ), NULL, this );
	m_bpZoomToCenter->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnZoomToCenterClick ), NULL, this );
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
	m_bpOpenFile->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnOpenFile ), NULL, this );
	m_bpSettings->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnSettings ), NULL, this );
	m_bpRequest->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnRequest ), NULL, this );
	m_cbAltitude->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GRIBUIDialogBase::OnAltitudeChange ), NULL, this );
	m_cbWind->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbWind->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
	m_cbWave->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbWave->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
	m_cbCurrent->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbCurrent->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
	m_cbWindGust->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbWindGust->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
	m_cbPressure->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbPressure->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
	m_cbPrecipitation->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbPrecipitation->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
	m_cbCloud->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbCloud->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
	m_cbAirTemperature->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbAirTemperature->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
	m_cbSeaTemperature->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbSeaTemperature->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
	m_cbCAPE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GRIBUIDialogBase::OnCBAny ), NULL, this );
	m_cbCAPE->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUIDialogBase::OnMouseEvent ), NULL, this );
	
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
	
	wxFlexGridSizer* fgSizer13;
	fgSizer13 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer13->SetFlexibleDirection( wxBOTH );
	fgSizer13->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_cLoopMode = new wxCheckBox( this, wxID_ANY, _("Loop Mode"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer13->Add( m_cLoopMode, 0, wxALL, 5 );
	
	m_staticText26 = new wxStaticText( this, wxID_ANY, _("Loop Start"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText26->Wrap( -1 );
	fgSizer13->Add( m_staticText26, 0, wxALL|wxEXPAND, 5 );
	
	wxString m_cLoopStartPointChoices[] = { _("Top of Grib File"), _("Current time forecast") };
	int m_cLoopStartPointNChoices = sizeof( m_cLoopStartPointChoices ) / sizeof( wxString );
	m_cLoopStartPoint = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cLoopStartPointNChoices, m_cLoopStartPointChoices, 0 );
	m_cLoopStartPoint->SetSelection( 0 );
	fgSizer13->Add( m_cLoopStartPoint, 0, wxALL|wxEXPAND, 5 );
	
	
	fgSizer13->Add( 0, 0, 1, wxEXPAND, 5 );
	
	wxStaticText* m_staticText4;
	m_staticText4 = new wxStaticText( this, wxID_ANY, _("Updates per Second"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	fgSizer13->Add( m_staticText4, 0, wxALL|wxEXPAND, 5 );
	
	m_sUpdatesPerSecond = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 90,-1 ), wxSP_ARROW_KEYS, 1, 60, 4 );
	fgSizer13->Add( m_sUpdatesPerSecond, 0, wxALL, 5 );
	
	m_cInterpolate = new wxCheckBox( this, wxID_ANY, _("Interpolate between gribs"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer13->Add( m_cInterpolate, 0, wxALL, 5 );
	
	m_tSlicesPerUpdate = new wxStaticText( this, wxID_ANY, _("Time Interval"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tSlicesPerUpdate->Wrap( -1 );
	fgSizer13->Add( m_tSlicesPerUpdate, 0, wxALL|wxEXPAND, 5 );
	
	wxArrayString m_sSlicesPerUpdateChoices;
	m_sSlicesPerUpdate = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_sSlicesPerUpdateChoices, 0 );
	m_sSlicesPerUpdate->SetSelection( 0 );
	fgSizer13->Add( m_sSlicesPerUpdate, 0, wxALL|wxEXPAND, 5 );
	
	
	sbSizer4->Add( fgSizer13, 1, wxEXPAND, 5 );
	
	
	fgSizer4->Add( sbSizer4, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer5;
	sbSizer5 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Data") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer15;
	fgSizer15 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer15->SetFlexibleDirection( wxBOTH );
	fgSizer15->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxArrayString m_cDataTypeChoices;
	m_cDataType = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cDataTypeChoices, 0 );
	m_cDataType->SetSelection( 0 );
	fgSizer15->Add( m_cDataType, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticText* m_staticText12;
	m_staticText12 = new wxStaticText( this, wxID_ANY, _("Units"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	m_staticText12->SetMinSize( wxSize( 110,-1 ) );
	
	fgSizer15->Add( m_staticText12, 0, wxALL|wxEXPAND, 5 );
	
	wxArrayString m_cDataUnitsChoices;
	m_cDataUnits = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cDataUnitsChoices, 0 );
	m_cDataUnits->SetSelection( 0 );
	m_cDataUnits->SetMinSize( wxSize( 100,-1 ) );
	
	fgSizer15->Add( m_cDataUnits, 0, wxALL|wxEXPAND, 5 );
	
	m_cbBarbedArrows = new wxCheckBox( this, wxID_ANY, _("Barbed Arrows"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer15->Add( m_cbBarbedArrows, 0, wxALL, 5 );
	
	wxString m_cBarbedColoursChoices[] = { _("Default Colour"), _("Controled Colours") };
	int m_cBarbedColoursNChoices = sizeof( m_cBarbedColoursChoices ) / sizeof( wxString );
	m_cBarbedColours = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cBarbedColoursNChoices, m_cBarbedColoursChoices, 0 );
	m_cBarbedColours->SetSelection( 0 );
	fgSizer15->Add( m_cBarbedColours, 0, wxALL|wxEXPAND, 5 );
	
	m_cBarbedVisibility = new wxCheckBox( this, wxID_ANY, _("Always visible"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer15->Add( m_cBarbedVisibility, 0, wxALL|wxEXPAND, 5 );
	
	m_cbIsoBars = new wxCheckBox( this, wxID_ANY, _("IsoBars"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer15->Add( m_cbIsoBars, 0, wxALL, 5 );
	
	m_fIsoBarSpacing = new wxFlexGridSizer( 0, 2, 0, 0 );
	m_fIsoBarSpacing->SetFlexibleDirection( wxBOTH );
	m_fIsoBarSpacing->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticText* m_tIsoBarSpacing;
	m_tIsoBarSpacing = new wxStaticText( this, wxID_ANY, _("Spacing"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tIsoBarSpacing->Wrap( -1 );
	m_fIsoBarSpacing->Add( m_tIsoBarSpacing, 0, wxBOTTOM|wxEXPAND|wxRIGHT, 5 );
	
	m_sIsoBarSpacing = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 90,-1 ), wxSP_ARROW_KEYS, 1, 1000, 1 );
	m_fIsoBarSpacing->Add( m_sIsoBarSpacing, 0, wxALL, 5 );
	
	
	fgSizer15->Add( m_fIsoBarSpacing, 1, wxALL|wxEXPAND, 5 );
	
	m_fIsoBarVisibility = new wxFlexGridSizer( 0, 1, 0, 0 );
	m_fIsoBarVisibility->SetFlexibleDirection( wxBOTH );
	m_fIsoBarVisibility->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_sIsoBarVisibility = new wxCheckBox( this, wxID_ANY, _("Always visible"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fIsoBarVisibility->Add( m_sIsoBarVisibility, 0, wxALL|wxEXPAND, 5 );
	
	
	fgSizer15->Add( m_fIsoBarVisibility, 1, wxALL|wxEXPAND, 5 );
	
	m_cbDirectionArrows = new wxCheckBox( this, wxID_ANY, _("Direction Arrows"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer15->Add( m_cbDirectionArrows, 0, wxALL, 5 );
	
	wxString m_cDirectionArrowFormChoices[] = { _("Single Arrow"), _("Double Arrow"), _("Proportional Arrow") };
	int m_cDirectionArrowFormNChoices = sizeof( m_cDirectionArrowFormChoices ) / sizeof( wxString );
	m_cDirectionArrowForm = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cDirectionArrowFormNChoices, m_cDirectionArrowFormChoices, 0 );
	m_cDirectionArrowForm->SetSelection( 1 );
	fgSizer15->Add( m_cDirectionArrowForm, 0, wxALL|wxEXPAND, 5 );
	
	wxString m_cDirectionArrowSizeChoices[] = { _("Default Size"), _("Small Size") };
	int m_cDirectionArrowSizeNChoices = sizeof( m_cDirectionArrowSizeChoices ) / sizeof( wxString );
	m_cDirectionArrowSize = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cDirectionArrowSizeNChoices, m_cDirectionArrowSizeChoices, 0 );
	m_cDirectionArrowSize->SetSelection( 0 );
	fgSizer15->Add( m_cDirectionArrowSize, 0, wxALL|wxEXPAND, 5 );
	
	m_cbOverlayMap = new wxCheckBox( this, wxID_ANY, _("OverlayMap"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer15->Add( m_cbOverlayMap, 0, wxALL, 5 );
	
	m_tOverlayColors = new wxStaticText( this, wxID_ANY, _("Colors"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tOverlayColors->Wrap( -1 );
	fgSizer15->Add( m_tOverlayColors, 0, wxALL|wxEXPAND, 5 );
	
	wxString m_cOverlayColorsChoices[] = { _("Generic"), _("Wind"), _("Air Temp"), _("Sea Temp"), _("Rainfall"), _("Cloud Cover"), _("Current") };
	int m_cOverlayColorsNChoices = sizeof( m_cOverlayColorsChoices ) / sizeof( wxString );
	m_cOverlayColors = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cOverlayColorsNChoices, m_cOverlayColorsChoices, 0 );
	m_cOverlayColors->SetSelection( 0 );
	fgSizer15->Add( m_cOverlayColors, 0, wxALL|wxEXPAND, 5 );
	
	m_cbNumbers = new wxCheckBox( this, wxID_ANY, _("Numbers"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer15->Add( m_cbNumbers, 0, wxALL, 5 );
	
	m_ctNumbers = new wxStaticText( this, wxID_ANY, _("Minimum Spacing"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ctNumbers->Wrap( -1 );
	fgSizer15->Add( m_ctNumbers, 0, wxALL|wxEXPAND, 5 );
	
	m_sNumbersSpacing = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 90,-1 ), wxSP_ARROW_KEYS, 30, 100, 50 );
	fgSizer15->Add( m_sNumbersSpacing, 0, wxALL, 5 );
	
	m_cbParticles = new wxCheckBox( this, wxID_ANY, _("Particle Map"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer15->Add( m_cbParticles, 0, wxALL, 5 );
	
	m_ctParticles = new wxStaticText( this, wxID_ANY, _("Density"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ctParticles->Wrap( -1 );
	fgSizer15->Add( m_ctParticles, 0, wxALL, 5 );
	
	m_sParticleDensity = new wxSlider( this, wxID_ANY, 5, 1, 10, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	fgSizer15->Add( m_sParticleDensity, 0, wxALL|wxEXPAND, 5 );
	
	
	sbSizer5->Add( fgSizer15, 1, wxALL|wxEXPAND, 5 );
	
	
	fgSizer4->Add( sbSizer5, 1, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer7;
	sbSizer7 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Display") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer34;
	fgSizer34 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer34->SetFlexibleDirection( wxBOTH );
	fgSizer34->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_staticText24 = new wxStaticText( this, wxID_ANY, _("Overlay Transparency (%)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText24->Wrap( -1 );
	fgSizer34->Add( m_staticText24, 0, wxALL, 5 );
	
	m_sTransparency = new wxSlider( this, wxID_ANY, 50, 0, 100, wxDefaultPosition, wxSize( 200,-1 ), wxSL_BOTTOM|wxSL_HORIZONTAL|wxSL_LABELS );
	fgSizer34->Add( m_sTransparency, 0, wxALL|wxEXPAND, 5 );
	
	
	sbSizer7->Add( fgSizer34, 1, wxEXPAND, 5 );
	
	
	fgSizer4->Add( sbSizer7, 1, wxEXPAND, 5 );
	
	m_sButton = new wxStdDialogButtonSizer();
	m_sButtonOK = new wxButton( this, wxID_OK );
	m_sButton->AddButton( m_sButtonOK );
	m_sButtonApply = new wxButton( this, wxID_APPLY );
	m_sButton->AddButton( m_sButtonApply );
	m_sButtonCancel = new wxButton( this, wxID_CANCEL );
	m_sButton->AddButton( m_sButtonCancel );
	m_sButton->Realize();
	
	fgSizer4->Add( m_sButton, 1, wxALIGN_RIGHT|wxSHAPED, 5 );
	
	
	this->SetSizer( fgSizer4 );
	this->Layout();
	fgSizer4->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_cInterpolate->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnIntepolateChange ), NULL, this );
	m_cDataType->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribSettingsDialogBase::OnDataTypeChoice ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sButtonApply->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnApply ), NULL, this );
}

GribSettingsDialogBase::~GribSettingsDialogBase()
{
	// Disconnect Events
	m_cInterpolate->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnIntepolateChange ), NULL, this );
	m_cDataType->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribSettingsDialogBase::OnDataTypeChoice ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sButtonApply->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnApply ), NULL, this );
	
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
	
	wxString m_rbLoadOptionsChoices[] = { _("Load the More Recent File in Directory"), _("Load the Last Opened File") };
	int m_rbLoadOptionsNChoices = sizeof( m_rbLoadOptionsChoices ) / sizeof( wxString );
	m_rbLoadOptions = new wxRadioBox( this, wxID_ANY, _("Load File Options"), wxDefaultPosition, wxDefaultSize, m_rbLoadOptionsNChoices, m_rbLoadOptionsChoices, 1, wxRA_SPECIFY_COLS );
	m_rbLoadOptions->SetSelection( 0 );
	fgSizer6->Add( m_rbLoadOptions, 0, wxALL|wxEXPAND, 5 );
	
	wxString m_rbStartOptionsChoices[] = { _("Start at the first forecast in GRIB file"), _("Start at the nearest forecast to current time"), _("Authorize Interpolation to start at current time") };
	int m_rbStartOptionsNChoices = sizeof( m_rbStartOptionsChoices ) / sizeof( wxString );
	m_rbStartOptions = new wxRadioBox( this, wxID_ANY, _("Start Options"), wxDefaultPosition, wxDefaultSize, m_rbStartOptionsNChoices, m_rbStartOptionsChoices, 1, wxRA_SPECIFY_COLS );
	m_rbStartOptions->SetSelection( 0 );
	fgSizer6->Add( m_rbStartOptions, 0, wxALL|wxEXPAND, 5 );
	
	wxString m_rbTimeFormatChoices[] = { _("Local Time"), _("UTC") };
	int m_rbTimeFormatNChoices = sizeof( m_rbTimeFormatChoices ) / sizeof( wxString );
	m_rbTimeFormat = new wxRadioBox( this, wxID_ANY, _("Time Options"), wxDefaultPosition, wxDefaultSize, m_rbTimeFormatNChoices, m_rbTimeFormatChoices, 1, wxRA_SPECIFY_COLS );
	m_rbTimeFormat->SetSelection( 0 );
	fgSizer6->Add( m_rbTimeFormat, 0, wxALL|wxEXPAND, 5 );
	
	
	fgSizer8->Add( fgSizer6, 1, wxEXPAND, 5 );
	
	wxStdDialogButtonSizer* m_sdbSizer2;
	wxButton* m_sdbSizer2OK;
	wxButton* m_sdbSizer2Cancel;
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
	
	// Connect Events
	m_rbStartOptions->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( GribPreferencesDialogBase::OnStartOptionChange ), NULL, this );
}

GribPreferencesDialogBase::~GribPreferencesDialogBase()
{
	// Disconnect Events
	m_rbStartOptions->Disconnect( wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler( GribPreferencesDialogBase::OnStartOptionChange ), NULL, this );
	
}

GribRequestSettingBase::GribRequestSettingBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxFlexGridSizer* fgSizer101;
	fgSizer101 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer101->SetFlexibleDirection( wxBOTH );
	fgSizer101->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_sScrolledDialog = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL );
	m_sScrolledDialog->SetScrollRate( 5, 5 );
	wxBoxSizer* bSizer3;
	bSizer3 = new wxBoxSizer( wxVERTICAL );

	m_fgScrollSizer = new wxFlexGridSizer( 0, 1, 0, 0 );
	m_fgScrollSizer->SetFlexibleDirection( wxBOTH );
	m_fgScrollSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticBoxSizer* sbSizer7;
	sbSizer7 = new wxStaticBoxSizer( new wxStaticBox( m_sScrolledDialog, wxID_ANY, _("Profile") ), wxVERTICAL );

	wxFlexGridSizer* fgSizer17;
	fgSizer17 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer17->SetFlexibleDirection( wxBOTH );
	fgSizer17->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_pSenderSizer = new wxFlexGridSizer( 0, 2, 0, 0 );
	m_pSenderSizer->AddGrowableCol( 1 );
	m_pSenderSizer->SetFlexibleDirection( wxBOTH );
	m_pSenderSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText* m_staticText25;
	m_staticText25 = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("Sender eMail Address"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText25->Wrap( -1 );
	m_pSenderSizer->Add( m_staticText25, 0, wxALL, 5 );

	m_pSenderAddress = new wxTextCtrl( m_sScrolledDialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_pSenderSizer->Add( m_pSenderAddress, 0, wxALL|wxEXPAND, 5 );

	wxStaticLine* m_staticline4;
	m_staticline4 = new wxStaticLine( m_sScrolledDialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	m_pSenderSizer->Add( m_staticline4, 0, wxEXPAND | wxALL, 5 );

	wxStaticLine* m_staticline5;
	m_staticline5 = new wxStaticLine( m_sScrolledDialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	m_pSenderSizer->Add( m_staticline5, 0, wxEXPAND | wxALL, 5 );


	fgSizer17->Add( m_pSenderSizer, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer9;
	fgSizer9 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer9->SetFlexibleDirection( wxBOTH );
	fgSizer9->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSizer33;
	fgSizer33 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer33->SetFlexibleDirection( wxBOTH );
	fgSizer33->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText* m_staticText15;
	m_staticText15 = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("Mail To "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText15->Wrap( -1 );
	fgSizer33->Add( m_staticText15, 0, wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 5 );

	wxArrayString m_pMailToChoices;
	m_pMailTo = new wxChoice( m_sScrolledDialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_pMailToChoices, 0 );
	m_pMailTo->SetSelection( 0 );
	fgSizer33->Add( m_pMailTo, 0, wxALL, 5 );

	wxStaticText* m_staticText16;
	m_staticText16 = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("Forecast Model "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText16->Wrap( -1 );
	fgSizer33->Add( m_staticText16, 0, wxALL|wxEXPAND|wxLEFT|wxRIGHT|wxTOP, 5 );

	wxArrayString m_pModelChoices;
	m_pModel = new wxChoice( m_sScrolledDialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_pModelChoices, 0 );
	m_pModel->SetSelection( 0 );
	fgSizer33->Add( m_pModel, 0, wxALL, 5 );

	
	fgSizer9->Add( fgSizer33, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer32;
	fgSizer32 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer32->SetFlexibleDirection( wxBOTH );
	fgSizer32->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_cMovingGribEnabled = new wxCheckBox( m_sScrolledDialog, wxID_ANY, _("Moving Grib"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer32->Add( m_cMovingGribEnabled, 0, wxALL, 5 );

	m_fgMovingParams = new wxFlexGridSizer( 0, 3, 0, 0 );
	m_fgMovingParams->SetFlexibleDirection( wxBOTH );
	m_fgMovingParams->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText* m_staticText27;
	m_staticText27 = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("Speed"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText27->Wrap( -1 );
	m_fgMovingParams->Add( m_staticText27, 0, wxALL, 5 );

	m_sMovingSpeed = new wxSpinCtrl( m_sScrolledDialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 1, 30, 1 );
	m_fgMovingParams->Add( m_sMovingSpeed, 0, wxALL, 5 );

	wxStaticText* m_sSpeedUnit;
	m_sSpeedUnit = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("Kts"), wxDefaultPosition, wxDefaultSize, 0 );
	m_sSpeedUnit->Wrap( -1 );
	m_fgMovingParams->Add( m_sSpeedUnit, 0, wxBOTTOM|wxEXPAND|wxRIGHT|wxTOP, 5 );

	wxStaticText* m_staticText29;
	m_staticText29 = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("Course"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText29->Wrap( -1 );
	m_fgMovingParams->Add( m_staticText29, 0, wxALL, 5 );

	m_sMovingCourse = new wxSpinCtrl( m_sScrolledDialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxSP_ARROW_KEYS, 1, 360, 1 );
	m_fgMovingParams->Add( m_sMovingCourse, 0, wxALL, 5 );

	m_sCourseUnit = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("Deg"), wxDefaultPosition, wxDefaultSize, 0 );
	m_sCourseUnit->Wrap( -1 );
	m_fgMovingParams->Add( m_sCourseUnit, 0, wxBOTTOM|wxEXPAND|wxRIGHT|wxTOP, 5 );

	
	fgSizer32->Add( m_fgMovingParams, 1, wxEXPAND, 5 );
	
	
	fgSizer9->Add( fgSizer32, 1, wxEXPAND, 5 );
	
	m_fgLog = new wxFlexGridSizer( 0, 2, 0, 0 );
	m_fgLog->SetFlexibleDirection( wxBOTH );
	m_fgLog->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText* m_tLogin;
	m_tLogin = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("zyGrib Login"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tLogin->Wrap( -1 );
	m_fgLog->Add( m_tLogin, 0, wxALL, 5 );

	m_pLogin = new wxTextCtrl( m_sScrolledDialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_fgLog->Add( m_pLogin, 0, wxALL, 5 );

	wxStaticText* m_tCode;
	m_tCode = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("zyGrib Code"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tCode->Wrap( -1 );
	m_fgLog->Add( m_tCode, 0, wxALL, 5 );

	m_pCode = new wxTextCtrl( m_sScrolledDialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_fgLog->Add( m_pCode, 0, wxALL, 5 );


	fgSizer9->Add( m_fgLog, 1, wxEXPAND, 5 );
	
	
	fgSizer17->Add( fgSizer9, 1, wxEXPAND, 10 );
	
	wxFlexGridSizer* fgSizer13;
	fgSizer13 = new wxFlexGridSizer( 0, 9, 0, 0 );
	fgSizer13->SetFlexibleDirection( wxBOTH );
	fgSizer13->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText* m_staticText17;
	m_staticText17 = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("Resolution"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText17->Wrap( -1 );
	fgSizer13->Add( m_staticText17, 0, wxALL, 5 );

	wxArrayString m_pResolutionChoices;
	m_pResolution = new wxChoice( m_sScrolledDialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_pResolutionChoices, 0 );
	m_pResolution->SetSelection( 0 );
	fgSizer13->Add( m_pResolution, 0, wxALL, 1 );

	m_tResUnit = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("Deg"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tResUnit->Wrap( -1 );
	fgSizer13->Add( m_tResUnit, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText* m_staticText18;
	m_staticText18 = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("Interval"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText18->Wrap( -1 );
	fgSizer13->Add( m_staticText18, 0, wxALL, 5 );

	wxArrayString m_pIntervalChoices;
	m_pInterval = new wxChoice( m_sScrolledDialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_pIntervalChoices, 0 );
	m_pInterval->SetSelection( 0 );
	fgSizer13->Add( m_pInterval, 0, wxALL, 1 );

	wxStaticText* m_staticText20;
	m_staticText20 = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("Hours"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText20->Wrap( -1 );
	fgSizer13->Add( m_staticText20, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

	wxStaticText* m_staticText19;
	m_staticText19 = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("Time Range"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText19->Wrap( -1 );
	fgSizer13->Add( m_staticText19, 0, wxALL, 5 );

	wxArrayString m_pTimeRangeChoices;
	m_pTimeRange = new wxChoice( m_sScrolledDialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_pTimeRangeChoices, 0 );
	m_pTimeRange->SetSelection( 0 );
	fgSizer13->Add( m_pTimeRange, 0, wxALL, 1 );

	m_staticText21 = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("Days"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText21->Wrap( -1 );
	fgSizer13->Add( m_staticText21, 0, wxTOP|wxBOTTOM|wxRIGHT, 5 );

	
	fgSizer17->Add( fgSizer13, 1, wxEXPAND, 5 );
	
	
	sbSizer7->Add( fgSizer17, 1, wxEXPAND, 5 );


	m_fgScrollSizer->Add( sbSizer7, 1, wxEXPAND, 5 );

	wxStaticBoxSizer* sbSizer81;
	sbSizer81 = new wxStaticBoxSizer( new wxStaticBox( m_sScrolledDialog, wxID_ANY, _("Area Selection") ), wxVERTICAL );

	wxFlexGridSizer* fgSizer36;
	fgSizer36 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer36->SetFlexibleDirection( wxBOTH );
	fgSizer36->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxFlexGridSizer* fgSizer34;
	fgSizer34 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer34->SetFlexibleDirection( wxBOTH );
	fgSizer34->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_cManualZoneSel = new wxCheckBox( m_sScrolledDialog, wxID_ANY, _("Manual Selection"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cManualZoneSel->SetValue(true);
	fgSizer34->Add( m_cManualZoneSel, 0, wxLEFT, 5 );

	m_toggleSelection = new wxToggleButton( m_sScrolledDialog, wxID_ANY, _("Start graphic Sel."), wxDefaultPosition, wxDefaultSize, 0 );
	m_toggleSelection->SetValue( true );
	fgSizer34->Add( m_toggleSelection, 0, wxLEFT, 5 );


	fgSizer36->Add( fgSizer34, 1, wxEXPAND, 5 );

	fgZoneCoordinatesSizer = new wxFlexGridSizer( 0, 6, 0, 0 );
	fgZoneCoordinatesSizer->SetFlexibleDirection( wxBOTH );
	fgZoneCoordinatesSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText* m_staticText34;
	m_staticText34 = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("Max Lat"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText34->Wrap( -1 );
	fgZoneCoordinatesSizer->Add( m_staticText34, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );

	m_spMaxLat = new wxSpinCtrl( m_sScrolledDialog, MAXLAT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -180, 180, 0 );
	fgZoneCoordinatesSizer->Add( m_spMaxLat, 0, wxLEFT|wxRIGHT, 5 );

	m_stMaxLatNS = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("N"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stMaxLatNS->Wrap( -1 );
	fgZoneCoordinatesSizer->Add( m_stMaxLatNS, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 20 );

	m_staticText36 = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("Max Long"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText36->Wrap( -1 );
	fgZoneCoordinatesSizer->Add( m_staticText36, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );

	m_spMaxLon = new wxSpinCtrl( m_sScrolledDialog, MAXLON, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -180, 180, 0 );
	fgZoneCoordinatesSizer->Add( m_spMaxLon, 0, wxLEFT|wxRIGHT, 5 );

	m_stMaxLonEW = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("E"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stMaxLonEW->Wrap( -1 );
	fgZoneCoordinatesSizer->Add( m_stMaxLonEW, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );

	wxStaticText* m_staticText38;
	m_staticText38 = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("Min Lat"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText38->Wrap( -1 );
	fgZoneCoordinatesSizer->Add( m_staticText38, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );

	m_spMinLat = new wxSpinCtrl( m_sScrolledDialog, MINLAT, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -180, 180, 0 );
	fgZoneCoordinatesSizer->Add( m_spMinLat, 0, wxLEFT|wxRIGHT, 5 );

	m_stMinLatNS = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("S"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stMinLatNS->Wrap( -1 );
	fgZoneCoordinatesSizer->Add( m_stMinLatNS, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 20 );

	wxStaticText* m_staticText40;
	m_staticText40 = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("Min Long"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText40->Wrap( -1 );
	fgZoneCoordinatesSizer->Add( m_staticText40, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5 );

	m_spMinLon = new wxSpinCtrl( m_sScrolledDialog, MINLON, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -180, 180, 0 );
	fgZoneCoordinatesSizer->Add( m_spMinLon, 0, wxLEFT|wxRIGHT, 5 );

	m_stMinLonEW = new wxStaticText( m_sScrolledDialog, wxID_ANY, _("W"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stMinLonEW->Wrap( -1 );
	fgZoneCoordinatesSizer->Add( m_stMinLonEW, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0 );


	fgSizer36->Add( fgZoneCoordinatesSizer, 1, wxEXPAND|wxLEFT, 20 );


	sbSizer81->Add( fgSizer36, 1, wxBOTTOM|wxEXPAND|wxTOP, 5 );


	m_fgScrollSizer->Add( sbSizer81, 1, wxEXPAND, 5 );

	wxStaticBoxSizer* sbSizer8;
	sbSizer8 = new wxStaticBoxSizer( new wxStaticBox( m_sScrolledDialog, wxID_ANY, _("Data Selection") ), wxVERTICAL );

	wxFlexGridSizer* fgSizer10;
	fgSizer10 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer10->SetFlexibleDirection( wxBOTH );
	fgSizer10->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_pWind = new wxCheckBox( m_sScrolledDialog, wxID_ANY, _("Wind"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer10->Add( m_pWind, 0, wxALL|wxEXPAND, 5 );

	m_pPress = new wxCheckBox( m_sScrolledDialog, wxID_ANY, _("Pressure"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer10->Add( m_pPress, 0, wxALL|wxEXPAND, 5 );

	m_pWindGust = new wxCheckBox( m_sScrolledDialog, wxID_ANY, _("Wind Gust"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer10->Add( m_pWindGust, 0, wxALL|wxEXPAND, 5 );

	m_pRainfall = new wxCheckBox( m_sScrolledDialog, wxID_ANY, _("Rainfall"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer10->Add( m_pRainfall, 0, wxALL|wxEXPAND, 5 );

	m_pCloudCover = new wxCheckBox( m_sScrolledDialog, wxID_ANY, _("Cloud Cover"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer10->Add( m_pCloudCover, 0, wxALL|wxEXPAND, 5 );

	m_pAirTemp = new wxCheckBox( m_sScrolledDialog, wxID_ANY, _("Air Temperature(2m)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pAirTemp->SetValue(true);
	fgSizer10->Add( m_pAirTemp, 0, wxALL|wxEXPAND, 5 );

	m_pSeaTemp = new wxCheckBox( m_sScrolledDialog, wxID_ANY, _("Sea Temperature(surf.)"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer10->Add( m_pSeaTemp, 0, wxALL|wxEXPAND, 5 );

	m_pCurrent = new wxCheckBox( m_sScrolledDialog, wxID_ANY, _("Current"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer10->Add( m_pCurrent, 0, wxALL|wxEXPAND, 5 );

	m_pCAPE = new wxCheckBox( m_sScrolledDialog, wxID_ANY, _("CAPE"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer10->Add( m_pCAPE, 0, wxALL|wxEXPAND, 5 );

	wxFlexGridSizer* fgSizer28;
	fgSizer28 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer28->SetFlexibleDirection( wxBOTH );
	fgSizer28->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_pWaves = new wxCheckBox( m_sScrolledDialog, wxID_ANY, _("Waves"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer28->Add( m_pWaves, 0, wxALL, 5 );

	wxArrayString m_pWModelChoices;
	m_pWModel = new wxChoice( m_sScrolledDialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_pWModelChoices, 0 );
	m_pWModel->SetSelection( 0 );
	fgSizer28->Add( m_pWModel, 0, wxALL, 1 );

	
	fgSizer10->Add( fgSizer28, 1, wxEXPAND, 5 );
	
	wxFlexGridSizer* fgSizer331;
	fgSizer331 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer331->SetFlexibleDirection( wxBOTH );
	fgSizer331->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_pAltitudeData = new wxCheckBox( m_sScrolledDialog, wxID_ANY, _("Geopotential Height"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer331->Add( m_pAltitudeData, 0, wxALL|wxEXPAND, 5 );


	fgSizer10->Add( fgSizer331, 1, wxEXPAND, 5 );
	
	m_fgAltitudeData = new wxFlexGridSizer( 0, 2, 0, 0 );
	m_fgAltitudeData->SetFlexibleDirection( wxBOTH );
	m_fgAltitudeData->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_p850hpa = new wxCheckBox( m_sScrolledDialog, wxID_ANY, _("850 hPa"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgAltitudeData->Add( m_p850hpa, 0, wxALL|wxEXPAND, 5 );

	m_p700hpa = new wxCheckBox( m_sScrolledDialog, wxID_ANY, _("700 hPa"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgAltitudeData->Add( m_p700hpa, 0, wxALL|wxEXPAND, 5 );

	m_p500hpa = new wxCheckBox( m_sScrolledDialog, wxID_ANY, _("500 hPa"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgAltitudeData->Add( m_p500hpa, 0, wxALL|wxEXPAND, 5 );

	m_p300hpa = new wxCheckBox( m_sScrolledDialog, wxID_ANY, _("300 hPa"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgAltitudeData->Add( m_p300hpa, 0, wxALL|wxEXPAND, 5 );


	fgSizer10->Add( m_fgAltitudeData, 1, wxEXPAND, 5 );
	
	
	sbSizer8->Add( fgSizer10, 1, wxEXPAND, 5 );


	m_fgScrollSizer->Add( sbSizer8, 1, wxEXPAND|wxTOP, 5 );

	wxStaticBoxSizer* sbSizer6;
	sbSizer6 = new wxStaticBoxSizer( new wxStaticBox( m_sScrolledDialog, wxID_ANY, _("Mail") ), wxVERTICAL );

	wxFlexGridSizer* fgSizer11;
	fgSizer11 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer11->AddGrowableCol( 0 );
	fgSizer11->SetFlexibleDirection( wxBOTH );
	fgSizer11->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_MailImage = new wxTextCtrl( m_sScrolledDialog, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY );
	fgSizer11->Add( m_MailImage, 0, wxALL|wxEXPAND, 5 );


	sbSizer6->Add( fgSizer11, 1, wxEXPAND, 5 );


	m_fgScrollSizer->Add( sbSizer6, 1, wxEXPAND, 5 );


	bSizer3->Add( m_fgScrollSizer, 1, 0, 5 );


	m_sScrolledDialog->SetSizer( bSizer3 );
	m_sScrolledDialog->Layout();
	bSizer3->Fit( m_sScrolledDialog );
	fgSizer101->Add( m_sScrolledDialog, 1, 0, 5 );

	m_fgFixedSizer = new wxFlexGridSizer( 0, 3, 0, 0 );
	m_fgFixedSizer->SetFlexibleDirection( wxBOTH );
	m_fgFixedSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText* m_staticText181;
	m_staticText181 = new wxStaticText( this, wxID_ANY, _("Estimated File Size"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText181->Wrap( -1 );
	m_fgFixedSizer->Add( m_staticText181, 0, wxALL, 5 );

	m_tFileSize = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_tFileSize->Wrap( -1 );
	m_fgFixedSizer->Add( m_tFileSize, 0, wxTOP|wxBOTTOM, 5 );

	m_tLimit = new wxStaticText( this, wxID_ANY, _(" (Max size )"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tLimit->Wrap( -1 );
	m_fgFixedSizer->Add( m_tLimit, 0, wxALL, 5 );


	fgSizer101->Add( m_fgFixedSizer, 1, wxEXPAND, 5 );

	m_rButton = new wxStdDialogButtonSizer();
	m_rButtonYes = new wxButton( this, wxID_YES );
	m_rButton->AddButton( m_rButtonYes );
	m_rButtonApply = new wxButton( this, wxID_APPLY );
	m_rButton->AddButton( m_rButtonApply );
	m_rButtonCancel = new wxButton( this, wxID_CANCEL );
	m_rButton->AddButton( m_rButtonCancel );
	m_rButton->Realize();
	
	fgSizer101->Add( m_rButton, 1, wxALIGN_RIGHT|wxSHAPED, 5 );
	
	
	this->SetSizer( fgSizer101 );
	this->Layout();
	fgSizer101->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GribRequestSettingBase::OnClose ) );
	m_pMailTo->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribRequestSettingBase::OnTopChange ), NULL, this );
	m_pModel->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribRequestSettingBase::OnTopChange ), NULL, this );
	m_cMovingGribEnabled->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnMovingClick ), NULL, this );
	m_sMovingSpeed->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_sMovingCourse->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pLogin->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pCode->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pResolution->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pInterval->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pTimeRange->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribRequestSettingBase::OnTimeRangeChange ), NULL, this );
	m_cManualZoneSel->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnZoneSelectionModeChange ), NULL, this );
	m_toggleSelection->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnTooggleSelection ), NULL, this );
	m_spMaxLat->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( GribRequestSettingBase::OnCoordinatesChange ), NULL, this );
	m_spMaxLon->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( GribRequestSettingBase::OnCoordinatesChange ), NULL, this );
	m_spMinLat->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( GribRequestSettingBase::OnCoordinatesChange ), NULL, this );
	m_spMinLon->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( GribRequestSettingBase::OnCoordinatesChange ), NULL, this );
	m_pWind->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pPress->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pWindGust->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pRainfall->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pCloudCover->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pAirTemp->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pSeaTemp->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pCurrent->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pCAPE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pWaves->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pWModel->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pAltitudeData->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_p850hpa->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_p700hpa->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_p500hpa->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_p300hpa->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_rButtonApply->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnSaveMail ), NULL, this );
	m_rButtonCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnCancel ), NULL, this );
	m_rButtonYes->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnSendMaiL ), NULL, this );
}

GribRequestSettingBase::~GribRequestSettingBase()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GribRequestSettingBase::OnClose ) );
	m_pMailTo->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribRequestSettingBase::OnTopChange ), NULL, this );
	m_pModel->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribRequestSettingBase::OnTopChange ), NULL, this );
	m_cMovingGribEnabled->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnMovingClick ), NULL, this );
	m_sMovingSpeed->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_sMovingCourse->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pLogin->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pCode->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pResolution->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pInterval->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pTimeRange->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribRequestSettingBase::OnTimeRangeChange ), NULL, this );
	m_cManualZoneSel->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnZoneSelectionModeChange ), NULL, this );
	m_toggleSelection->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnTooggleSelection ), NULL, this );
	m_spMaxLat->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( GribRequestSettingBase::OnCoordinatesChange ), NULL, this );
	m_spMaxLon->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( GribRequestSettingBase::OnCoordinatesChange ), NULL, this );
	m_spMinLat->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( GribRequestSettingBase::OnCoordinatesChange ), NULL, this );
	m_spMinLon->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( GribRequestSettingBase::OnCoordinatesChange ), NULL, this );
	m_pWind->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pPress->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pWindGust->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pRainfall->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pCloudCover->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pAirTemp->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pSeaTemp->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pCurrent->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pCAPE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pWaves->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pWModel->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pAltitudeData->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_p850hpa->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_p700hpa->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_p500hpa->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_p300hpa->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_rButtonApply->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnSaveMail ), NULL, this );
	m_rButtonCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnCancel ), NULL, this );
	m_rButtonYes->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnSendMaiL ), NULL, this );
	
}

GRIBTableBase::GRIBTableBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxSize( -1,-1 ) );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	wxFlexGridSizer* fgSizer18;
	fgSizer18 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer18->AddGrowableRow( 1 );
	fgSizer18->SetFlexibleDirection( wxBOTH );
	fgSizer18->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_NONE );
	
	wxFlexGridSizer* fgSizer19;
	fgSizer19 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer19->SetFlexibleDirection( wxBOTH );
	fgSizer19->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_NONE );
	
	m_pPositionText = new wxStaticText( this, wxID_ANY, _("Data at Cursor Position:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pPositionText->Wrap( -1 );
	fgSizer19->Add( m_pPositionText, 0, wxALL|wxEXPAND, 5 );
	
	m_pCursorPosition = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_pCursorPosition->Wrap( -1 );
	fgSizer19->Add( m_pCursorPosition, 0, wxALL|wxEXPAND, 5 );
	
	
	fgSizer18->Add( fgSizer19, 1, wxEXPAND, 10 );
	
	wxFlexGridSizer* fgSizer20;
	fgSizer20 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer20->AddGrowableRow( 0 );
	fgSizer20->SetFlexibleDirection( wxBOTH );
	fgSizer20->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	m_pGribTable = new wxGrid( this, wxID_ANY, wxDefaultPosition, wxSize( -1,50 ), 0 );
	
	// Grid
	m_pGribTable->CreateGrid( 2, 0 );
	m_pGribTable->EnableEditing( false );
	m_pGribTable->EnableGridLines( true );
	m_pGribTable->EnableDragGridSize( false );
	m_pGribTable->SetMargins( 0, 0 );
	
	// Columns
	m_pGribTable->EnableDragColMove( false );
	m_pGribTable->EnableDragColSize( false );
	m_pGribTable->SetColLabelSize( 0 );
	m_pGribTable->SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Rows
	m_pGribTable->EnableDragRowSize( false );
	m_pGribTable->SetRowLabelSize( 120 );
	m_pGribTable->SetRowLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );
	
	// Label Appearance
	
	// Cell Defaults
	m_pGribTable->SetDefaultCellFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 90, false, wxEmptyString ) );
	m_pGribTable->SetDefaultCellAlignment( wxALIGN_CENTRE, wxALIGN_TOP );
	fgSizer20->Add( m_pGribTable, 0, wxALL|wxEXPAND, 5 );
	
	
	fgSizer18->Add( fgSizer20, 1, wxEXPAND, 5 );
	
	m_pButtonTable = new wxStdDialogButtonSizer();
	m_pButtonTableOK = new wxButton( this, wxID_OK );
	m_pButtonTable->AddButton( m_pButtonTableOK );
	m_pButtonTable->Realize();
	
	fgSizer18->Add( m_pButtonTable, 1, wxEXPAND, 5 );
	
	
	bSizer1->Add( fgSizer18, 1, wxEXPAND, 5 );
	
	
	this->SetSizer( bSizer1 );
	this->Layout();
	bSizer1->Fit( this );
	
	this->Centre( wxBOTH );
	
	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GRIBTableBase::OnClose ) );
	m_pGribTable->Connect( wxEVT_GRID_CELL_LEFT_CLICK, wxGridEventHandler( GRIBTableBase::OnClick ), NULL, this );
	m_pGribTable->Connect( wxEVT_GRID_LABEL_LEFT_CLICK, wxGridEventHandler( GRIBTableBase::OnClick ), NULL, this );
	m_pGribTable->Connect( wxEVT_GRID_RANGE_SELECT, wxGridRangeSelectEventHandler( GRIBTableBase::OnRangeClick ), NULL, this );
	m_pButtonTableOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBTableBase::OnOKButton ), NULL, this );
}

GRIBTableBase::~GRIBTableBase()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GRIBTableBase::OnClose ) );
	m_pGribTable->Disconnect( wxEVT_GRID_CELL_LEFT_CLICK, wxGridEventHandler( GRIBTableBase::OnClick ), NULL, this );
	m_pGribTable->Disconnect( wxEVT_GRID_LABEL_LEFT_CLICK, wxGridEventHandler( GRIBTableBase::OnClick ), NULL, this );
	m_pGribTable->Disconnect( wxEVT_GRID_RANGE_SELECT, wxGridRangeSelectEventHandler( GRIBTableBase::OnRangeClick ), NULL, this );
	m_pButtonTableOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBTableBase::OnOKButton ), NULL, this );
	
}
