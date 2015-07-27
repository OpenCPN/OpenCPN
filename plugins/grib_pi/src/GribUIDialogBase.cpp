///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun  5 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "GribUIDialogBase.h"

///////////////////////////////////////////////////////////////////////////

GRIBUICtrlBarBase::GRIBUICtrlBarBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	m_fgCtrlBarSizer = new wxFlexGridSizer( 1, 2, 0, 0 );
	m_fgCtrlBarSizer->AddGrowableCol( 0 );
	m_fgCtrlBarSizer->SetFlexibleDirection( wxHORIZONTAL );
	m_fgCtrlBarSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_NONE );

	wxFlexGridSizer* fgSizer49;
	fgSizer49 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer49->AddGrowableCol( 0 );
	fgSizer49->SetFlexibleDirection( wxVERTICAL );
	fgSizer49->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxFlexGridSizer* fgSizer50;
	fgSizer50 = new wxFlexGridSizer( 0, 15, 0, 0 );
	fgSizer50->AddGrowableCol( 3 );
	fgSizer50->AddGrowableCol( 7 );
	fgSizer50->AddGrowableCol( 10 );
	fgSizer50->SetFlexibleDirection( wxHORIZONTAL );
	fgSizer50->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_bpPrev = new wxBitmapButton( this, ID_BTNPREV, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpPrev->SetToolTip( _("Previous") );

	fgSizer50->Add( m_bpPrev, 0, wxALL, 1 );

	wxArrayString m_cRecordForecastChoices;
	m_cRecordForecast = new wxChoice( this, ID_CTRLTIME, wxDefaultPosition, wxDefaultSize, m_cRecordForecastChoices, 0 );
	m_cRecordForecast->SetSelection( 0 );
	fgSizer50->Add( m_cRecordForecast, 0, wxALIGN_CENTER_VERTICAL|wxALL, 1 );

	m_bpNext = new wxBitmapButton( this, ID_BTNNEXT, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpNext->SetToolTip( _("Next") );

	fgSizer50->Add( m_bpNext, 0, wxALL, 1 );


	fgSizer50->Add( 0, 0, 1, wxEXPAND|wxLEFT|wxRIGHT, 1 );

	m_bpAltitude = new wxBitmapButton( this, ID_CTRLALTITUDE, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpAltitude->SetToolTip( _("Select geoptential altitude") );

	fgSizer50->Add( m_bpAltitude, 0, wxALL, 1 );

	m_bpNow = new wxBitmapButton( this, ID_BTNNOW, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpNow->SetToolTip( _("Now") );

	fgSizer50->Add( m_bpNow, 0, wxALL, 1 );

	m_bpZoomToCenter = new wxBitmapButton( this, ID_BTNZOOMTC, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpZoomToCenter->SetToolTip( _("Zoom To Center") );

	fgSizer50->Add( m_bpZoomToCenter, 0, wxALL, 1 );

	m_bpShowCursorData = new wxBitmapButton( this, ID_BTNSHOWCDATA, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	fgSizer50->Add( m_bpShowCursorData, 0, wxALL, 1 );


	fgSizer50->Add( 0, 0, 1, wxEXPAND|wxLEFT|wxRIGHT, 1 );

	m_bpPlay = new wxBitmapButton( this, ID_BTNPLAY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	fgSizer50->Add( m_bpPlay, 0, wxBOTTOM|wxLEFT|wxTOP, 1 );

	m_sTimeline = new wxSlider( this, ID_TIMELINE, 1, 0, 10, wxDefaultPosition, wxSize( 90,-1 ), wxSL_HORIZONTAL );
	fgSizer50->Add( m_sTimeline, 0, wxEXPAND, 1 );


	fgSizer50->Add( 0, 0, 1, wxEXPAND|wxLEFT|wxRIGHT, 1 );

	m_bpOpenFile = new wxBitmapButton( this, ID_BTNOPENFILE, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpOpenFile->SetToolTip( _("Open a new file") );

	fgSizer50->Add( m_bpOpenFile, 0, wxALL, 1 );

	m_bpSettings = new wxBitmapButton( this, ID_BTNSETTING, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bpSettings->SetToolTip( _("Settings") );

	fgSizer50->Add( m_bpSettings, 0, wxALL, 1 );

	m_bpRequest = new wxBitmapButton( this, ID_BTNREQUEST, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	fgSizer50->Add( m_bpRequest, 0, wxALL, 1 );


	fgSizer49->Add( fgSizer50, 1, wxEXPAND, 5 );

	m_fgCDataSizer = new wxFlexGridSizer( 0, 2, 0, 0 );
	m_fgCDataSizer->SetFlexibleDirection( wxBOTH );
	m_fgCDataSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );


	fgSizer49->Add( m_fgCDataSizer, 1, wxEXPAND, 5 );


	m_fgCtrlBarSizer->Add( fgSizer49, 1, wxEXPAND, 0 );

	m_fgCtrlGrabberSize = new wxFlexGridSizer( 0, 1, 0, 0 );
	m_fgCtrlGrabberSize->SetFlexibleDirection( wxBOTH );
	m_fgCtrlGrabberSize->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );


	m_fgCtrlBarSizer->Add( m_fgCtrlGrabberSize, 1, wxEXPAND, 0 );


	this->SetSizer( m_fgCtrlBarSizer );
	this->Layout();
	m_fgCtrlBarSizer->Fit( this );

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GRIBUICtrlBarBase::OnClose ) );
	this->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Connect( wxEVT_MIDDLE_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Connect( wxEVT_MIDDLE_UP, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Connect( wxEVT_RIGHT_UP, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Connect( wxEVT_MOTION, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Connect( wxEVT_MIDDLE_DCLICK, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Connect( wxEVT_RIGHT_DCLICK, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Connect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Connect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Connect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Connect( wxEVT_SIZE, wxSizeEventHandler( GRIBUICtrlBarBase::OnSize ) );
	m_bpPrev->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnPrev ), NULL, this );
	m_bpPrev->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_cRecordForecast->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GRIBUICtrlBarBase::OnRecordForecast ), NULL, this );
	m_cRecordForecast->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_bpNext->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnNext ), NULL, this );
	m_bpNext->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_bpAltitude->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnAltitude ), NULL, this );
	m_bpNow->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnNow ), NULL, this );
	m_bpNow->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_bpZoomToCenter->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnZoomToCenterClick ), NULL, this );
	m_bpZoomToCenter->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_bpShowCursorData->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnShowCursorData ), NULL, this );
	m_bpShowCursorData->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_bpPlay->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnPlayStop ), NULL, this );
	m_bpPlay->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_sTimeline->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_sTimeline->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GRIBUICtrlBarBase::OnTimeline ), NULL, this );
	m_sTimeline->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GRIBUICtrlBarBase::OnTimeline ), NULL, this );
	m_sTimeline->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GRIBUICtrlBarBase::OnTimeline ), NULL, this );
	m_sTimeline->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GRIBUICtrlBarBase::OnTimeline ), NULL, this );
	m_sTimeline->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GRIBUICtrlBarBase::OnTimeline ), NULL, this );
	m_sTimeline->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GRIBUICtrlBarBase::OnTimeline ), NULL, this );
	m_sTimeline->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GRIBUICtrlBarBase::OnTimeline ), NULL, this );
	m_sTimeline->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GRIBUICtrlBarBase::OnTimeline ), NULL, this );
	m_sTimeline->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GRIBUICtrlBarBase::OnTimeline ), NULL, this );
	m_bpOpenFile->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnOpenFile ), NULL, this );
	m_bpOpenFile->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_bpSettings->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnSettings ), NULL, this );
	m_bpSettings->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_bpRequest->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnRequest ), NULL, this );
	m_bpRequest->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
}

