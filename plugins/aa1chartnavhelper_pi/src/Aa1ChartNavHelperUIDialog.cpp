///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct  8 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "Aa1ChartNavHelperUIDialog.h"

///////////////////////////////////////////////////////////////////////////

Aa1ChartNavHelperUIDialogBase::Aa1ChartNavHelperUIDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( -1,-1 ), wxSize( -1,-1 ) );
	
	wxFlexGridSizer* fgSizer6;
	fgSizer6 = new wxFlexGridSizer( 0, 1, 0, 0 );
	fgSizer6->AddGrowableCol( 0 );
	fgSizer6->AddGrowableRow( 0 );
	fgSizer6->SetFlexibleDirection( wxBOTH );
	fgSizer6->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
	bSframe = new wxBoxSizer( wxVERTICAL );
	
	bSframe->SetMinSize( wxSize( 200,-1 ) ); 
		
	wxFlexGridSizer* gSizer4;
	gSizer4 = new wxFlexGridSizer( 2, 2, 0, 0 );
	gSizer4->AddGrowableCol( 1 );
	gSizer4->SetFlexibleDirection( wxBOTH );
	gSizer4->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
	
    
	
	fgSizer6->Add( bSframe, 1, wxEXPAND, 5 );
	
    sbPlot = new wxStaticBoxSizer(  wxVERTICAL, this, _("Chart Info") );
    
    m_cbEnableCompass = new wxCheckBox( this, wxID_ANY, wxT("Enable Compass"), wxDefaultPosition, wxDefaultSize, 0 );
        m_cbEnableCompass->SetMinSize( wxSize( 200,-1 ) );
        sbPlot->Add( m_cbEnableCompass, 1, wxALL, 5 );
	
	m_bPlotSettings = new wxButton( this, wxID_ANY, _("Settings..."), wxDefaultPosition, wxDefaultSize, 0 );
        m_bPlotSettings->SetMinSize( wxSize( 200,-1 ) );
        sbPlot->Add( m_bPlotSettings, 1, wxALL, 5 );
	
        fgSizer6->Add( sbPlot, 1, wxEXPAND/*|wxFIXED_MINSIZE*/, 0 );
    
    wxStdDialogButtonSizer *m_sdbSizerBtns = new wxStdDialogButtonSizer();
    wxButton *m_sdbSizerBtnsOK = new wxButton( this, wxID_OK );
    m_sdbSizerBtns->AddButton( m_sdbSizerBtnsOK );
    m_sdbSizerBtns->Realize();
    
    fgSizer6->Add( m_sdbSizerBtns, 0, wxALL|wxEXPAND, 5 );
	
	this->SetSizer( fgSizer6 );
	this->Layout();
	
	this->Centre( wxBOTH );
	
	// Connect Events
	m_bPlotSettings->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Aa1ChartNavHelperUIDialogBase::PlotSettings ), NULL, this );
	m_cbEnableCompass->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( Aa1ChartNavHelperUIDialogBase::EnableCompassChanged ), NULL, this );

}

Aa1ChartNavHelperUIDialogBase::~Aa1ChartNavHelperUIDialogBase()
{
	// Disconnect Events
	m_bPlotSettings->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Aa1ChartNavHelperUIDialogBase::PlotSettings ), NULL, this );
	m_cbEnableCompass->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( Aa1ChartNavHelperUIDialogBase::EnableCompassChanged ), NULL, this );
	
}

