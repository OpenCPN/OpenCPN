///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun  6 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "squiddioPrefsDialogBase.h"
#include <wx/scrolwin.h>

#ifdef __OCPN__ANDROID__
#include <QtWidgets/QScroller>
#include "qdebug.h"
#endif

///////////////////////////////////////////////////////////////////////////

SquiddioPrefsDialogBase::SquiddioPrefsDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

        wxBoxSizer* topSizer = new wxBoxSizer( wxVERTICAL );
        SetSizer( topSizer );
        
        wxBoxSizer* labelSizer = new wxBoxSizer( wxHORIZONTAL );
        topSizer->Add( labelSizer, 0, wxEXPAND, 5 );
        
        wxStaticText *labelBox = new wxStaticText( this, wxID_ANY, _("Squiddio PlugIn Preferences"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
        labelSizer->Add(labelBox, 1, wxEXPAND, 0);
        
        
        wxScrolledWindow *itemScrollWin = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxSize(-1, -1), wxVSCROLL | wxHSCROLL);
        itemScrollWin->SetScrollRate(2, 2);
        
        topSizer->Add( itemScrollWin, 1, wxEXPAND | wxALL, 0 );
        
        wxBoxSizer* scrollSizer = new wxBoxSizer( wxVERTICAL );
        itemScrollWin->SetSizer( scrollSizer );

        //--------------------------------------------------------
        
//	wxFlexGridSizer* fgMainSizer = new wxFlexGridSizer( 0, 1, 0, 0 );
//        scrollSizer->Add(fgMainSizer, 1, wxEXPAND | wxALL, 0);
        
//	fgMainSizer->SetFlexibleDirection( wxBOTH );
//	fgMainSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	wxFlexGridSizer* fgSubSizer = new wxFlexGridSizer( 3, 1, 20, 0 );
        scrollSizer->Add( fgSubSizer, 1, wxEXPAND, 5 );
        fgSubSizer->SetFlexibleDirection( wxBOTH );
	fgSubSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
//	fgSubSizer->SetMinSize( wxSize( -1,400 ) ); 
        
 	wxStaticBoxSizer* sbDestSizer;
        sbDestSizer = new wxStaticBoxSizer( new wxStaticBox( itemScrollWin, wxID_ANY, _("View Destinations") ), wxVERTICAL );
         fgSubSizer->Add( sbDestSizer, 1, wxEXPAND, 5 );
        
	wxFlexGridSizer* fgSizer5 = new wxFlexGridSizer( 0, 3, 0, 0 );
        fgSubSizer/*sbDestSizer*/->Add( fgSizer5, 1, wxEXPAND, 5 );
        fgSizer5->SetFlexibleDirection( wxBOTH );
	fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
        m_checkBoxAll = new wxCheckBox( itemScrollWin, ID_CB_ALL, _("All"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer5->Add( m_checkBoxAll, 0, wxALL, 5 );
	
        m_checkBoxMarinas = new wxCheckBox( itemScrollWin, wxID_ANY, _("Marinas"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer5->Add( m_checkBoxMarinas, 0, wxALL, 5 );
	
        m_checkBoxAnchorages = new wxCheckBox( itemScrollWin, wxID_ANY, _("Anchorages"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer5->Add( m_checkBoxAnchorages, 0, wxALL, 5 );
	
        m_checkBoxYachtClubs = new wxCheckBox( itemScrollWin, wxID_ANY, _("Yacht clubs"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer5->Add( m_checkBoxYachtClubs, 0, wxALL, 5 );
	
        m_checkBoxDocks = new wxCheckBox( itemScrollWin, wxID_ANY, _("Docks/piers"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer5->Add( m_checkBoxDocks, 0, wxALL, 5 );
	
        m_checkBoxRamps = new wxCheckBox( itemScrollWin, wxID_ANY, _("Boat ramps"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer5->Add( m_checkBoxRamps, 0, wxALL, 5 );
	
        m_checkBoxFuelStations = new wxCheckBox( itemScrollWin, wxID_ANY, _("Fuel Stations"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer5->Add( m_checkBoxFuelStations, 0, wxALL, 5 );
	
        m_checkBoxOthers = new wxCheckBox( itemScrollWin, wxID_ANY, _("Other"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer5->Add( m_checkBoxOthers, 0, wxALL, 5 );
	
        m_checkBoxAIS = new wxCheckBox( itemScrollWin, wxID_ANY, _("AIS marker"), wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer5->Add( m_checkBoxAIS, 0, wxALL, 5 );
	
	
	
	
	
	wxStaticBoxSizer* sbLogSizer;
        sbLogSizer = new wxStaticBoxSizer( new wxStaticBox( itemScrollWin, wxID_ANY, _("Share logs with friends") ), wxVERTICAL );
	
	wxFlexGridSizer* fgSizerLogs;
	fgSizerLogs = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizerLogs->SetFlexibleDirection( wxBOTH );
	fgSizerLogs->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
        m_staticText6 = new wxStaticText( itemScrollWin, wxID_ANY, _("Send"), wxDefaultPosition, wxDefaultSize, 0 );
	//m_staticText6->Wrap( -1 );
	fgSizerLogs->Add( m_staticText6, 0, wxALL | wxEXPAND, 5 );
	
	wxString m_choiceHowOftenChoices[] = { _("Never"), _("Once a day"), _("Twice a day"), _("Once an hour"), _("Twice an hour"), _("Once a minute"), _("Twice a minute"), _("Every 10 seconds") };
	int m_choiceHowOftenNChoices = sizeof( m_choiceHowOftenChoices ) / sizeof( wxString );
        m_choiceHowOften = new wxChoice( itemScrollWin, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choiceHowOftenNChoices, m_choiceHowOftenChoices, 0 );
	m_choiceHowOften->SetSelection( 0 );
	m_choiceHowOften->SetMinSize( wxSize( 250,-1 ) );
	
	fgSizerLogs->Add( m_choiceHowOften, 0, wxALL, 5 );
	
        m_staticText4 = new wxStaticText( itemScrollWin, wxID_ANY, _("Receive"), wxDefaultPosition, wxSize(150, -1), 0 );
	//m_staticText4->Wrap( -1 );
        fgSizerLogs->Add( m_staticText4, 0, wxALL | wxEXPAND, 5 );
	
	wxString m_choiceReceiveChoices[] = { _("Never"), _("Once a day"), _("Twice a day"), _("Once an hour"), _("Twice an hour"), _("Once a minute"), _("Twice a minute"), _("Every 10 seconds") };
	int m_choiceReceiveNChoices = sizeof( m_choiceReceiveChoices ) / sizeof( wxString );
        m_choiceReceive = new wxChoice( itemScrollWin, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choiceReceiveNChoices, m_choiceReceiveChoices, 0 );
	m_choiceReceive->SetSelection( 0 );
	m_choiceReceive->SetMinSize( wxSize( 250,-1 ) );
	
	fgSizerLogs->Add( m_choiceReceive, 0, wxALL, 5 );
	
        m_staticText7 = new wxStaticText( itemScrollWin, wxID_ANY, _("sQuiddio ID"), wxDefaultPosition, wxDefaultSize, 0 );
	//m_staticText7->Wrap( -1 );
        fgSizerLogs->Add( m_staticText7, 0, wxALL | wxEXPAND, 5 );
	
        m_textSquiddioID = new wxTextCtrl( itemScrollWin, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_textSquiddioID->Enable( false );
	m_textSquiddioID->SetToolTip( _("The email address you use to log into sQuidd.io") );
	
	fgSizerLogs->Add( m_textSquiddioID, 0, wxALL|wxEXPAND, 5 );
	
        m_staticText8 = new wxStaticText( itemScrollWin, wxID_ANY, _("API Key"), wxDefaultPosition, wxDefaultSize, 0 );
	//m_staticText8->Wrap( -1 );
        fgSizerLogs->Add( m_staticText8, 0, wxALL | wxEXPAND, 5 );
	
        m_textApiKey = new wxTextCtrl( itemScrollWin, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_textApiKey->Enable( false );
	m_textApiKey->SetToolTip( _("Retrieve your API Key from your sQuidd.io profile. Log into http://squidd.io and click Edit Profile in the Dashboard, then select the Numbers and Keys tab. Click Help for more information. \n\nIMPORTANT! Do not use your sQuiddio Password as your API Key! ") );
	
	fgSizerLogs->Add( m_textApiKey, 0, wxALL|wxEXPAND, 5 );
	
	
	sbLogSizer->Add( fgSizerLogs, 1, wxEXPAND, 5 );
	fgSubSizer->Add( sbLogSizer, 1, wxEXPAND, 5 );

        wxBoxSizer* m_sdbButtonSizer = new wxBoxSizer( wxHORIZONTAL );
        topSizer->Add( m_sdbButtonSizer, 0, wxEXPAND, 5 );
        
	m_sdbButtonSizerOK = new wxButton( this, wxID_OK );
        m_sdbButtonSizer->Add( m_sdbButtonSizerOK, 0, wxALL | wxALIGN_RIGHT, 15 );
	m_sdbButtonSizerCancel = new wxButton( this, wxID_CANCEL );
        m_sdbButtonSizer->Add( m_sdbButtonSizerCancel, 0, wxALL | wxALIGN_RIGHT, 15 );

        m_sdbButtonSizerHelp = new wxButton( this, wxID_HELP );
        m_sdbButtonSizer->Add( m_sdbButtonSizerHelp, 0, wxALL | wxALIGN_RIGHT, 15 );
        
#ifdef __OCPN__ANDROID__        
        m_sdbButtonSizerHelp->Hide();
#endif        
        
//	m_sdbButtonSizer->Realize();
	
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_checkBoxAll->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( SquiddioPrefsDialogBase::OnCheckBoxAll ), NULL, this );
	m_choiceHowOften->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( SquiddioPrefsDialogBase::OnShareChoice ), NULL, this );
	m_choiceReceive->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( SquiddioPrefsDialogBase::OnShareChoice ), NULL, this );
	m_sdbButtonSizerHelp->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SquiddioPrefsDialogBase::LaunchHelpPage ), NULL, this );
        m_sdbButtonSizerOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SquiddioPrefsDialogBase::OnOKClick ), NULL, this );
        
}

SquiddioPrefsDialogBase::~SquiddioPrefsDialogBase()
{
	// Disconnect Events
	m_checkBoxAll->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( SquiddioPrefsDialogBase::OnCheckBoxAll ), NULL, this );
	m_choiceHowOften->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( SquiddioPrefsDialogBase::OnShareChoice ), NULL, this );
	m_choiceReceive->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( SquiddioPrefsDialogBase::OnShareChoice ), NULL, this );
	m_sdbButtonSizerHelp->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SquiddioPrefsDialogBase::LaunchHelpPage ), NULL, this );
        m_sdbButtonSizerOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SquiddioPrefsDialogBase::OnOKClick ), NULL, this );
        
}