GRIBUICtrlBarBase::~GRIBUICtrlBarBase()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( GRIBUICtrlBarBase::OnClose ) );
	this->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_LEFT_UP, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_MIDDLE_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_MIDDLE_UP, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_RIGHT_UP, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_MOTION, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_MIDDLE_DCLICK, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_RIGHT_DCLICK, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_SIZE, wxSizeEventHandler( GRIBUICtrlBarBase::OnSize ) );
	m_bpPrev->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnPrev ), NULL, this );
	m_bpPrev->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_cRecordForecast->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GRIBUICtrlBarBase::OnRecordForecast ), NULL, this );
	m_cRecordForecast->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_bpNext->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnNext ), NULL, this );
	m_bpNext->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_bpAltitude->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnAltitude ), NULL, this );
	m_bpNow->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnNow ), NULL, this );
	m_bpNow->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_bpZoomToCenter->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnZoomToCenterClick ), NULL, this );
	m_bpZoomToCenter->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_bpShowCursorData->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnShowCursorData ), NULL, this );
	m_bpShowCursorData->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_bpPlay->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnPlayStop ), NULL, this );
	m_bpPlay->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_sTimeline->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_sTimeline->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GRIBUICtrlBarBase::OnTimeline ), NULL, this );
	m_sTimeline->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GRIBUICtrlBarBase::OnTimeline ), NULL, this );
	m_sTimeline->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GRIBUICtrlBarBase::OnTimeline ), NULL, this );
	m_sTimeline->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GRIBUICtrlBarBase::OnTimeline ), NULL, this );
	m_sTimeline->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GRIBUICtrlBarBase::OnTimeline ), NULL, this );
	m_sTimeline->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GRIBUICtrlBarBase::OnTimeline ), NULL, this );
	m_sTimeline->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GRIBUICtrlBarBase::OnTimeline ), NULL, this );
	m_sTimeline->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GRIBUICtrlBarBase::OnTimeline ), NULL, this );
	m_sTimeline->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GRIBUICtrlBarBase::OnTimeline ), NULL, this );
	m_bpOpenFile->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnOpenFile ), NULL, this );
	m_bpOpenFile->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_bpSettings->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnSettings ), NULL, this );
	m_bpSettings->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );
	m_bpRequest->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GRIBUICtrlBarBase::OnRequest ), NULL, this );
	m_bpRequest->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( GRIBUICtrlBarBase::OnMouseEvent ), NULL, this );

}

GRIBUICDataBase::GRIBUICDataBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxDefaultSize );
	this->SetFont( wxFont( 9, 70, 90, 90, false, wxEmptyString ) );

	wxFlexGridSizer* fgSizer57;
	fgSizer57 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer57->SetFlexibleDirection( wxBOTH );
	fgSizer57->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_fgCdataSizer = new wxFlexGridSizer( 0, 1, 0, 0 );
	m_fgCdataSizer->SetFlexibleDirection( wxBOTH );
	m_fgCdataSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );


	fgSizer57->Add( m_fgCdataSizer, 1, wxEXPAND, 5 );


	this->SetSizer( fgSizer57 );
	this->Layout();
	fgSizer57->Fit( this );

	this->Centre( wxBOTH );
}

GRIBUICDataBase::~GRIBUICDataBase()
{
}

