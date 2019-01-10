///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "RoutePropDlg.h"

///////////////////////////////////////////////////////////////////////////

RoutePropDlg::RoutePropDlg( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxFrame( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 300,300 ), wxDefaultSize );

	wxBoxSizer* bSizerMain;
	bSizerMain = new wxBoxSizer( wxVERTICAL );

	m_ntbRteProp = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_pnlBasic = new wxPanel( m_ntbRteProp, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizerData;
	bSizerData = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* bSizerName;
	bSizerName = new wxBoxSizer( wxHORIZONTAL );

	m_stName = new wxStaticText( m_pnlBasic, wxID_ANY, _("Name"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stName->Wrap( -1 );
	bSizerName->Add( m_stName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_tcName = new wxTextCtrl( m_pnlBasic, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizerName->Add( m_tcName, 1, wxALL, 5 );


	bSizerData->Add( bSizerName, 0, wxEXPAND, 5 );

	wxBoxSizer* bSizerFromTo;
	bSizerFromTo = new wxBoxSizer( wxHORIZONTAL );

	m_stFrom = new wxStaticText( m_pnlBasic, wxID_ANY, _("From"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stFrom->Wrap( -1 );
	bSizerFromTo->Add( m_stFrom, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_tcFrom = new wxTextCtrl( m_pnlBasic, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizerFromTo->Add( m_tcFrom, 1, wxALL, 5 );

	m_stTo = new wxStaticText( m_pnlBasic, wxID_ANY, _("To"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stTo->Wrap( -1 );
	bSizerFromTo->Add( m_stTo, 0, wxALL, 5 );

	m_tcTo = new wxTextCtrl( m_pnlBasic, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	bSizerFromTo->Add( m_tcTo, 1, wxALL, 5 );


	bSizerData->Add( bSizerFromTo, 0, wxEXPAND, 5 );

	wxWrapSizer* wSizerParams;
	wSizerParams = new wxWrapSizer( wxHORIZONTAL, wxEXTEND_LAST_ON_EACH_LINE|wxREMOVE_LEADING_SPACES|wxWRAPSIZER_DEFAULT_FLAGS );

	wxBoxSizer* bSizerDistance;
	bSizerDistance = new wxBoxSizer( wxVERTICAL );

	m_stDistTotal = new wxStaticText( m_pnlBasic, wxID_ANY, _("Distance"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stDistTotal->Wrap( -1 );
	bSizerDistance->Add( m_stDistTotal, 0, wxALL, 5 );

	m_tcDistance = new wxTextCtrl( m_pnlBasic, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_tcDistance->SetMaxSize( wxSize( 80,-1 ) );

	bSizerDistance->Add( m_tcDistance, 0, wxALL|wxEXPAND, 5 );


	wSizerParams->Add( bSizerDistance, 1, wxEXPAND, 0 );

	wxBoxSizer* bSizerSpeed;
	bSizerSpeed = new wxBoxSizer( wxVERTICAL );

	m_stPlanSpeed = new wxStaticText( m_pnlBasic, wxID_ANY, _("Plan speed"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stPlanSpeed->Wrap( -1 );
	bSizerSpeed->Add( m_stPlanSpeed, 0, wxALL, 5 );

	m_tcPlanSpeed = new wxTextCtrl( m_pnlBasic, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
	m_tcPlanSpeed->SetMaxSize( wxSize( 80,-1 ) );

	bSizerSpeed->Add( m_tcPlanSpeed, 0, wxALL, 5 );


	wSizerParams->Add( bSizerSpeed, 1, wxEXPAND, 0 );

	wxBoxSizer* bSizerEnroute;
	bSizerEnroute = new wxBoxSizer( wxVERTICAL );

	m_stEnroute = new wxStaticText( m_pnlBasic, wxID_ANY, _("Enroute"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stEnroute->Wrap( -1 );
	bSizerEnroute->Add( m_stEnroute, 0, wxALL, 5 );

	m_tcEnroute = new wxTextCtrl( m_pnlBasic, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
	m_tcEnroute->SetMaxSize( wxSize( 80,-1 ) );

	bSizerEnroute->Add( m_tcEnroute, 0, wxALL, 5 );


	wSizerParams->Add( bSizerEnroute, 1, wxEXPAND, 0 );

	wxBoxSizer* bSizerDeparture;
	bSizerDeparture = new wxBoxSizer( wxVERTICAL );

	m_stDeparture = new wxStaticText( m_pnlBasic, wxID_ANY, _("Departure"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stDeparture->Wrap( -1 );
	bSizerDeparture->Add( m_stDeparture, 0, wxALL, 5 );

	wxBoxSizer* bSizerDepartureTS;
	bSizerDepartureTS = new wxBoxSizer( wxHORIZONTAL );

	m_dpDepartureDate = new wxDatePickerCtrl( m_pnlBasic, wxID_ANY, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT );
	bSizerDepartureTS->Add( m_dpDepartureDate, 0, 0, 5 );

	m_tpDepartureTime = new wxTimePickerCtrl( m_pnlBasic, wxID_ANY, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT );
	bSizerDepartureTS->Add( m_tpDepartureTime, 0, 0, 5 );


	bSizerDeparture->Add( bSizerDepartureTS, 0, 0, 5 );


	wSizerParams->Add( bSizerDeparture, 1, wxEXPAND, 0 );

	wxBoxSizer* bSizerTime;
	bSizerTime = new wxBoxSizer( wxVERTICAL );

	m_stTimeZone = new wxStaticText( m_pnlBasic, wxID_ANY, _("Time"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stTimeZone->Wrap( -1 );
	bSizerTime->Add( m_stTimeZone, 0, wxALL, 5 );

	wxString m_choiceTimezoneChoices[] = { _("UTC"), _("Local@PC"), _("LMT@Location") };
	int m_choiceTimezoneNChoices = sizeof( m_choiceTimezoneChoices ) / sizeof( wxString );
	m_choiceTimezone = new wxChoice( m_pnlBasic, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choiceTimezoneNChoices, m_choiceTimezoneChoices, 0 );
	m_choiceTimezone->SetSelection( 0 );
	m_choiceTimezone->SetMaxSize( wxSize( 100,-1 ) );

	bSizerTime->Add( m_choiceTimezone, 0, wxALL, 5 );


	wSizerParams->Add( bSizerTime, 1, wxEXPAND, 0 );


	bSizerData->Add( wSizerParams, 0, wxEXPAND, 0 );

	m_dvlcWaypoints = new wxDataViewListCtrl( m_pnlBasic, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_dataViewListColumnLeg = m_dvlcWaypoints->AppendTextColumn( _("Leg"), wxDATAVIEW_CELL_INERT, 30, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
	m_dataViewListColumnToWpt = m_dvlcWaypoints->AppendTextColumn( _("To waypoint"), wxDATAVIEW_CELL_INERT, 80, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
	m_dataViewListColumnDistance = m_dvlcWaypoints->AppendTextColumn( _("Distance"), wxDATAVIEW_CELL_INERT, 50, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
	m_dataViewListColumnBearing = m_dvlcWaypoints->AppendTextColumn( _("Bearing"), wxDATAVIEW_CELL_INERT, 50, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
	m_dataViewListColumnLat = m_dvlcWaypoints->AppendTextColumn( _("Latitude"), wxDATAVIEW_CELL_INERT, 80, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
	m_dataViewListColumnLon = m_dvlcWaypoints->AppendTextColumn( _("Longitude"), wxDATAVIEW_CELL_INERT, 80, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
	m_dataViewListColumnETE = m_dvlcWaypoints->AppendTextColumn( _("ETE"), wxDATAVIEW_CELL_INERT, 80, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
	m_dataViewListColumnETA = m_dvlcWaypoints->AppendTextColumn( _("ETA"), wxDATAVIEW_CELL_INERT, -1, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
	m_dataViewListColumnSpeed = m_dvlcWaypoints->AppendTextColumn( _("Speed"), wxDATAVIEW_CELL_EDITABLE, 50, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
	m_dataViewListColumnNTE = m_dvlcWaypoints->AppendTextColumn( _("Next tide event"), wxDATAVIEW_CELL_INERT, -1, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
	m_dataViewListColumnDesc = m_dvlcWaypoints->AppendTextColumn( _("Description"), wxDATAVIEW_CELL_INERT, -1, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
	m_dataViewListColumnCourse = m_dvlcWaypoints->AppendTextColumn( _("Course"), wxDATAVIEW_CELL_INERT, 80, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
	m_dataViewListColumnEmpty = m_dvlcWaypoints->AppendTextColumn( wxEmptyString, wxDATAVIEW_CELL_INERT, -1, static_cast<wxAlignment>(wxALIGN_LEFT), wxDATAVIEW_COL_RESIZABLE );
	bSizerData->Add( m_dvlcWaypoints, 1, wxALL|wxEXPAND, 5 );


	m_pnlBasic->SetSizer( bSizerData );
	m_pnlBasic->Layout();
	bSizerData->Fit( m_pnlBasic );
	m_ntbRteProp->AddPage( m_pnlBasic, _("Basic"), false );
	m_pnlAdvanced = new wxPanel( m_ntbRteProp, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer19;
	bSizer19 = new wxBoxSizer( wxVERTICAL );

	m_stDescription = new wxStaticText( m_pnlAdvanced, wxID_ANY, _("Description"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stDescription->Wrap( -1 );
	bSizer19->Add( m_stDescription, 0, wxALL, 5 );

	m_textCtrl9 = new wxTextCtrl( m_pnlAdvanced, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
	bSizer19->Add( m_textCtrl9, 0, wxALL|wxEXPAND, 5 );

	wxStaticBoxSizer* sbSizerLinks;
	sbSizerLinks = new wxStaticBoxSizer( new wxStaticBox( m_pnlAdvanced, wxID_ANY, _("Links") ), wxVERTICAL );


	bSizer19->Add( sbSizerLinks, 1, wxEXPAND, 5 );


	m_pnlAdvanced->SetSizer( bSizer19 );
	m_pnlAdvanced->Layout();
	bSizer19->Fit( m_pnlAdvanced );
	m_ntbRteProp->AddPage( m_pnlAdvanced, _("Advanced"), false );

	bSizerMain->Add( m_ntbRteProp, 1, wxEXPAND | wxALL, 5 );

	wxWrapSizer* wSizerCustomBtns;
	wSizerCustomBtns = new wxWrapSizer( wxHORIZONTAL, wxWRAPSIZER_DEFAULT_FLAGS );

	m_btnPrint = new wxButton( this, wxID_ANY, _("Print"), wxDefaultPosition, wxDefaultSize, 0 );
	wSizerCustomBtns->Add( m_btnPrint, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_btnExtend = new wxButton( this, wxID_ANY, _("Extend"), wxDefaultPosition, wxDefaultSize, 0 );
	wSizerCustomBtns->Add( m_btnExtend, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_btnSplit = new wxButton( this, wxID_ANY, _("Split"), wxDefaultPosition, wxDefaultSize, 0 );
	wSizerCustomBtns->Add( m_btnSplit, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


	wSizerCustomBtns->Add( 0, 0, 1, wxEXPAND, 5 );

	m_sdbSizerBtns = new wxStdDialogButtonSizer();
	m_sdbSizerBtnsOK = new wxButton( this, wxID_OK );
	m_sdbSizerBtns->AddButton( m_sdbSizerBtnsOK );
	m_sdbSizerBtnsCancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizerBtns->AddButton( m_sdbSizerBtnsCancel );
	m_sdbSizerBtns->Realize();

	wSizerCustomBtns->Add( m_sdbSizerBtns, 1, wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );


	bSizerMain->Add( wSizerCustomBtns, 0, wxALIGN_BOTTOM|wxEXPAND, 0 );


	this->SetSizer( bSizerMain );
	this->Layout();

	this->Centre( wxBOTH );

	// Connect Events
	this->Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( RoutePropDlg::RoutePropDlgOnClose ) );
	this->Connect( wxEVT_SIZE, wxSizeEventHandler( RoutePropDlg::RoutePropDlgOnSize ) );
	m_ntbRteProp->Connect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( RoutePropDlg::RoutePropDlgOnNotebookPageChanged ), NULL, this );
	m_tcPlanSpeed->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( RoutePropDlg::PlanSpeedOnText ), NULL, this );
	m_tcPlanSpeed->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( RoutePropDlg::PlanSpeedOnTextEnter ), NULL, this );
	m_dpDepartureDate->Connect( wxEVT_DATE_CHANGED, wxDateEventHandler( RoutePropDlg::DepartureDateOnDateChanged ), NULL, this );
	m_tpDepartureTime->Connect( wxEVT_TIME_CHANGED, wxDateEventHandler( RoutePropDlg::DepartureTimeOnTimeChanged ), NULL, this );
	m_choiceTimezone->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( RoutePropDlg::TimezoneOnChoice ), NULL, this );
	m_dvlcWaypoints->Connect( wxEVT_COMMAND_DATAVIEW_ITEM_EDITING_DONE, wxDataViewEventHandler( RoutePropDlg::WaypointsOnDataViewListCtrlItemEditingDone ), NULL, this );
	m_dvlcWaypoints->Connect( wxEVT_COMMAND_DATAVIEW_ITEM_EDITING_STARTED, wxDataViewEventHandler( RoutePropDlg::WaypointsOnDataViewListCtrlItemEditingStarted ), NULL, this );
	m_dvlcWaypoints->Connect( wxEVT_COMMAND_DATAVIEW_ITEM_START_EDITING, wxDataViewEventHandler( RoutePropDlg::WaypointsOnDataViewListCtrlItemStartEditing ), NULL, this );
	m_dvlcWaypoints->Connect( wxEVT_COMMAND_DATAVIEW_ITEM_VALUE_CHANGED, wxDataViewEventHandler( RoutePropDlg::WaypointsOnDataViewListCtrlItemValueChanged ), NULL, this );
	m_btnPrint->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RoutePropDlg::PrintOnButtonClick ), NULL, this );
	m_btnExtend->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RoutePropDlg::ExtendOnButtonClick ), NULL, this );
	m_btnSplit->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RoutePropDlg::SplitOnButtonClick ), NULL, this );
	m_sdbSizerBtnsCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RoutePropDlg::BtnsOnCancelButtonClick ), NULL, this );
	m_sdbSizerBtnsOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RoutePropDlg::BtnsOnOKButtonClick ), NULL, this );
}

RoutePropDlg::~RoutePropDlg()
{
	// Disconnect Events
	this->Disconnect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( RoutePropDlg::RoutePropDlgOnClose ) );
	this->Disconnect( wxEVT_SIZE, wxSizeEventHandler( RoutePropDlg::RoutePropDlgOnSize ) );
	m_ntbRteProp->Disconnect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( RoutePropDlg::RoutePropDlgOnNotebookPageChanged ), NULL, this );
	m_tcPlanSpeed->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( RoutePropDlg::PlanSpeedOnText ), NULL, this );
	m_tcPlanSpeed->Disconnect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( RoutePropDlg::PlanSpeedOnTextEnter ), NULL, this );
	m_dpDepartureDate->Disconnect( wxEVT_DATE_CHANGED, wxDateEventHandler( RoutePropDlg::DepartureDateOnDateChanged ), NULL, this );
	m_tpDepartureTime->Disconnect( wxEVT_TIME_CHANGED, wxDateEventHandler( RoutePropDlg::DepartureTimeOnTimeChanged ), NULL, this );
	m_choiceTimezone->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( RoutePropDlg::TimezoneOnChoice ), NULL, this );
	m_dvlcWaypoints->Disconnect( wxEVT_COMMAND_DATAVIEW_ITEM_EDITING_DONE, wxDataViewEventHandler( RoutePropDlg::WaypointsOnDataViewListCtrlItemEditingDone ), NULL, this );
	m_dvlcWaypoints->Disconnect( wxEVT_COMMAND_DATAVIEW_ITEM_EDITING_STARTED, wxDataViewEventHandler( RoutePropDlg::WaypointsOnDataViewListCtrlItemEditingStarted ), NULL, this );
	m_dvlcWaypoints->Disconnect( wxEVT_COMMAND_DATAVIEW_ITEM_START_EDITING, wxDataViewEventHandler( RoutePropDlg::WaypointsOnDataViewListCtrlItemStartEditing ), NULL, this );
	m_dvlcWaypoints->Disconnect( wxEVT_COMMAND_DATAVIEW_ITEM_VALUE_CHANGED, wxDataViewEventHandler( RoutePropDlg::WaypointsOnDataViewListCtrlItemValueChanged ), NULL, this );
	m_btnPrint->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RoutePropDlg::PrintOnButtonClick ), NULL, this );
	m_btnExtend->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RoutePropDlg::ExtendOnButtonClick ), NULL, this );
	m_btnSplit->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RoutePropDlg::SplitOnButtonClick ), NULL, this );
	m_sdbSizerBtnsCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RoutePropDlg::BtnsOnCancelButtonClick ), NULL, this );
	m_sdbSizerBtnsOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( RoutePropDlg::BtnsOnOKButtonClick ), NULL, this );

}