Aa1ChartNavHelperPrefsDialog::Aa1ChartNavHelperPrefsDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer( wxVERTICAL );

    
    m_cbShowToolbarIcon = new wxCheckBox( this, wxID_ANY, _("Show toolbar icon"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer2->Add( m_cbShowToolbarIcon, 0, wxALL, 5 );
    
    m_cbShowLiveToolbarIcon = new wxCheckBox( this, wxID_ANY, _("Show data in toolbar icon"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer2->Add( m_cbShowLiveToolbarIcon, 0, wxALL, 5 );
    
	wxStaticBoxSizer* sbSizer4;
	sbSizer4 = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Window transparency") ), wxVERTICAL );
	
	m_sOpacity = new wxSlider( this, wxID_ANY, 255, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_INVERSE );
	sbSizer4->Add( m_sOpacity, 0, wxBOTTOM|wxEXPAND|wxTOP, 5 );
	
	
	bSizer2->Add( sbSizer4, 1, wxALL|wxEXPAND, 5 );
	
	m_sdbSizer1 = new wxStdDialogButtonSizer();
	m_sdbSizer1OK = new wxButton( this, wxID_OK );
	m_sdbSizer1->AddButton( m_sdbSizer1OK );
	m_sdbSizer1Cancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizer1->AddButton( m_sdbSizer1Cancel );
	m_sdbSizer1->Realize();
	
	bSizer2->Add( m_sdbSizer1, 0, wxBOTTOM|wxEXPAND|wxTOP, 5 );
	
	
	this->SetSizer( bSizer2 );
	this->Layout();
	bSizer2->Fit( this );
	
	this->Centre( wxBOTH );
}

Aa1ChartNavHelperPrefsDialog::~Aa1ChartNavHelperPrefsDialog()
{
}

Aa1ChartNavHelperPlotSettingsDialogBase::Aa1ChartNavHelperPlotSettingsDialogBase( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxGridSizer* gSizer2;
	gSizer2 = new wxGridSizer( 0, 3, 0, 0 );
	
	
	m_staticTextValYear = new wxStaticText( this, wxID_ANY, wxT("Val Year"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextValYear->Wrap( -1 );
	gSizer2->Add( m_staticTextValYear, 0, wxALL, 5 );

	m_scCtrlValYearSpacing = new wxSpinCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1900, 3000, 2002);
	gSizer2->Add( m_scCtrlValYearSpacing, 0, wxALL, 5 );

	m_staticTextValYear = new wxStaticText( this, wxID_ANY, wxT(" "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextValYear->Wrap( -1 );
	gSizer2->Add( m_staticTextValYear, 0, wxALL, 5 );

	m_checkBoxUseDefaultVar = new wxCheckBox( this, wxID_ANY, wxT("Use Default Var"), wxDefaultPosition, wxDefaultSize, 0 );
	gSizer2->Add( m_checkBoxUseDefaultVar, 0, wxALL, 5 );

	m_textDefaultVar = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	gSizer2->Add( m_textDefaultVar, 0, wxALL, 5 );

	m_staticTextDefaultVar1 = new wxStaticText( this, wxID_ANY, wxT(" "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextDefaultVar1->Wrap( -1 );
	gSizer2->Add( m_staticTextDefaultVar1, 0, wxALL, 5 );

	m_staticTextDefaultVar2 = new wxStaticText( this, wxID_ANY, wxT("   Year Delta"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextDefaultVar2->Wrap( -1 );
	gSizer2->Add( m_staticTextDefaultVar2, 0, wxALL, 5 );

	m_textDefaultVarYearDelta = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	gSizer2->Add( m_textDefaultVarYearDelta, 0, wxALL, 5 );

	m_staticTextDefaultVar3 = new wxStaticText( this, wxID_ANY, wxT(" "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticTextDefaultVar3->Wrap( -1 );
	gSizer2->Add( m_staticTextDefaultVar3, 0, wxALL, 5 );

	
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
	m_bAbout->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Aa1ChartNavHelperPlotSettingsDialogBase::About ), NULL, this );
	m_bSave->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Aa1ChartNavHelperPlotSettingsDialogBase::Save ), NULL, this );
	m_bCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Aa1ChartNavHelperPlotSettingsDialogBase::Cancel ), NULL, this );
}

Aa1ChartNavHelperPlotSettingsDialogBase::~Aa1ChartNavHelperPlotSettingsDialogBase()
{
	// Disconnect Events
	m_bAbout->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Aa1ChartNavHelperPlotSettingsDialogBase::About ), NULL, this );
	m_bSave->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Aa1ChartNavHelperPlotSettingsDialogBase::Save ), NULL, this );
	m_bCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( Aa1ChartNavHelperPlotSettingsDialogBase::Cancel ), NULL, this );
	
}