CursorDataBase::CursorDataBase( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
	fgSizer30 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer30->AddGrowableRow( 1 );
	fgSizer30->SetFlexibleDirection( wxBOTH );
	fgSizer30->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );

	m_stTrackingText = new wxStaticText( this, wxID_ANY, _("MyLabel"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stTrackingText->Wrap( -1 );
	fgSizer30->Add( m_stTrackingText, 0, wxEXPAND|wxLEFT, 5 );

	wxFlexGridSizer* fgSizer43;
	fgSizer43 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer43->SetFlexibleDirection( wxBOTH );
	fgSizer43->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_fgTrackingControls = new wxFlexGridSizer( 0, 8, 0, 0 );
	m_fgTrackingControls->SetFlexibleDirection( wxBOTH );
	m_fgTrackingControls->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_cbWind = new wxCheckBox( this, ID_CB_WIND, _("Wind"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbWind, 0, wxEXPAND|wxLEFT|wxTOP, 2 );

	m_tcWindSpeed = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), wxTE_READONLY );
	m_fgTrackingControls->Add( m_tcWindSpeed, 0, 0, 1 );

	m_tcWindSpeedBf = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxPoint( -1,-1 ), wxSize( 45,-1 ), wxTE_READONLY );
	m_fgTrackingControls->Add( m_tcWindSpeedBf, 0, wxALIGN_CENTER_VERTICAL, 1 );

	m_tcWindDirection = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 45,-1 ), wxTE_READONLY );
	m_fgTrackingControls->Add( m_tcWindDirection, 0, 0, 1 );

	m_cbWindGust = new wxCheckBox( this, ID_CB_WIND_GUSTS, _("Wind Gust"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbWindGust, 0, wxALL|wxEXPAND, 5 );

	m_tcWindGust = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), wxTE_READONLY );
	m_fgTrackingControls->Add( m_tcWindGust, 0, 0, 1 );


	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 1 );


	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 1 );

	m_cbPressure = new wxCheckBox( this, ID_CB_PRESSURE, _("Pressure"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbPressure, 0, wxALL|wxEXPAND, 5 );

	m_tcPressure = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), wxTE_READONLY );
	m_tcPressure->SetToolTip( _("Mean Sea Level Pressure (MSLP)") );

	m_fgTrackingControls->Add( m_tcPressure, 0, 0, 1 );


	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 5 );


	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 5 );

	m_cbWave = new wxCheckBox( this, ID_CB_WAVES, _("Waves"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbWave, 0, wxALL|wxEXPAND, 5 );

	m_tcWaveHeight = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), wxTE_READONLY );
	m_tcWaveHeight->SetToolTip( _("Significant Wave Height") );

	m_fgTrackingControls->Add( m_tcWaveHeight, 0, 0, 1 );

	m_tcWavePeriode = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 45,-1 ), wxTE_READONLY );
	m_tcWavePeriode->SetToolTip( _("Waves period") );

	m_fgTrackingControls->Add( m_tcWavePeriode, 0, 0, 1 );

	m_tcWaveDirection = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 45,-1 ), wxTE_READONLY );
	m_tcWaveDirection->SetToolTip( _("Waves Direction") );

	m_fgTrackingControls->Add( m_tcWaveDirection, 0, 0, 1 );

	m_cbCurrent = new wxCheckBox( this, ID_CB_CURRENT, _("Current"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbCurrent, 0, wxALL|wxEXPAND, 5 );

	m_tcCurrentVelocity = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), wxTE_READONLY );
	m_tcCurrentVelocity->SetToolTip( _("Surface Current Speed") );

	m_fgTrackingControls->Add( m_tcCurrentVelocity, 0, 0, 1 );

	m_tcCurrentDirection = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 45,-1 ), wxTE_READONLY );
	m_tcCurrentDirection->SetToolTip( _("Surface Current Direction") );

	m_fgTrackingControls->Add( m_tcCurrentDirection, 0, 0, 1 );


	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 5 );

	m_cbPrecipitation = new wxCheckBox( this, ID_CB_RAINFALL, _("Rainfall"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbPrecipitation, 0, wxALL|wxEXPAND, 5 );

	m_tcPrecipitation = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), wxTE_READONLY );
	m_tcPrecipitation->SetToolTip( _("Cumulative Rainfall per Hour") );

	m_fgTrackingControls->Add( m_tcPrecipitation, 0, 0, 1 );


	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 1 );


	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 5 );

	m_cbCloud = new wxCheckBox( this, ID_CB_CLOUD_COVER, _("Cloud Cover"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbCloud, 0, wxALL|wxEXPAND, 5 );

	m_tcCloud = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), wxTE_READONLY );
	m_tcCloud->SetToolTip( _("Cloud Cover") );

	m_fgTrackingControls->Add( m_tcCloud, 0, 0, 1 );


	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 1 );


	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 5 );

	m_cbAirTemperature = new wxCheckBox( this, ID_CB_AIR_TEMP, _("Air Temp"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbAirTemperature, 0, wxALL|wxEXPAND, 5 );

	m_tcAirTemperature = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), wxTE_READONLY );
	m_fgTrackingControls->Add( m_tcAirTemperature, 0, 0, 1 );


	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 1 );


	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 5 );

	m_cbSeaTemperature = new wxCheckBox( this, ID_CB_SEA_TEMP, _("Sea Temp"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbSeaTemperature, 0, wxALL|wxEXPAND, 5 );

	m_tcSeaTemperature = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 70,-1 ), wxTE_READONLY );
	m_tcSeaTemperature->SetToolTip( _("Sea or Air Temperature at Surface Level") );

	m_fgTrackingControls->Add( m_tcSeaTemperature, 0, 0, 1 );


	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 5 );


	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 5 );

	m_cbCAPE = new wxCheckBox( this, ID_CB_CAPE, _("CAPE"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgTrackingControls->Add( m_cbCAPE, 0, wxALL|wxEXPAND, 5 );

	m_tcCAPE = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxPoint( -1,-1 ), wxSize( 70,-1 ), wxTE_READONLY );
	m_tcCAPE->SetToolTip( _("Convective Available Potential Energy") );

	m_fgTrackingControls->Add( m_tcCAPE, 0, 0, 1 );


	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 5 );


	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 5 );

	m_cbAltitude = new wxCheckBox( this, wxID_ANY, _("Geop. Altitude"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cbAltitude->SetValue(true);
	m_cbAltitude->Enable( false );

	m_fgTrackingControls->Add( m_cbAltitude, 0, wxALL|wxEXPAND, 5 );

	m_tcAltitude = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), wxTE_READONLY );
	m_tcAltitude->SetMinSize( wxSize( 70,-1 ) );

	m_fgTrackingControls->Add( m_tcAltitude, 0, 0, 5 );


	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 5 );


	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 5 );

	m_cbTemp = new wxCheckBox( this, wxID_ANY, _("Geop. Temp"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cbTemp->SetValue(true);
	m_cbTemp->Enable( false );

	m_fgTrackingControls->Add( m_cbTemp, 0, wxALL|wxEXPAND, 5 );

	m_tcTemp = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_tcTemp->SetMinSize( wxSize( 70,-1 ) );

	m_fgTrackingControls->Add( m_tcTemp, 0, 0, 5 );


	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 5 );


	m_fgTrackingControls->Add( 0, 0, 1, wxEXPAND, 5 );

	m_cbRelHumid = new wxCheckBox( this, wxID_ANY, _("Geop. R. Hum."), wxDefaultPosition, wxDefaultSize, 0 );
	m_cbRelHumid->SetValue(true);
	m_cbRelHumid->Enable( false );

	m_fgTrackingControls->Add( m_cbRelHumid, 0, wxALL|wxEXPAND, 5 );

	m_tcRelHumid = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_tcRelHumid->SetMinSize( wxSize( 70,-1 ) );

	m_fgTrackingControls->Add( m_tcRelHumid, 0, 0, 5 );


	fgSizer43->Add( m_fgTrackingControls, 1, wxEXPAND, 1 );


	fgSizer30->Add( fgSizer43, 1, wxEXPAND, 5 );


	this->SetSizer( fgSizer30 );
	this->Layout();
	fgSizer30->Fit( this );

	// Connect Events
	this->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Connect( wxEVT_MIDDLE_DOWN, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Connect( wxEVT_MIDDLE_UP, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Connect( wxEVT_RIGHT_UP, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Connect( wxEVT_MOTION, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Connect( wxEVT_MIDDLE_DCLICK, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Connect( wxEVT_RIGHT_DCLICK, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Connect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Connect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Connect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	m_stTrackingText->Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Connect( wxEVT_LEFT_UP, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Connect( wxEVT_MIDDLE_DOWN, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Connect( wxEVT_MIDDLE_UP, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Connect( wxEVT_RIGHT_UP, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Connect( wxEVT_MOTION, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Connect( wxEVT_MIDDLE_DCLICK, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Connect( wxEVT_RIGHT_DCLICK, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Connect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Connect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Connect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_cbWind->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbWind->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );
	m_cbWindGust->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbWindGust->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );
	m_cbPressure->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbPressure->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );
	m_cbWave->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbWave->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );
	m_cbCurrent->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbCurrent->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );
	m_cbPrecipitation->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbPrecipitation->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );
	m_cbCloud->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbCloud->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );
	m_cbAirTemperature->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbAirTemperature->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );
	m_cbSeaTemperature->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbSeaTemperature->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );
	m_cbCAPE->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbCAPE->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );
}

CursorDataBase::~CursorDataBase()
{
	// Disconnect Events
	this->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_LEFT_UP, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_MIDDLE_DOWN, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_MIDDLE_UP, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_RIGHT_UP, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_MOTION, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_MIDDLE_DCLICK, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_RIGHT_DCLICK, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	this->Disconnect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( CursorDataBase::OnMouseEvent ) );
	m_stTrackingText->Disconnect( wxEVT_LEFT_DOWN, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Disconnect( wxEVT_LEFT_UP, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Disconnect( wxEVT_MIDDLE_DOWN, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Disconnect( wxEVT_MIDDLE_UP, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Disconnect( wxEVT_RIGHT_UP, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Disconnect( wxEVT_MOTION, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Disconnect( wxEVT_MIDDLE_DCLICK, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Disconnect( wxEVT_RIGHT_DCLICK, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Disconnect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Disconnect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_stTrackingText->Disconnect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( CursorDataBase::OnMouseEvent ), NULL, this );
	m_cbWind->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbWind->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );
	m_cbWindGust->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbWindGust->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );
	m_cbPressure->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbPressure->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );
	m_cbWave->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbWave->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );
	m_cbCurrent->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbCurrent->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );
	m_cbPrecipitation->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbPrecipitation->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );
	m_cbCloud->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbCloud->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );
	m_cbAirTemperature->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbAirTemperature->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );
	m_cbSeaTemperature->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbSeaTemperature->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );
	m_cbCAPE->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CursorDataBase::OnCBAny ), NULL, this );
	m_cbCAPE->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( CursorDataBase::OnMenuCallBack ), NULL, this );

}

