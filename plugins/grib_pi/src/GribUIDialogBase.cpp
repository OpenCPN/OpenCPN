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
	fgSizer1 = new wxFlexGridSizer( 4, 1, 0, 0 );
	fgSizer1->AddGrowableRow( 2 );
	fgSizer1->SetFlexibleDirection( wxVERTICAL );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	
	wxStaticBoxSizer* sbSizer1;
	sbSizer1 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("GRIB File Directory") ), wxHORIZONTAL );
	
	m_dirPicker = new wxDirPickerCtrl( this, ID_GRIBDIR, wxEmptyString, wxT("Select a GRIB Directory"), wxDefaultPosition, wxDefaultSize, wxDIRP_DEFAULT_STYLE );
	sbSizer1->Add( m_dirPicker, 0, wxALL, 5 );
	
	m_bConfig = new wxButton( this, ID_CONFIG, wxT("Config"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizer1->Add( m_bConfig, 0, wxALL, 5 );
	
	
	fgSizer1->Add( sbSizer1, 1, wxEXPAND|wxFIXED_MINSIZE, 5 );
	
	wxFlexGridSizer* fgSizer5;
	fgSizer5 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer5->AddGrowableCol( 1 );
	fgSizer5->SetFlexibleDirection( wxBOTH );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_tbPlayStop = new wxToggleButton( this, ID_PLAYSTOP, wxT("Play"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer5->Add( m_tbPlayStop, 0, wxALL, 5 );
	
	m_sTimeline = new wxSlider( this, ID_TIMELINE, 2, 1, 10, wxDefaultPosition, wxSize( -1,-1 ), wxSL_HORIZONTAL );
	fgSizer5->Add( m_sTimeline, 0, wxEXPAND, 5 );
	
	
	fgSizer1->Add( fgSizer5, 1, wxEXPAND, 5 );
	
	m_stDateTime = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_stDateTime->Wrap( -1 );
	fgSizer1->Add( m_stDateTime, 0, wxALL|wxEXPAND, 5 );
	
	m_pRecordTree = new GribRecordTree(this, ID_GRIBRECORDTREE);
	m_pRecordTree->SetMinSize( wxSize( -1,120 ) );
	
	fgSizer1->Add( m_pRecordTree, 0, wxALL|wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("GRIB Data at Cursor") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	m_cbWind = new wxCheckBox( this, ID_CB_WIND_SPEED, wxT("Wind"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cbWind->SetValue(true); 
	fgSizer4->Add( m_cbWind, 0, wxALL, 5 );
	
	m_tcWindSpeed = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_tcWindSpeed->SetMaxLength( 3 ); 
	fgSizer4->Add( m_tcWindSpeed, 0, wxALL, 5 );
	
	m_tcWindDirection = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_tcWindDirection->SetMaxLength( 3 ); 
	fgSizer4->Add( m_tcWindDirection, 0, wxALL, 5 );
	
	m_cbWave = new wxCheckBox( this, ID_CB_SIG_WAVE_HEIGHT, wxT("Wave"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer4->Add( m_cbWave, 0, wxALL, 5 );
	
	m_tcWaveHeight = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_tcWaveHeight->SetMaxLength( 5 ); 
	fgSizer4->Add( m_tcWaveHeight, 0, wxALL, 5 );
	
	m_tcWaveDirection = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_tcWaveDirection->SetMaxLength( 3 ); 
	fgSizer4->Add( m_tcWaveDirection, 0, wxALL, 5 );
	
	m_cbCurrent = new wxCheckBox( this, ID_CB_CURRENT_VELOCITY, wxT("Current"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer4->Add( m_cbCurrent, 0, wxALL, 5 );
	
	m_tcCurrentVelocity = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_tcCurrentVelocity->SetMaxLength( 5 ); 
	fgSizer4->Add( m_tcCurrentVelocity, 0, wxALL, 5 );
	
	m_tcCurrentDirection = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_tcCurrentDirection->SetMaxLength( 5 ); 
	fgSizer4->Add( m_tcCurrentDirection, 0, wxALL, 5 );
	
	m_cbPressure = new wxCheckBox( this, ID_CB_PRESSURE, wxT("Pressure"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cbPressure->SetValue(true); 
	fgSizer4->Add( m_cbPressure, 0, wxALL, 5 );
	
	m_tcPressure = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_tcPressure->SetMaxLength( 5 ); 
	fgSizer4->Add( m_tcPressure, 0, wxALL, 5 );
	
	
	fgSizer4->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_cbSeaTemperature = new wxCheckBox( this, ID_CB_SEA_TEMPERATURE, wxT("Sea Temperature"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer4->Add( m_cbSeaTemperature, 0, wxALL, 5 );
	
	m_tcSeaTemperature = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_tcSeaTemperature->SetMaxLength( 5 ); 
	fgSizer4->Add( m_tcSeaTemperature, 0, wxALL, 5 );
	
	
	sbSizer2->Add( fgSizer4, 1, wxEXPAND, 5 );
	
	
	fgSizer1->Add( sbSizer2, 1, wxFIXED_MINSIZE, 5 );
	
	
	this->SetSizer( fgSizer1 );
	this->Layout();
	
	this->Centre( wxBOTH );
}

GRIBUIDialogBase::~GRIBUIDialogBase()
{
}

GRIBConfigDialog::GRIBConfigDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxSize( -1,-1 ) );
	
	wxFlexGridSizer* fgSizer4;
	fgSizer4 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer4->SetFlexibleDirection( wxBOTH );
	fgSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("playback") ), wxVERTICAL );
	
	wxGridSizer* gSizer2;
	gSizer2 = new wxGridSizer( 0, 2, 0, 0 );
	
	m_cInterpolate = new wxCheckBox( this, wxID_ANY, wxT("Interpolate between gribs"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cInterpolate->SetValue(true); 
	gSizer2->Add( m_cInterpolate, 0, 0, 5 );
	
	m_cLoopMode = new wxCheckBox( this, wxID_ANY, wxT("Loop Mode"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer2->Add( m_cLoopMode, 0, 0, 5 );
	
	m_staticText5 = new wxStaticText( this, wxID_ANY, wxT("Playback Speed"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText5->Wrap( -1 );
	gSizer2->Add( m_staticText5, 0, 0, 5 );
	
	m_sPlaybackSpeed = new wxSlider( this, wxID_ANY, 50, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	gSizer2->Add( m_sPlaybackSpeed, 0, wxEXPAND, 5 );
	
	m_staticText4 = new wxStaticText( this, wxID_ANY, wxT("Slices per hour"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	gSizer2->Add( m_staticText4, 0, wxALL, 5 );
	
	m_sHourDivider = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 20, 2 );
	gSizer2->Add( m_sHourDivider, 0, wxALL, 5 );
	
	
	sbSizer4->Add( gSizer2, 1, 0, 5 );
	
	
	fgSizer4->Add( sbSizer4, 1, 0, 5 );
	
	wxStaticBoxSizer* sbSizer5;
	sbSizer5 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, wxT("Display") ), wxVERTICAL );
	
	wxGridSizer* gSizer3;
	gSizer3 = new wxGridSizer( 0, 3, 0, 0 );
	
	wxString m_cDataTypeChoices[] = { wxT("Wind"), wxT("Pressure"), wxT("Significant Wave"), wxT("Sea Surface Temp"), wxT("Current") };
	int m_cDataTypeNChoices = sizeof( m_cDataTypeChoices ) / sizeof( wxString );
	m_cDataType = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cDataTypeNChoices, m_cDataTypeChoices, 0 );
	m_cDataType->SetSelection( 0 );
	gSizer3->Add( m_cDataType, 0, wxALL, 5 );
	
	m_staticText12 = new wxStaticText( this, wxID_ANY, wxT("Units"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	gSizer3->Add( m_staticText12, 0, wxALL, 5 );
	
	wxArrayString m_cDataUnitsChoices;
	m_cDataUnits = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cDataUnitsChoices, 0 );
	m_cDataUnits->SetSelection( 0 );
	gSizer3->Add( m_cDataUnits, 0, wxALL, 5 );
	
	m_cbDirectionArrows = new wxCheckBox( this, wxID_ANY, wxT("Direction Arrows"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer3->Add( m_cbDirectionArrows, 0, wxALL, 5 );
	
	m_cbIsoBars = new wxCheckBox( this, wxID_ANY, wxT("Iso Bars"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer3->Add( m_cbIsoBars, 0, wxALL, 5 );
	
	m_cbOverlayMap = new wxCheckBox( this, wxID_ANY, wxT("OverlayMap"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer3->Add( m_cbOverlayMap, 0, wxALL, 5 );
	
	m_cbNumbers = new wxCheckBox( this, wxID_ANY, wxT("Numbers"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer3->Add( m_cbNumbers, 0, wxALL, 5 );
	
	
	sbSizer5->Add( gSizer3, 1, wxEXPAND, 5 );
	
	
	fgSizer4->Add( sbSizer5, 1, wxFIXED_MINSIZE, 5 );
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton( this, wxID_OK );
	m_sdbSizer1->AddButton( m_sdbSizer1OK );
	m_sdbSizer1Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer1->AddButton( m_sdbSizer1Cancel );
	m_sdbSizer1->Realize();
	
	fgSizer4->Add( m_sdbSizer1, 1, 0, 5 );
	
	
	this->SetSizer( fgSizer4 );
	this->Layout();
	fgSizer4->Fit( this );
	
	this->Centre( wxBOTH );
}

GRIBConfigDialog::~GRIBConfigDialog()
{
}
