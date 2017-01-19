///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "WmmUIDialog.h"
///////////////////////////////////////////////////////////////////////////

WmmUIDialogBase::WmmUIDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
        wxFont *pFont = OCPNGetFont(_T("Dialog"), 0);
        SetFont(*pFont);
    
	this->SetSizeHints( wxSize( -1,-1 ), wxSize( -1,-1 ) );
	
	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer6->AddGrowableCol( 0 );
	fgSizer6->AddGrowableRow( 0 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	bSframe = new wxBoxSizer( wxVERTICAL );
        fgSizer6->Add( bSframe, 1, wxEXPAND, 5 );
	bSframe->SetMinSize( wxSize( 200,-1 ) ); 
        
        
	sbSboat = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Boat") ), wxVERTICAL );
	
	gSboat = new wxFlexGridSizer( 6, 2, 0, 0 );
	gSboat->AddGrowableCol( 1 );
	gSboat->SetFlexibleDirection( wxBOTH );
	gSboat->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
        bSframe->Add( sbSboat, 1, wxEXPAND|wxFIXED_MINSIZE, 0 );
        
	m_staticText8 = new wxStaticText( this, wxID_ANY, _("F "), wxDefaultPosition, wxDefaultSize, 0 );
	//m_staticText8->Wrap( -1 );
	m_staticText8->SetToolTip( _("Total Intensity") );
	
	gSboat->Add( m_staticText8, 0, wxALL, 5 );
	
	m_tbF = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxSIMPLE_BORDER );
	m_tbF->SetMaxLength( 0 ); 
	gSboat->Add( m_tbF, 1, wxEXPAND, 5 );
	
	m_staticText9 = new wxStaticText( this, wxID_ANY, _("H "), wxDefaultPosition, wxDefaultSize, 0 );
	//m_staticText9->Wrap( -1 );
	m_staticText9->SetToolTip( _("Horizontal Intensity") );
	
	gSboat->Add( m_staticText9, 0, wxALL, 5 );
	
	m_tbH = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxSIMPLE_BORDER );
	m_tbH->SetMaxLength( 0 ); 
	gSboat->Add( m_tbH, 1, wxEXPAND, 5 );
	
	m_staticText10 = new wxStaticText( this, wxID_ANY, _("X "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10->Wrap( -1 );
	m_staticText10->SetToolTip( _("North Component") );
	
	gSboat->Add( m_staticText10, 0, wxALL, 5 );
	
	m_tbX = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxSIMPLE_BORDER );
	m_tbX->SetMaxLength( 0 ); 
	gSboat->Add( m_tbX, 1, wxEXPAND, 5 );
	
	m_staticText11 = new wxStaticText( this, wxID_ANY, _("Y "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText11->Wrap( -1 );
	m_staticText11->SetToolTip( _("East Component") );
	
	gSboat->Add( m_staticText11, 0, wxALL, 5 );
	
	m_tbY = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxSIMPLE_BORDER );
	m_tbY->SetMaxLength( 0 ); 
	gSboat->Add( m_tbY, 1, wxEXPAND, 5 );
	
	m_staticText12 = new wxStaticText( this, wxID_ANY, _("Z "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	m_staticText12->SetToolTip( _("Vertical Component") );
	
	gSboat->Add( m_staticText12, 0, wxALL, 5 );
	
	m_tbZ = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxSIMPLE_BORDER );
	m_tbZ->SetMaxLength( 0 ); 
	gSboat->Add( m_tbZ, 1, wxEXPAND, 5 );
	
	m_staticText14 = new wxStaticText( this, wxID_ANY, _("Incl"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText14->Wrap( -1 );
	m_staticText14->SetToolTip( _("(DIP) - Geomagnetic Inclination ") );
	m_staticText14->SetMinSize( wxSize( 50,-1 ) );
	
	gSboat->Add( m_staticText14, 0, wxALL, 5 );
	
	m_tbI = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxSIMPLE_BORDER );
	m_tbI->SetMaxLength( 0 ); 
	gSboat->Add( m_tbI, 1, wxEXPAND, 0 );
	
	
	sbSboat->Add( gSboat, 1, wxEXPAND, 0 );

        wxBoxSizer *varBox = new wxBoxSizer(wxHORIZONTAL);
        sbSboat->Add( varBox, 0, wxEXPAND, 0 );
        
        m_staticText13 = new wxStaticText( this, wxID_ANY, _("Vari"), wxDefaultPosition, wxDefaultSize, 0 );
        //m_staticText13->Wrap( -1 );
        m_staticText13->SetToolTip( _("Magnetic Variation") );
        varBox->Add( m_staticText13, 1, wxALL, 5 );
        
        m_tbD = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxSIMPLE_BORDER );
        m_tbD->SetMaxLength( 0 ); 
        //m_tbD->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );
        
        varBox->Add( m_tbD, 3, wxEXPAND, 5 );
        
        
        
        bSframe->AddSpacer(5);
        
        
	sbScursor = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Cursor") ), wxVERTICAL );
        bSframe->Add( sbScursor, 1, wxEXPAND|wxFIXED_MINSIZE, 0 );
        
	gScursor = new wxFlexGridSizer( 6, 2, 0, 0 );
	gScursor->AddGrowableCol( 1 );
	gScursor->SetFlexibleDirection( wxBOTH );
	gScursor->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
        sbScursor->Add( gScursor, 1, wxEXPAND|wxFIXED_MINSIZE, 0 );
        
	m_staticText81 = new wxStaticText( this, wxID_ANY, _("F "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText81->Wrap( -1 );
	m_staticText81->SetToolTip( _("Total Intensity") );
	
	gScursor->Add( m_staticText81, 0, wxALL, 5 );
	
	m_tcF = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxSIMPLE_BORDER );
	m_tcF->SetMaxLength( 0 ); 
	gScursor->Add( m_tcF, 0, wxEXPAND, 5 );
	
	m_staticText91 = new wxStaticText( this, wxID_ANY, _("H "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText91->Wrap( -1 );
	m_staticText91->SetToolTip( _("Horizontal Intensity") );
	
	gScursor->Add( m_staticText91, 0, wxALL, 5 );
	
	m_tcH = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxSIMPLE_BORDER );
	m_tcH->SetMaxLength( 0 ); 
	gScursor->Add( m_tcH, 0, wxEXPAND, 5 );
	
	m_staticText101 = new wxStaticText( this, wxID_ANY, _("X "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText101->Wrap( -1 );
	m_staticText101->SetToolTip( _("North Component") );
	
	gScursor->Add( m_staticText101, 0, wxALL, 5 );
	
	m_tcX = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxSIMPLE_BORDER );
	m_tcX->SetMaxLength( 0 ); 
	gScursor->Add( m_tcX, 0, wxEXPAND, 5 );
	
	m_staticText111 = new wxStaticText( this, wxID_ANY, _("Y "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText111->Wrap( -1 );
	m_staticText111->SetToolTip( _("East Component") );
	
	gScursor->Add( m_staticText111, 0, wxALL, 5 );
	
	m_tcY = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxSIMPLE_BORDER );
	m_tcY->SetMaxLength( 0 ); 
	gScursor->Add( m_tcY, 0, wxEXPAND, 5 );
	
	m_staticText121 = new wxStaticText( this, wxID_ANY, _("Z "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText121->Wrap( -1 );
	m_staticText121->SetToolTip( _("Vertical Component") );
	
	gScursor->Add( m_staticText121, 0, wxALL, 5 );
	
	m_tcZ = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxSIMPLE_BORDER );
	m_tcZ->SetMaxLength( 0 ); 
	gScursor->Add( m_tcZ, 0, wxEXPAND, 5 );
	
	m_staticText141 = new wxStaticText( this, wxID_ANY, _("Incl"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText141->Wrap( -1 );
	m_staticText141->SetToolTip( _("(DIP) - Geomagnetic Inclination ") );
	m_staticText141->SetMinSize( wxSize( 50,-1 ) );
	
	gScursor->Add( m_staticText141, 0, wxALL, 5 );
	
	m_tcI = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxSIMPLE_BORDER );
	m_tcI->SetMaxLength( 0 ); 
	gScursor->Add( m_tcI, 0, wxEXPAND, 5 );
	
        wxBoxSizer *varBoxCursor = new wxBoxSizer(wxHORIZONTAL);
        sbScursor->Add( varBoxCursor, 0, wxEXPAND, 0 );
        
        m_staticText131 = new wxStaticText( this, wxID_ANY, _("Vari"), wxDefaultPosition, wxDefaultSize, 0 );
        //m_staticText131->Wrap( -1 );
        m_staticText131->SetToolTip( _("Magnetic Variation") );
        varBoxCursor->Add( m_staticText131, 1, wxALL, 5 );
        
        m_tcD = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxSIMPLE_BORDER );
        m_tcD->SetMaxLength( 0 ); 
        //m_tcD->SetFont( wxFont( wxNORMAL_FONT->GetPointSize(), 70, 90, 92, false, wxEmptyString ) );
        
        varBoxCursor->Add( m_tcD, 3, wxEXPAND, 5 );

        bSframe->AddSpacer(5);
        
	
	
	
	//sbPlot = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Plot"), wxVERTICAL );
        sbPlot = new wxStaticBoxSizer(  wxVERTICAL, this, _("Plot") );
        fgSizer6->Add( sbPlot, 1, wxEXPAND/*|wxFIXED_MINSIZE*/, 0 );
        
        
	m_cbEnablePlot = new wxCheckBox( this, wxID_ANY, _("Enable plot"), wxDefaultPosition, wxDefaultSize, 0 );
        //m_cbEnablePlot->SetMinSize( wxSize( 200,-1 ) );
        sbPlot->Add( m_cbEnablePlot, 0, wxEXPAND | wxALL, 5 );
	
	m_bPlotSettings = new wxButton( this, wxID_ANY, _("Settings..."), wxDefaultPosition, wxDefaultSize, 0 );
        //m_bPlotSettings->SetMinSize( wxSize( 200,-1 ) );
        sbPlot->Add( m_bPlotSettings, 0, wxALL, 5 );
	
        
        wxStdDialogButtonSizer *m_sdbSizerBtns = new wxStdDialogButtonSizer();
        wxButton *m_sdbSizerBtnsOK = new wxButton( this, wxID_OK );
        m_sdbSizerBtns->AddButton( m_sdbSizerBtnsOK );
        m_sdbSizerBtns->Realize();
        
        fgSizer6->Add( m_sdbSizerBtns, 0, wxALL|wxEXPAND, 5 );
	
	this->SetSizer( fgSizer6 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_cbEnablePlot->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( WmmUIDialogBase::EnablePlotChanged ), NULL, this );
	m_bPlotSettings->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( WmmUIDialogBase::PlotSettings ), NULL, this );
}

WmmUIDialogBase::~WmmUIDialogBase()
{
	// Disconnect Events
	m_cbEnablePlot->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( WmmUIDialogBase::EnablePlotChanged ), NULL, this );
	m_bPlotSettings->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( WmmUIDialogBase::PlotSettings ), NULL, this );
	
}

WmmPlotSettingsDialogBase::WmmPlotSettingsDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxGridSizer* gSizer2;
	gSizer2 = new wxGridSizer( 0, 3, 0, 0 );
	
	m_cbDeclination = new wxCheckBox( this, wxID_ANY, _("Declination"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cbDeclination->SetValue(true); 
	gSizer2->Add( m_cbDeclination, 0, wxALL, 5 );
	
	m_scDeclinationSpacing = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 90, 10 );
	gSizer2->Add( m_scDeclinationSpacing, 0, wxALL, 5 );
	
	m_staticText15 = new wxStaticText( this, wxID_ANY, _("Degrees"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText15->Wrap( -1 );
	gSizer2->Add( m_staticText15, 0, wxALL, 5 );
	
	m_cbInclination = new wxCheckBox( this, wxID_ANY, _("Inclination"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer2->Add( m_cbInclination, 0, wxALL, 5 );
	
	m_scInclinationSpacing = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 90, 10 );
	gSizer2->Add( m_scInclinationSpacing, 0, wxALL, 5 );
	
	m_staticText18 = new wxStaticText( this, wxID_ANY, _("Degrees"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText18->Wrap( -1 );
	gSizer2->Add( m_staticText18, 0, wxALL, 5 );
	
	m_cbFieldStrength = new wxCheckBox( this, wxID_ANY, _("Field Strength"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer2->Add( m_cbFieldStrength, 0, wxALL, 5 );
	
	m_scFieldStrengthSpacing = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 100, 100000, 10000 );
	gSizer2->Add( m_scFieldStrengthSpacing, 0, wxALL, 5 );
	
	m_staticText19 = new wxStaticText( this, wxID_ANY, _("Nano Tesla"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText19->Wrap( -1 );
	gSizer2->Add( m_staticText19, 0, wxALL, 5 );
	
	///m_dpDate = new wxDatePickerCtrl( this, wxID_ANY, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT );
	///gSizer2->Add( m_dpDate, 0, wxALL, 5 );
	
	m_sStep = new wxSlider( this, wxID_ANY, 5, 1, 8, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	gSizer2->Add( m_sStep, 0, wxALL|wxEXPAND|wxTOP, 5 );
	
	m_staticText181 = new wxStaticText( this, wxID_ANY, _("Step Size"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText181->Wrap( -1 );
	gSizer2->Add( m_staticText181, 0, wxALL, 5 );
	
	m_staticText191 = new wxStaticText( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText191->Wrap( -1 );
	gSizer2->Add( m_staticText191, 0, wxALL, 5 );
	
	m_sPoleAccuracy = new wxSlider( this, wxID_ANY, 1, 1, 5, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	gSizer2->Add( m_sPoleAccuracy, 0, wxALL|wxEXPAND, 5 );
	
	m_staticText20 = new wxStaticText( this, wxID_ANY, _("Pole Accuracy"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText20->Wrap( -1 );
	gSizer2->Add( m_staticText20, 0, wxALL, 5 );
	
	m_bAbout = new wxButton( this, wxID_ANY, _("About"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer2->Add( m_bAbout, 0, wxALL, 5 );
	
	m_bSave = new wxButton( this, wxID_ANY, _("Save"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer2->Add( m_bSave, 0, wxALL, 5 );
	
	m_bCancel = new wxButton( this, wxID_ANY, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer2->Add( m_bCancel, 0, wxALL, 5 );
	
	
	this->SetSizer( gSizer2 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_bAbout->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( WmmPlotSettingsDialogBase::About ), NULL, this );
	m_bSave->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( WmmPlotSettingsDialogBase::Save ), NULL, this );
	m_bCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( WmmPlotSettingsDialogBase::Cancel ), NULL, this );
}

WmmPlotSettingsDialogBase::~WmmPlotSettingsDialogBase()
{
	// Disconnect Events
	m_bAbout->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( WmmPlotSettingsDialogBase::About ), NULL, this );
	m_bSave->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( WmmPlotSettingsDialogBase::Save ), NULL, this );
	m_bCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( WmmPlotSettingsDialogBase::Cancel ), NULL, this );
	
}