GribSettingsDialogBase::GribSettingsDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxSize( -1,-1 ) );

	wxFlexGridSizer* fgSizer53;
	fgSizer53 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer53->SetFlexibleDirection( wxBOTH );
	fgSizer53->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_nSettingsBook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	wxScrolledWindow* m_scSetDataPanel;
	m_scSetDataPanel = new wxScrolledWindow( m_nSettingsBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_scSetDataPanel->SetScrollRate( 5, 5 );
	m_fgSetDataSizer = new wxFlexGridSizer( 0, 1, 0, 0 );
	m_fgSetDataSizer->AddGrowableCol( 0 );
	m_fgSetDataSizer->AddGrowableRow( 1 );
	m_fgSetDataSizer->SetFlexibleDirection( wxBOTH );
	m_fgSetDataSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticBoxSizer* sbSizer101;
	sbSizer101 = new wxStaticBoxSizer( new wxStaticBox( m_scSetDataPanel, wxID_ANY, _("Data Display Options") ), wxVERTICAL );

	wxFlexGridSizer* fgSizer15;
	fgSizer15 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer15->SetFlexibleDirection( wxBOTH );
	fgSizer15->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxArrayString m_cDataTypeChoices;
	m_cDataType = new wxChoice( m_scSetDataPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cDataTypeChoices, 0 );
	m_cDataType->SetSelection( 0 );
	fgSizer15->Add( m_cDataType, 0, wxBOTTOM|wxEXPAND|wxTOP, 10 );

	wxStaticText* m_staticText12;
	m_staticText12 = new wxStaticText( m_scSetDataPanel, wxID_ANY, _("Units"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	m_staticText12->SetMinSize( wxSize( 110,-1 ) );

	fgSizer15->Add( m_staticText12, 0, wxALL|wxEXPAND, 15 );

	wxArrayString m_cDataUnitsChoices;
	m_cDataUnits = new wxChoice( m_scSetDataPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cDataUnitsChoices, 0 );
	m_cDataUnits->SetSelection( 0 );
	fgSizer15->Add( m_cDataUnits, 0, wxBOTTOM|wxEXPAND|wxTOP, 5 );

	m_cbBarbedArrows = new wxCheckBox( m_scSetDataPanel, wxID_ANY, _("Barbed Arrows"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer15->Add( m_cbBarbedArrows, 0, wxBOTTOM|wxTOP, 5 );

	m_fgBarbedData1 = new wxFlexGridSizer( 0, 1, 0, 0 );
	m_fgBarbedData1->SetFlexibleDirection( wxBOTH );
	m_fgBarbedData1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxString m_cBarbedColoursChoices[] = { _("Default Colour"), _("Controled Colours") };
	int m_cBarbedColoursNChoices = sizeof( m_cBarbedColoursChoices ) / sizeof( wxString );
	m_cBarbedColours = new wxChoice( m_scSetDataPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cBarbedColoursNChoices, m_cBarbedColoursChoices, 0 );
	m_cBarbedColours->SetSelection( 0 );
	m_fgBarbedData1->Add( m_cBarbedColours, 0, wxALL|wxEXPAND, 5 );

	wxFlexGridSizer* fgSizer42;
	fgSizer42 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer42->SetFlexibleDirection( wxBOTH );
	fgSizer42->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText* m_staticText41;
	m_staticText41 = new wxStaticText( m_scSetDataPanel, wxID_ANY, _("Spacing(pixels)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText41->Wrap( -1 );
	fgSizer42->Add( m_staticText41, 0, wxALL, 5 );

	m_cBarbArrFixSpac = new wxCheckBox( m_scSetDataPanel, BARBFIXSPACING, _("Fixed"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer42->Add( m_cBarbArrFixSpac, 0, wxALL|wxLEFT|wxRIGHT|wxTOP, 5 );


	fgSizer42->Add( 0, 0, 1, wxEXPAND, 5 );

	m_cBarbArrMinSpac = new wxCheckBox( m_scSetDataPanel, BARBMINSPACING, _("Minimum"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer42->Add( m_cBarbArrMinSpac, 0, wxBOTTOM|wxLEFT|wxRIGHT, 5 );


	m_fgBarbedData1->Add( fgSizer42, 1, wxEXPAND, 0 );


	fgSizer15->Add( m_fgBarbedData1, 1, wxEXPAND, 0 );

	m_fgBarbedData2 = new wxFlexGridSizer( 0, 1, 0, 0 );
	m_fgBarbedData2->SetFlexibleDirection( wxBOTH );
	m_fgBarbedData2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_cBarbedVisibility = new wxCheckBox( m_scSetDataPanel, wxID_ANY, _("Always visible"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgBarbedData2->Add( m_cBarbedVisibility, 0, wxALL|wxEXPAND, 5 );

	m_sBarbArrSpacing = new wxSpinCtrl( m_scSetDataPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 30, 100, 30 );
	m_fgBarbedData2->Add( m_sBarbArrSpacing, 0, wxBOTTOM|wxTOP, 15 );


	fgSizer15->Add( m_fgBarbedData2, 1, wxEXPAND, 0 );

	m_cbIsoBars = new wxCheckBox( m_scSetDataPanel, wxID_ANY, _("IsoBars"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer15->Add( m_cbIsoBars, 0, wxBOTTOM|wxTOP, 5 );

	m_fIsoBarSpacing = new wxFlexGridSizer( 0, 2, 0, 0 );
	m_fIsoBarSpacing->SetFlexibleDirection( wxBOTH );
	m_fIsoBarSpacing->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_tIsoBarSpacing = new wxStaticText( m_scSetDataPanel, wxID_ANY, _("Spacing"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tIsoBarSpacing->Wrap( -1 );
	m_fIsoBarSpacing->Add( m_tIsoBarSpacing, 0, wxALL, 5 );

	m_sIsoBarSpacing = new wxSpinCtrl( m_scSetDataPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 90,-1 ), wxSP_ARROW_KEYS, 1, 1000, 1 );
	m_fIsoBarSpacing->Add( m_sIsoBarSpacing, 0, wxALL, 5 );


	fgSizer15->Add( m_fIsoBarSpacing, 1, wxALL|wxEXPAND, 0 );

	m_fIsoBarVisibility = new wxFlexGridSizer( 0, 1, 0, 0 );
	m_fIsoBarVisibility->SetFlexibleDirection( wxBOTH );
	m_fIsoBarVisibility->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_sIsoBarVisibility = new wxCheckBox( m_scSetDataPanel, wxID_ANY, _("Always visible"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fIsoBarVisibility->Add( m_sIsoBarVisibility, 0, wxALL|wxEXPAND, 5 );


	fgSizer15->Add( m_fIsoBarVisibility, 1, wxALL|wxEXPAND, 5 );

	m_cbDirectionArrows = new wxCheckBox( m_scSetDataPanel, wxID_ANY, _("Direction Arrows"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer15->Add( m_cbDirectionArrows, 0, wxBOTTOM|wxTOP, 5 );

	m_fgDirArrData1 = new wxFlexGridSizer( 0, 1, 0, 0 );
	m_fgDirArrData1->SetFlexibleDirection( wxBOTH );
	m_fgDirArrData1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxString m_cDirectionArrowFormChoices[] = { _("Single Arrow"), _("Double Arrow"), _("Proportional Arrow") };
	int m_cDirectionArrowFormNChoices = sizeof( m_cDirectionArrowFormChoices ) / sizeof( wxString );
	m_cDirectionArrowForm = new wxChoice( m_scSetDataPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cDirectionArrowFormNChoices, m_cDirectionArrowFormChoices, 0 );
	m_cDirectionArrowForm->SetSelection( 1 );
	m_fgDirArrData1->Add( m_cDirectionArrowForm, 0, wxBOTTOM|wxEXPAND|wxTOP, 5 );

	wxFlexGridSizer* fgSizer43;
	fgSizer43 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer43->SetFlexibleDirection( wxBOTH );
	fgSizer43->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText* m_staticText42;
	m_staticText42 = new wxStaticText( m_scSetDataPanel, wxID_ANY, _("Spacing(pixels)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText42->Wrap( -1 );
	fgSizer43->Add( m_staticText42, 0, wxALL, 5 );

	m_cDirArrFixSpac = new wxCheckBox( m_scSetDataPanel, DIRFIXSPACING, _("Fixed"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer43->Add( m_cDirArrFixSpac, 0, wxLEFT|wxRIGHT|wxTOP, 5 );


	fgSizer43->Add( 0, 0, 1, wxEXPAND, 5 );

	m_cDirArrMinSpac = new wxCheckBox( m_scSetDataPanel, DIRMINSPACING, _("Minimum"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer43->Add( m_cDirArrMinSpac, 0, wxBOTTOM|wxLEFT|wxRIGHT, 5 );


	m_fgDirArrData1->Add( fgSizer43, 1, wxEXPAND, 0 );


	fgSizer15->Add( m_fgDirArrData1, 1, wxEXPAND, 5 );

	m_fgDirArrData2 = new wxFlexGridSizer( 0, 1, 0, 0 );
	m_fgDirArrData2->SetFlexibleDirection( wxBOTH );
	m_fgDirArrData2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxString m_cDirectionArrowSizeChoices[] = { _("Default Size"), _("Small Size") };
	int m_cDirectionArrowSizeNChoices = sizeof( m_cDirectionArrowSizeChoices ) / sizeof( wxString );
	m_cDirectionArrowSize = new wxChoice( m_scSetDataPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cDirectionArrowSizeNChoices, m_cDirectionArrowSizeChoices, 0 );
	m_cDirectionArrowSize->SetSelection( 0 );
	m_fgDirArrData2->Add( m_cDirectionArrowSize, 0, wxALL|wxEXPAND, 5 );

	m_sDirArrSpacing = new wxSpinCtrl( m_scSetDataPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 30, 100, 30 );
	m_fgDirArrData2->Add( m_sDirArrSpacing, 0, wxALL, 5 );


	fgSizer15->Add( m_fgDirArrData2, 1, wxEXPAND, 0 );

	m_cbOverlayMap = new wxCheckBox( m_scSetDataPanel, wxID_ANY, _("OverlayMap"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer15->Add( m_cbOverlayMap, 0, wxBOTTOM|wxTOP, 5 );

	m_tOverlayColors = new wxStaticText( m_scSetDataPanel, wxID_ANY, _("Colors"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tOverlayColors->Wrap( -1 );
	fgSizer15->Add( m_tOverlayColors, 0, wxALL|wxEXPAND, 5 );

	wxString m_cOverlayColorsChoices[] = { _("Generic"), _("Wind"), _("Air Temp"), _("Sea Temp"), _("Rainfall"), _("Cloud Cover"), _("Current") };
	int m_cOverlayColorsNChoices = sizeof( m_cOverlayColorsChoices ) / sizeof( wxString );
	m_cOverlayColors = new wxChoice( m_scSetDataPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cOverlayColorsNChoices, m_cOverlayColorsChoices, 0 );
	m_cOverlayColors->SetSelection( 0 );
	fgSizer15->Add( m_cOverlayColors, 0, wxALL|wxEXPAND, 5 );

	m_cbNumbers = new wxCheckBox( m_scSetDataPanel, wxID_ANY, _("Numbers"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer15->Add( m_cbNumbers, 0, wxBOTTOM|wxTOP, 5 );

	m_fgNumData1 = new wxFlexGridSizer( 0, 2, 0, 0 );
	m_fgNumData1->SetFlexibleDirection( wxBOTH );
	m_fgNumData1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticText* m_staticText43;
	m_staticText43 = new wxStaticText( m_scSetDataPanel, wxID_ANY, _("Spacing(pixels)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText43->Wrap( -1 );
	m_fgNumData1->Add( m_staticText43, 0, wxALL, 5 );

	m_cNumFixSpac = new wxCheckBox( m_scSetDataPanel, NUMFIXSPACING, _("Fixed\n"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgNumData1->Add( m_cNumFixSpac, 0, wxLEFT|wxTOP, 5 );


	m_fgNumData1->Add( 0, 0, 1, wxEXPAND, 5 );

	m_cNumMinSpac = new wxCheckBox( m_scSetDataPanel, NUMMINSPACING, _("Minimum"), wxDefaultPosition, wxDefaultSize, 0 );
	m_fgNumData1->Add( m_cNumMinSpac, 0, wxBOTTOM|wxLEFT, 5 );


	fgSizer15->Add( m_fgNumData1, 1, wxEXPAND, 0 );

	m_sNumbersSpacing = new wxSpinCtrl( m_scSetDataPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 90,-1 ), wxSP_ARROW_KEYS, 30, 100, 50 );
	fgSizer15->Add( m_sNumbersSpacing, 0, wxALL, 5 );

	m_cbParticles = new wxCheckBox( m_scSetDataPanel, wxID_ANY, _("Particle Map"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer15->Add( m_cbParticles, 0, wxBOTTOM|wxTOP, 5 );

	m_ctParticles = new wxStaticText( m_scSetDataPanel, wxID_ANY, _("Density"), wxDefaultPosition, wxDefaultSize, 0 );
	m_ctParticles->Wrap( -1 );
	fgSizer15->Add( m_ctParticles, 0, wxALL, 5 );

	m_sParticleDensity = new wxSlider( m_scSetDataPanel, wxID_ANY, 5, 1, 10, wxDefaultPosition, wxDefaultSize, wxSL_BOTTOM|wxSL_HORIZONTAL|wxSL_LABELS );
	fgSizer15->Add( m_sParticleDensity, 0, wxALL|wxEXPAND, 5 );


	sbSizer101->Add( fgSizer15, 1, wxEXPAND, 5 );


	m_fgSetDataSizer->Add( sbSizer101, 1, wxEXPAND|wxTOP, 10 );


	m_fgSetDataSizer->Add( 0, 0, 1, wxEXPAND, 5 );

	wxStaticBoxSizer* sbSizer12;
	sbSizer12 = new wxStaticBoxSizer( new wxStaticBox( m_scSetDataPanel, wxID_ANY, _("Transparency for all Overlay Maps") ), wxVERTICAL );

	wxFlexGridSizer* fgSizer34;
	fgSizer34 = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizer34->SetFlexibleDirection( wxBOTH );
	fgSizer34->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_staticText24 = new wxStaticText( m_scSetDataPanel, wxID_ANY, _("Overlay Transparency (%)"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText24->Wrap( -1 );
	fgSizer34->Add( m_staticText24, 0, wxEXPAND|wxRIGHT|wxTOP, 15 );

	m_sTransparency = new wxSlider( m_scSetDataPanel, wxID_ANY, 50, 0, 100, wxDefaultPosition, wxSize( -1,-1 ), wxSL_BOTTOM|wxSL_HORIZONTAL|wxSL_LABELS );
	m_sTransparency->SetMinSize( wxSize( 250,-1 ) );

	fgSizer34->Add( m_sTransparency, 0, wxALL|wxEXPAND, 5 );


	sbSizer12->Add( fgSizer34, 1, wxEXPAND|wxLEFT, 5 );


	m_fgSetDataSizer->Add( sbSizer12, 1, wxEXPAND|wxTOP, 10 );


	m_scSetDataPanel->SetSizer( m_fgSetDataSizer );
	m_scSetDataPanel->Layout();
	m_fgSetDataSizer->Fit( m_scSetDataPanel );
	m_nSettingsBook->AddPage( m_scSetDataPanel, _("Data"), false );
	wxScrolledWindow* m_scSetPlaybackPanel;
	m_scSetPlaybackPanel = new wxScrolledWindow( m_nSettingsBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_scSetPlaybackPanel->SetScrollRate( 5, 5 );
	m_fgSetPlaybackSizer = new wxFlexGridSizer( 0, 1, 0, 0 );
	m_fgSetPlaybackSizer->AddGrowableCol( 0 );
	m_fgSetPlaybackSizer->SetFlexibleDirection( wxBOTH );
	m_fgSetPlaybackSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticBoxSizer* sbSizer121;
	sbSizer121 = new wxStaticBoxSizer( new wxStaticBox( m_scSetPlaybackPanel, wxID_ANY, _("Playback Options") ), wxVERTICAL );

	wxFlexGridSizer* fgSizer48;
	fgSizer48 = new wxFlexGridSizer( 0, 3, 0, 0 );
	fgSizer48->SetFlexibleDirection( wxBOTH );
	fgSizer48->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_cLoopMode = new wxCheckBox( m_scSetPlaybackPanel, wxID_ANY, _("Loop Mode"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer48->Add( m_cLoopMode, 0, wxALL, 5 );

	m_staticText26 = new wxStaticText( m_scSetPlaybackPanel, wxID_ANY, _("Loop Start"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText26->Wrap( -1 );
	fgSizer48->Add( m_staticText26, 0, wxALL|wxEXPAND, 5 );

	wxString m_cLoopStartPointChoices[] = { _("Top of Grib File"), _("Current time forecast") };
	int m_cLoopStartPointNChoices = sizeof( m_cLoopStartPointChoices ) / sizeof( wxString );
	m_cLoopStartPoint = new wxChoice( m_scSetPlaybackPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cLoopStartPointNChoices, m_cLoopStartPointChoices, 0 );
	m_cLoopStartPoint->SetSelection( 0 );
	fgSizer48->Add( m_cLoopStartPoint, 0, wxALL|wxEXPAND, 5 );


	fgSizer48->Add( 0, 0, 1, wxEXPAND, 5 );

	wxStaticText* m_staticText4;
	m_staticText4 = new wxStaticText( m_scSetPlaybackPanel, wxID_ANY, _("Updates per Second"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	fgSizer48->Add( m_staticText4, 0, wxALL|wxEXPAND, 5 );

	m_sUpdatesPerSecond = new wxSpinCtrl( m_scSetPlaybackPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 90,-1 ), wxSP_ARROW_KEYS, 1, 60, 4 );
	fgSizer48->Add( m_sUpdatesPerSecond, 0, wxALL, 5 );

	m_cInterpolate = new wxCheckBox( m_scSetPlaybackPanel, wxID_ANY, _("Interpolate between gribs"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer48->Add( m_cInterpolate, 0, wxALL, 5 );

	m_tSlicesPerUpdate = new wxStaticText( m_scSetPlaybackPanel, wxID_ANY, _("Time Interval"), wxDefaultPosition, wxDefaultSize, 0 );
	m_tSlicesPerUpdate->Wrap( -1 );
	fgSizer48->Add( m_tSlicesPerUpdate, 0, wxALL|wxEXPAND, 5 );

	wxArrayString m_sSlicesPerUpdateChoices;
	m_sSlicesPerUpdate = new wxChoice( m_scSetPlaybackPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_sSlicesPerUpdateChoices, 0 );
	m_sSlicesPerUpdate->SetSelection( 0 );
	fgSizer48->Add( m_sSlicesPerUpdate, 0, wxALL|wxEXPAND, 5 );


	sbSizer121->Add( fgSizer48, 1, wxEXPAND, 5 );


	m_fgSetPlaybackSizer->Add( sbSizer121, 1, wxEXPAND, 5 );


	m_scSetPlaybackPanel->SetSizer( m_fgSetPlaybackSizer );
	m_scSetPlaybackPanel->Layout();
	m_fgSetPlaybackSizer->Fit( m_scSetPlaybackPanel );
	m_nSettingsBook->AddPage( m_scSetPlaybackPanel, _("Playback"), false );
	wxScrolledWindow* m_scSetGuiPanel;
	m_scSetGuiPanel = new wxScrolledWindow( m_nSettingsBook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
	m_scSetGuiPanel->SetScrollRate( 5, 5 );
	m_fgSetGuiSizer = new wxFlexGridSizer( 0, 1, 0, 0 );
	m_fgSetGuiSizer->AddGrowableCol( 0 );
	m_fgSetGuiSizer->SetFlexibleDirection( wxBOTH );
	m_fgSetGuiSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticBoxSizer* sbSizer9;
	sbSizer9 = new wxStaticBoxSizer( new wxStaticBox( m_scSetGuiPanel, wxID_ANY, _("Dialogs Style") ), wxVERTICAL );

	wxFlexGridSizer* fgSizer47;
	fgSizer47 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer47->SetFlexibleDirection( wxBOTH );
	fgSizer47->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_rbCurDataAttaWCap = new wxRadioButton( m_scSetGuiPanel, wxID_ANY, _("Only one Dialog with a title/drag bar"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
	fgSizer47->Add( m_rbCurDataAttaWCap, 0, wxALL, 5 );

	m_rbCurDataAttaWoCap = new wxRadioButton( m_scSetGuiPanel, wxID_ANY, _("Only one Dialog without title/drag bar"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer47->Add( m_rbCurDataAttaWoCap, 0, wxALL, 5 );

	m_rbCurDataIsolHoriz = new wxRadioButton( m_scSetGuiPanel, wxID_ANY, _("Two separated Dialogs - horizontal Cursor Data display"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer47->Add( m_rbCurDataIsolHoriz, 0, wxALL, 5 );

	m_rbCurDataIsolVertic = new wxRadioButton( m_scSetGuiPanel, wxID_ANY, _("Two separated Dialogs - vertical Cursor Data display"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer47->Add( m_rbCurDataIsolVertic, 0, wxALL, 5 );


	sbSizer9->Add( fgSizer47, 1, wxEXPAND, 5 );


	m_fgSetGuiSizer->Add( sbSizer9, 1, wxEXPAND, 5 );


	m_fgSetGuiSizer->Add( 0, 0, 1, wxALL, 5 );

	wxStaticBoxSizer* sbSizer10;
	sbSizer10 = new wxStaticBoxSizer( new wxStaticBox( m_scSetGuiPanel, wxID_ANY, _("Buttons/Ctrl Visibility") ), wxVERTICAL );

	wxFlexGridSizer* fgSizer52;
	fgSizer52 = new wxFlexGridSizer( 0, 4, 0, 0 );
	fgSizer52->SetFlexibleDirection( wxBOTH );
	fgSizer52->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );


	fgSizer52->Add( 0, 0, 1, wxEXPAND, 5 );


	fgSizer52->Add( 0, 0, 1, wxEXPAND, 5 );

	wxStaticText* m_staticText44;
	m_staticText44 = new wxStaticText( m_scSetGuiPanel, wxID_ANY, _("Attached Dialog"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText44->Wrap( -1 );
	fgSizer52->Add( m_staticText44, 0, wxBOTTOM|wxLEFT|wxRIGHT, 5 );

	wxStaticText* m_staticText45;
	m_staticText45 = new wxStaticText( m_scSetGuiPanel, wxID_ANY, _("Isolated Dialog"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText45->Wrap( -1 );
	fgSizer52->Add( m_staticText45, 0, wxBOTTOM|wxLEFT|wxRIGHT, 5 );

	m_biAltitude = new wxStaticBitmap( m_scSetGuiPanel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_biAltitude, 0, wxALL, 2 );

	wxStaticText* m_staticText46;
	m_staticText46 = new wxStaticText( m_scSetGuiPanel, wxID_ANY, _("Altitude Choice"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText46->Wrap( -1 );
	fgSizer52->Add( m_staticText46, 0, wxALL, 3 );

	wxCheckBox* m_checkBox48;
	m_checkBox48 = new wxCheckBox( m_scSetGuiPanel, AC0, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_checkBox48, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );

	wxCheckBox* m_checkBox49;
	m_checkBox49 = new wxCheckBox( m_scSetGuiPanel, AC1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_checkBox49, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );

	m_biNow = new wxStaticBitmap( m_scSetGuiPanel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_biNow, 0, wxALL, 3 );

	wxStaticText* m_staticText47;
	m_staticText47 = new wxStaticText( m_scSetGuiPanel, wxID_ANY, _("Now"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText47->Wrap( -1 );
	fgSizer52->Add( m_staticText47, 0, wxALL, 3 );

	wxCheckBox* m_checkBox50;
	m_checkBox50 = new wxCheckBox( m_scSetGuiPanel, NW0, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_checkBox50, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );

	wxCheckBox* m_checkBox51;
	m_checkBox51 = new wxCheckBox( m_scSetGuiPanel, NW1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_checkBox51, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );

	m_biZoomToCenter = new wxStaticBitmap( m_scSetGuiPanel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_biZoomToCenter, 0, wxALL, 3 );

	wxStaticText* m_staticText48;
	m_staticText48 = new wxStaticText( m_scSetGuiPanel, wxID_ANY, _("Zoom to Center"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText48->Wrap( -1 );
	fgSizer52->Add( m_staticText48, 0, wxALL, 3 );

	wxCheckBox* m_checkBox52;
	m_checkBox52 = new wxCheckBox( m_scSetGuiPanel, ZC0, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_checkBox52, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );

	wxCheckBox* m_checkBox53;
	m_checkBox53 = new wxCheckBox( m_scSetGuiPanel, ZC1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_checkBox53, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );

	m_biShowCursorData = new wxStaticBitmap( m_scSetGuiPanel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_biShowCursorData, 0, wxALL, 3 );

	wxStaticText* m_staticText49;
	m_staticText49 = new wxStaticText( m_scSetGuiPanel, wxID_ANY, _("Show Cursor Data"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText49->Wrap( -1 );
	fgSizer52->Add( m_staticText49, 0, wxALL, 3 );

	wxCheckBox* m_checkBox54;
	m_checkBox54 = new wxCheckBox( m_scSetGuiPanel, SCD0, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_checkBox54, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );

	wxCheckBox* m_checkBox55;
	m_checkBox55 = new wxCheckBox( m_scSetGuiPanel, SCD1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_checkBox55, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );

	m_biPlay = new wxStaticBitmap( m_scSetGuiPanel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_biPlay, 0, wxALL, 3 );

	wxStaticText* m_staticText50;
	m_staticText50 = new wxStaticText( m_scSetGuiPanel, wxID_ANY, _("Playback"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText50->Wrap( -1 );
	fgSizer52->Add( m_staticText50, 0, wxALL, 3 );

	wxCheckBox* m_checkBox56;
	m_checkBox56 = new wxCheckBox( m_scSetGuiPanel, PB0, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_checkBox56, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );

	wxCheckBox* m_checkBox57;
	m_checkBox57 = new wxCheckBox( m_scSetGuiPanel, PB1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_checkBox57, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );

	m_biTimeSlider = new wxStaticBitmap( m_scSetGuiPanel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_biTimeSlider, 0, wxALL, 3 );

	wxStaticText* m_staticText51;
	m_staticText51 = new wxStaticText( m_scSetGuiPanel, wxID_ANY, _("Time Slider"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText51->Wrap( -1 );
	fgSizer52->Add( m_staticText51, 0, wxALL, 3 );

	wxCheckBox* m_checkBox58;
	m_checkBox58 = new wxCheckBox( m_scSetGuiPanel, TL0, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_checkBox58, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );

	wxCheckBox* m_checkBox59;
	m_checkBox59 = new wxCheckBox( m_scSetGuiPanel, TL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_checkBox59, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );

	m_biOpenFile = new wxStaticBitmap( m_scSetGuiPanel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_biOpenFile, 0, wxALL, 3 );

	wxStaticText* m_staticText52;
	m_staticText52 = new wxStaticText( m_scSetGuiPanel, wxID_ANY, _("Open File"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText52->Wrap( -1 );
	fgSizer52->Add( m_staticText52, 0, wxALL, 3 );

	wxCheckBox* m_checkBox60;
	m_checkBox60 = new wxCheckBox( m_scSetGuiPanel, OF0, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_checkBox60, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );

	wxCheckBox* m_checkBox61;
	m_checkBox61 = new wxCheckBox( m_scSetGuiPanel, OF1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_checkBox61, 0, wxALIGN_CENTER_HORIZONTAL, 5 );

	m_biSettings = new wxStaticBitmap( m_scSetGuiPanel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_biSettings, 0, wxALL, 3 );

	wxStaticText* m_staticText53;
	m_staticText53 = new wxStaticText( m_scSetGuiPanel, wxID_ANY, _("Settings"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText53->Wrap( -1 );
	fgSizer52->Add( m_staticText53, 0, wxALL, 3 );

	wxCheckBox* m_checkBox62;
	m_checkBox62 = new wxCheckBox( m_scSetGuiPanel, STS0, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_checkBox62, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );

	wxCheckBox* m_checkBox63;
	m_checkBox63 = new wxCheckBox( m_scSetGuiPanel, STS1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_checkBox63, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );

	m_biRequest = new wxStaticBitmap( m_scSetGuiPanel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_biRequest, 0, wxALL, 3 );

	wxStaticText* m_staticText54;
	m_staticText54 = new wxStaticText( m_scSetGuiPanel, wxID_ANY, _("Request"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText54->Wrap( -1 );
	fgSizer52->Add( m_staticText54, 0, wxALL, 3 );

	wxCheckBox* m_checkBox64;
	m_checkBox64 = new wxCheckBox( m_scSetGuiPanel, RQ0, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_checkBox64, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );

	wxCheckBox* m_checkBox65;
	m_checkBox65 = new wxCheckBox( m_scSetGuiPanel, RQ1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer52->Add( m_checkBox65, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3 );


	sbSizer10->Add( fgSizer52, 1, wxEXPAND, 5 );


	m_fgSetGuiSizer->Add( sbSizer10, 1, wxEXPAND, 5 );


	m_scSetGuiPanel->SetSizer( m_fgSetGuiSizer );
	m_scSetGuiPanel->Layout();
	m_fgSetGuiSizer->Fit( m_scSetGuiPanel );
	m_nSettingsBook->AddPage( m_scSetGuiPanel, _("GUI"), true );

	fgSizer53->Add( m_nSettingsBook, 1, wxEXPAND | wxALL, 5 );

	m_sButton = new wxStdDialogButtonSizer();
	m_sButtonOK = new wxButton( this, wxID_OK );
	m_sButton->AddButton( m_sButtonOK );
	m_sButtonApply = new wxButton( this, wxID_APPLY );
	m_sButton->AddButton( m_sButtonApply );
	m_sButtonCancel = new wxButton( this, wxID_CANCEL );
	m_sButton->AddButton( m_sButtonCancel );
	m_sButton->Realize();

	fgSizer53->Add( m_sButton, 1, wxALIGN_RIGHT|wxSHAPED, 5 );


	this->SetSizer( fgSizer53 );
	this->Layout();
	fgSizer53->Fit( this );

	this->Centre( wxBOTH );

	// Connect Events
	m_nSettingsBook->Connect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( GribSettingsDialogBase::OnPageChange ), NULL, this );
	m_cDataType->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribSettingsDialogBase::OnDataTypeChoice ), NULL, this );
	m_cDataUnits->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribSettingsDialogBase::OnUnitChange ), NULL, this );
	m_cBarbArrFixSpac->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnSpacingModeChange ), NULL, this );
	m_cBarbArrMinSpac->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnSpacingModeChange ), NULL, this );
	m_cDirArrFixSpac->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnSpacingModeChange ), NULL, this );
	m_cDirArrMinSpac->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnSpacingModeChange ), NULL, this );
	m_cNumFixSpac->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnSpacingModeChange ), NULL, this );
	m_cNumMinSpac->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnSpacingModeChange ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_cInterpolate->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnIntepolateChange ), NULL, this );
	m_rbCurDataAttaWCap->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( GribSettingsDialogBase::OnCtrlandDataStyleChanged ), NULL, this );
	m_rbCurDataAttaWoCap->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( GribSettingsDialogBase::OnCtrlandDataStyleChanged ), NULL, this );
	m_rbCurDataIsolHoriz->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( GribSettingsDialogBase::OnCtrlandDataStyleChanged ), NULL, this );
	m_rbCurDataIsolVertic->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( GribSettingsDialogBase::OnCtrlandDataStyleChanged ), NULL, this );
	m_sButtonApply->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnApply ), NULL, this );
}

GribSettingsDialogBase::~GribSettingsDialogBase()
{
	// Disconnect Events
	m_nSettingsBook->Disconnect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( GribSettingsDialogBase::OnPageChange ), NULL, this );
	m_cDataType->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribSettingsDialogBase::OnDataTypeChoice ), NULL, this );
	m_cDataUnits->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribSettingsDialogBase::OnUnitChange ), NULL, this );
	m_cBarbArrFixSpac->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnSpacingModeChange ), NULL, this );
	m_cBarbArrMinSpac->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnSpacingModeChange ), NULL, this );
	m_cDirArrFixSpac->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnSpacingModeChange ), NULL, this );
	m_cDirArrMinSpac->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnSpacingModeChange ), NULL, this );
	m_cNumFixSpac->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnSpacingModeChange ), NULL, this );
	m_cNumMinSpac->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnSpacingModeChange ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_sTransparency->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( GribSettingsDialogBase::OnTransparencyChange ), NULL, this );
	m_cInterpolate->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnIntepolateChange ), NULL, this );
	m_rbCurDataAttaWCap->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( GribSettingsDialogBase::OnCtrlandDataStyleChanged ), NULL, this );
	m_rbCurDataAttaWoCap->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( GribSettingsDialogBase::OnCtrlandDataStyleChanged ), NULL, this );
	m_rbCurDataIsolHoriz->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( GribSettingsDialogBase::OnCtrlandDataStyleChanged ), NULL, this );
	m_rbCurDataIsolVertic->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( GribSettingsDialogBase::OnCtrlandDataStyleChanged ), NULL, this );
	m_sButtonApply->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( GribSettingsDialogBase::OnApply ), NULL, this );

}

GribPreferencesDialogBase::GribPreferencesDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	wxStaticBoxSizer* sbSizer9;
	sbSizer9 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("General Options") ), wxVERTICAL );

	wxFlexGridSizer* fgSizer46;
	fgSizer46 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer46->SetFlexibleDirection( wxBOTH );
	fgSizer46->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_cbUseHiDef = new wxCheckBox( this, wxID_ANY, _("Use High Definition Graphics"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer46->Add( m_cbUseHiDef, 0, wxALL, 5 );

	m_cbUseGradualColors = new wxCheckBox( this, wxID_ANY, _("Use Gradual Colors"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer46->Add( m_cbUseGradualColors, 0, wxALL, 5 );

	m_cbCopyFirstCumulativeRecord = new wxCheckBox( this, wxID_ANY, _("Copy First Cumulative Missing Record"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer46->Add( m_cbCopyFirstCumulativeRecord, 0, wxALL, 5 );

	m_cbCopyMissingWaveRecord = new wxCheckBox( this, wxID_ANY, _("Copy Missing Wave Records"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer46->Add( m_cbCopyMissingWaveRecord, 0, wxALL, 5 );


	sbSizer9->Add( fgSizer46, 1, wxEXPAND, 5 );


	fgSizer6->Add( sbSizer9, 1, wxEXPAND, 5 );

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
	m_rbTimeFormat->SetSelection( 1 );
	fgSizer6->Add( m_rbTimeFormat, 0, wxALL|wxEXPAND, 5 );

	wxStdDialogButtonSizer* m_sdbSizer2;
	wxButton* m_sdbSizer2OK;
	wxButton* m_sdbSizer2Cancel;
	m_sdbSizer2 = new wxStdDialogButtonSizer();
	m_sdbSizer2OK = new wxButton( this, wxID_OK );
	m_sdbSizer2->AddButton( m_sdbSizer2OK );
	m_sdbSizer2Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer2->AddButton( m_sdbSizer2Cancel );
	m_sdbSizer2->Realize();

	fgSizer6->Add( m_sdbSizer2, 1, wxEXPAND, 5 );


	this->SetSizer( fgSizer6 );
	this->Layout();
	fgSizer6->Fit( this );

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
	m_sScrolledDialog->SetScrollRate( 0, 5 );
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

	m_cManualZoneSel = new wxCheckBox( m_sScrolledDialog, wxID_ANY, _("Manual Selection"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer36->Add( m_cManualZoneSel, 0, wxLEFT, 5 );

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


	m_sScrolledDialog->SetSizer( m_fgScrollSizer );
	m_sScrolledDialog->Layout();
	m_fgScrollSizer->Fit( m_sScrolledDialog );
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
	m_sMovingSpeed->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( GribRequestSettingBase::OnAnySpinChange ), NULL, this );
	m_sMovingCourse->Connect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( GribRequestSettingBase::OnAnySpinChange ), NULL, this );
	m_pLogin->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pCode->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pResolution->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pInterval->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pTimeRange->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribRequestSettingBase::OnTimeRangeChange ), NULL, this );
	m_cManualZoneSel->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnZoneSelectionModeChange ), NULL, this );
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
	m_sMovingSpeed->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( GribRequestSettingBase::OnAnySpinChange ), NULL, this );
	m_sMovingCourse->Disconnect( wxEVT_COMMAND_SPINCTRL_UPDATED, wxSpinEventHandler( GribRequestSettingBase::OnAnySpinChange ), NULL, this );
	m_pLogin->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pCode->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pResolution->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pInterval->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribRequestSettingBase::OnAnyChange ), NULL, this );
	m_pTimeRange->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( GribRequestSettingBase::OnTimeRangeChange ), NULL, this );
	m_cManualZoneSel->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( GribRequestSettingBase::OnZoneSelectionModeChange ), NULL, this );
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
