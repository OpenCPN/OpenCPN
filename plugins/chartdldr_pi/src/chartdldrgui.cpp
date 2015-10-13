///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun  6 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "chartdldrgui.h"

///////////////////////////////////////////////////////////////////////////

#ifdef __OCPN__ANDROID__

QString qtStyleSheet = "QScrollBar:horizontal {\
border: 0px solid grey;\
background-color: rgb(240, 240, 240);\
height: 35px;\
margin: 0px 1px 0 1px;\
}\
QScrollBar::handle:horizontal {\
background-color: rgb(200, 200, 200);\
min-width: 20px;\
border-radius: 10px;\
}\
QScrollBar::add-line:horizontal {\
border: 0px solid grey;\
background: #32CC99;\
width: 0px;\
subcontrol-position: right;\
subcontrol-origin: margin;\
}\
QScrollBar::sub-line:horizontal {\
border: 0px solid grey;\
background: #32CC99;\
width: 0px;\
subcontrol-position: left;\
subcontrol-origin: margin;\
}\
QScrollBar:vertical {\
border: 0px solid grey;\
background-color: rgb(240, 240, 240);\
width: 35px;\
margin: 1px 0px 1px 0px;\
}\
QScrollBar::handle:vertical {\
background-color: rgb(200, 200, 200);\
min-height: 20px;\
border-radius: 10px;\
}\
QScrollBar::add-line:vertical {\
border: 0px solid grey;\
background: #32CC99;\
height: 0px;\
subcontrol-position: top;\
subcontrol-origin: margin;\
}\
QScrollBar::sub-line:vertical {\
border: 0px solid grey;\
background: #32CC99;\
height: 0px;\
subcontrol-position: bottom;\
subcontrol-origin: margin;\
}\
QCheckBox {\
spacing: 25px;\
}\
QCheckBox::indicator {\
width: 30px;\
height: 30px;\
}\
";

#endif

#ifdef __OCPN__ANDROID__
#include <QtWidgets/QScroller>
#endif

void AddSourceDlg::applyStyle()
{
    #ifdef __OCPN__ANDROID__
    m_panelPredefined->GetHandle()->setStyleSheet( qtStyleSheet);
    QScroller::ungrabGesture(m_panelPredefined->GetHandle());
    QScroller::ungrabGesture(m_treeCtrlPredefSrcs->GetHandle());
    #endif
}

AddSourceDlg::AddSourceDlg( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxSize( 500,-1 ), wxDefaultSize );

	wxBoxSizer* bSizerMain = new wxBoxSizer( wxVERTICAL );
        this->SetSizer( bSizerMain );

	wxStaticBoxSizer* sbSizerSourceSel = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Catalog") ), wxVERTICAL );
        bSizerMain->Add( sbSizerSourceSel, 1, wxALL|wxEXPAND, 5 );

	m_nbChoice = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP );
        sbSizerSourceSel->Add( m_nbChoice, 1, wxEXPAND | wxALL, 5 );
        m_nbChoice->SetMinSize( wxSize( -1,150 ) );


	m_panelPredefined = new wxPanel( m_nbChoice, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
        m_nbChoice->AddPage( m_panelPredefined, _("Predefined"), false );


	wxBoxSizer* bSizerPredefTree = new wxBoxSizer( wxVERTICAL );

        m_treeCtrlPredefSrcs = new wxTreeCtrl( m_panelPredefined, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT );
	bSizerPredefTree->Add( m_treeCtrlPredefSrcs, 1, wxALL|wxEXPAND, 5 );


	m_panelPredefined->SetSizer( bSizerPredefTree );
        m_panelPredefined->Layout();
        bSizerPredefTree->Fit( m_treeCtrlPredefSrcs/*m_panelPredefined*/ );


	m_panelCustom = new wxPanel( m_nbChoice, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxFlexGridSizer* fgSizerSourceSel;
	fgSizerSourceSel = new wxFlexGridSizer( 0, 2, 0, 0 );
	fgSizerSourceSel->AddGrowableCol( 1 );
	fgSizerSourceSel->SetFlexibleDirection( wxBOTH );
	fgSizerSourceSel->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	m_stName = new wxStaticText( m_panelCustom, wxID_ANY, _("Name"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stName->Wrap( -1 );
	fgSizerSourceSel->Add( m_stName, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_tSourceName = new wxTextCtrl( m_panelCustom, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_tSourceName->SetMaxLength( 0 );
	fgSizerSourceSel->Add( m_tSourceName, 0, wxALL|wxEXPAND, 5 );

	m_stUrl = new wxStaticText( m_panelCustom, wxID_ANY, _("URL"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stUrl->Wrap( -1 );
	fgSizerSourceSel->Add( m_stUrl, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

	m_tChartSourceUrl = new wxTextCtrl( m_panelCustom, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	m_tChartSourceUrl->SetMaxLength( 0 );
	fgSizerSourceSel->Add( m_tChartSourceUrl, 0, wxALL|wxEXPAND, 5 );


	m_panelCustom->SetSizer( fgSizerSourceSel );
	m_panelCustom->Layout();
	fgSizerSourceSel->Fit( m_panelCustom );
	m_nbChoice->AddPage( m_panelCustom, _("Custom"), true );


	wxStaticBoxSizer* sbSizerChartDir;
	sbSizerChartDir = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Chart Directory") ), wxVERTICAL );

        wxBoxSizer *dirbox = new wxBoxSizer(wxHORIZONTAL);
        sbSizerChartDir->Add(dirbox);

        m_tcChartDirectory = new wxTextCtrl(this, wxID_ANY,_T(""), wxDefaultPosition,
                                            wxSize(200, -1));
        dirbox->Add( m_tcChartDirectory, 3, wxALL|wxEXPAND, 5 );

        m_buttonChartDirectory = new wxButton(this, wxID_ANY,_("Select a folder"));
        dirbox->Add( m_buttonChartDirectory, 1, wxALL|wxEXPAND, 5 );


	bSizerMain->Add( sbSizerChartDir, 0, wxALL|wxEXPAND, 5 );

	m_sdbSizerBtns = new wxStdDialogButtonSizer();
	m_sdbSizerBtnsOK = new wxButton( this, wxID_OK );
	m_sdbSizerBtns->AddButton( m_sdbSizerBtnsOK );
	m_sdbSizerBtnsCancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizerBtns->AddButton( m_sdbSizerBtnsCancel );
	m_sdbSizerBtns->Realize();

	bSizerMain->Add( m_sdbSizerBtns, 0, wxALL|wxEXPAND, 5 );


	this->Layout();


	// Connect Events
	m_treeCtrlPredefSrcs->Connect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( AddSourceDlg::OnSourceSelected ), NULL, this );
	m_sdbSizerBtnsOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( AddSourceDlg::OnOkClick ), NULL, this );
        m_sdbSizerBtnsCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( AddSourceDlg::OnCancelClick ), NULL, this );
        m_buttonChartDirectory->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( AddSourceDlg::OnDirSelClick ), NULL, this );
}

AddSourceDlg::~AddSourceDlg()
{
	// Disconnect Events
	m_treeCtrlPredefSrcs->Disconnect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( AddSourceDlg::OnSourceSelected ), NULL, this );
	m_sdbSizerBtnsOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( AddSourceDlg::OnOkClick ), NULL, this );
        m_sdbSizerBtnsCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( AddSourceDlg::OnCancelClick ), NULL, this );
        m_buttonChartDirectory->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( AddSourceDlg::OnDirSelClick ), NULL, this );

}

void AddSourceDlg::OnDirSelClick( wxCommandEvent& event )
{
    wxString dir_spec;
    int response = PlatformDirSelectorDialog( this, &dir_spec, _("Choose Chart File Directory"), m_tcChartDirectory->GetValue());

    if( response == wxID_OK ) {
        m_tcChartDirectory->SetValue(dir_spec);
    }
}

#ifdef __OCPN__ANDROID__
ChartDldrPanel::ChartDldrPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{

        int border_size = 2;

        //   Main Sizer
        wxBoxSizer* mainSizer = new wxBoxSizer( wxVERTICAL );
        SetSizer( mainSizer );


        wxFlexGridSizer *groupsSizer = new wxFlexGridSizer( 5, 2, border_size, border_size );
        mainSizer->Add(groupsSizer, 1, wxEXPAND);

        groupsSizer->AddGrowableCol( 0 );
        groupsSizer->AddGrowableRow( 1, 1 );
        groupsSizer->AddGrowableRow( 3, 1 );

        wxStaticText *allChartsLabel = new wxStaticText( this, wxID_ANY, _("Catalogs") );
        groupsSizer->Add( allChartsLabel, 0, wxTOP | wxRIGHT | wxLEFT, border_size );

        wxStaticText *dummy1 = new wxStaticText( this, -1, _T("") );
        groupsSizer->Add( dummy1 );

        //   Sources list box
        wxBoxSizer* activeListSizer = new wxBoxSizer( wxVERTICAL );
        groupsSizer->Add( activeListSizer, 0, wxALL | wxEXPAND, 5 );

        m_lbChartSources = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_NO_SORT_HEADER|wxLC_REPORT|wxLC_SINGLE_SEL );
        m_lbChartSources->SetMinSize( wxSize( 100,100 ) );


        activeListSizer->Add( m_lbChartSources, 1, wxEXPAND );


        //  Buttons

	wxBoxSizer* bSizerCatalogBtns;
	bSizerCatalogBtns = new wxBoxSizer( wxVERTICAL );
        groupsSizer->Add( bSizerCatalogBtns, 0, wxALL | wxEXPAND, border_size );

	m_bAddSource = new wxButton( this, wxID_ANY, _("Add"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
	m_bAddSource->SetToolTip( _("Add a new chart catalog.") );

	bSizerCatalogBtns->Add( m_bAddSource, 0, wxALL|wxEXPAND, 5 );

	m_bDeleteSource = new wxButton( this, wxID_ANY, _("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
	m_bDeleteSource->SetToolTip( _("Delete the chart catalog. The downloaded charts are not deleted.") );

	bSizerCatalogBtns->Add( m_bDeleteSource, 0, wxALL|wxEXPAND, 5 );

	m_bEditSource = new wxButton( this, wxID_ANY, _("Edit..."), wxDefaultPosition, wxDefaultSize, 0 );
	bSizerCatalogBtns->Add( m_bEditSource, 0, wxALL|wxEXPAND, 5 );

	m_bUpdateChartList = new wxButton( this, wxID_ANY, _("Update"), wxDefaultPosition, wxDefaultSize, 0 );
	m_bUpdateChartList->SetDefault();
	m_bUpdateChartList->SetToolTip( _("Download the current chart catalog and update the data.") );

	bSizerCatalogBtns->Add( m_bUpdateChartList, 0, wxALL|wxEXPAND, 5 );

	m_bUpdateAllCharts = new wxButton( this, wxID_ANY, _("Update All"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizerCatalogBtns->Add( m_bUpdateAllCharts, 0, wxALIGN_BOTTOM|wxALL|wxEXPAND, 5 );


        //  Charts
        wxStaticText *groupsLabel = new wxStaticText( this, wxID_ANY, _("Charts") );
        groupsSizer->Add( groupsLabel, 0, wxTOP | wxRIGHT | wxLEFT, border_size );

        wxStaticText *dummy2 = new wxStaticText( this, -1, _T("") );
        groupsSizer->Add( dummy2 );

        wxBoxSizer* nbSizer = new wxBoxSizer( wxVERTICAL );
        groupsSizer->Add( nbSizer, 0, wxALL | wxEXPAND, border_size );

	m_clCharts = new wxCheckedListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);


        m_clCharts->SetMinSize( wxSize( 100,100 ) );
        nbSizer->Add( m_clCharts, 1, wxEXPAND );


        //  Buttons

        wxBoxSizer* bSizerBtns = new wxBoxSizer( wxVERTICAL );
        groupsSizer->Add( bSizerBtns, 0, wxALL, border_size );

        m_bHelp = new wxButton( this, wxID_ANY, _("Help"), wxDefaultPosition, wxDefaultSize, 0 );
        bSizerBtns->Add( m_bHelp, 0, wxALL, 5 );

        m_bDnldCharts = new wxButton( this, wxID_ANY, _("Download\n selected charts"), wxDefaultPosition, wxDefaultSize, 0 );
        bSizerBtns->Add( m_bDnldCharts, 1, wxALIGN_CENTER|wxALL, 5 );

        m_bShowLocal = new wxButton( this, wxID_ANY, _("Show local files"), wxDefaultPosition, wxDefaultSize, 0 );
        bSizerBtns->Add( m_bShowLocal, 0, wxALL, 5 );
       // m_bShowLocal->Hide();



	m_stCatalogInfo = new wxStaticText( this, wxID_ANY, _("%u charts total, %u updated, %u new"), wxDefaultPosition, wxDefaultSize, 0 );
	groupsSizer->Add( m_stCatalogInfo, 1, wxEXPAND| wxALL, 5 );
        mainSizer->Add( m_stCatalogInfo, 0, wxEXPAND| wxALL, 5 );


        this->Layout();

        m_lbChartSources->GetHandle()->setStyleSheet( qtStyleSheet);
        m_clCharts->GetHandle()->setStyleSheet( qtStyleSheet);


	// Connect Events
	this->Connect( wxEVT_PAINT, wxPaintEventHandler( ChartDldrPanel::OnPaint ) );
	m_lbChartSources->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( ChartDldrPanel::OnLeftDClick ), NULL, this );
	m_lbChartSources->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( ChartDldrPanel::SelectSource ), NULL, this );
	m_bAddSource->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::AddSource ), NULL, this );
	m_bDeleteSource->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::DeleteSource ), NULL, this );
	m_bEditSource->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::EditSource ), NULL, this );
	m_bUpdateChartList->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::UpdateChartList ), NULL, this );
	m_bUpdateAllCharts->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::UpdateAllCharts ), NULL, this );
	m_clCharts->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( ChartDldrPanel::OnContextMenu ), NULL, this );
	m_bHelp->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::DoHelp ), NULL, this );
	m_bDnldCharts->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::OnDownloadCharts ), NULL, this );
	m_bShowLocal->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::OnShowLocalDir ), NULL, this );
}

#else
ChartDldrPanel::ChartDldrPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
        wxBoxSizer* bSizerChartDlg;
        bSizerChartDlg = new wxBoxSizer( wxVERTICAL );

        wxStaticBoxSizer* sbSchartsource;
        sbSchartsource = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Catalogs") ), wxVERTICAL );

        wxBoxSizer* bSizerCatalogs;
        bSizerCatalogs = new wxBoxSizer( wxHORIZONTAL );

        m_lbChartSources = new wxListCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_NO_SORT_HEADER|wxLC_REPORT|wxLC_SINGLE_SEL );
        bSizerCatalogs->Add( m_lbChartSources, 1, wxALL|wxEXPAND, 5 );

        wxBoxSizer* bSizerCatalogBtns;
        bSizerCatalogBtns = new wxBoxSizer( wxVERTICAL );

        m_bAddSource = new wxButton( this, wxID_ANY, _("Add"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
        m_bAddSource->SetToolTip( _("Add a new chart catalog.") );

        bSizerCatalogBtns->Add( m_bAddSource, 0, wxALL|wxEXPAND, 5 );

        m_bDeleteSource = new wxButton( this, wxID_ANY, _("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
        m_bDeleteSource->SetToolTip( _("Delete the chart catalog. The downloaded charts are not deleted.") );

        bSizerCatalogBtns->Add( m_bDeleteSource, 0, wxALL|wxEXPAND, 5 );

        m_bEditSource = new wxButton( this, wxID_ANY, _("Edit..."), wxDefaultPosition, wxDefaultSize, 0 );
        bSizerCatalogBtns->Add( m_bEditSource, 0, wxALL|wxEXPAND, 5 );

        m_bUpdateChartList = new wxButton( this, wxID_ANY, _("Update"), wxDefaultPosition, wxDefaultSize, 0 );
        m_bUpdateChartList->SetDefault();
        m_bUpdateChartList->SetToolTip( _("Download the current chart catalog and update the data.") );

        bSizerCatalogBtns->Add( m_bUpdateChartList, 0, wxALL|wxEXPAND, 5 );

        m_bUpdateAllCharts = new wxButton( this, wxID_ANY, _("Update All"), wxDefaultPosition, wxDefaultSize, 0 );
        bSizerCatalogBtns->Add( m_bUpdateAllCharts, 0, wxALIGN_BOTTOM|wxALL|wxEXPAND, 5 );


        bSizerCatalogs->Add( bSizerCatalogBtns, 0, wxEXPAND, 5 );


        sbSchartsource->Add( bSizerCatalogs, 1, wxEXPAND, 0 );


        bSizerChartDlg->Add( sbSchartsource, 0, wxALL|wxEXPAND, 5 );

        wxStaticBoxSizer* sbSCharts;
        sbSCharts = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Charts") ), wxVERTICAL );

        m_clCharts = new wxCheckedListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
        //m_clCharts>SetImagesForList(wxLC_REPORT|wxLC_SINGLE_SEL);
        //m_clCharts>UpdateStyle();
        m_clCharts->SetMinSize( wxSize( -1,100 ) );

        sbSCharts->Add( m_clCharts, 1, wxALL|wxEXPAND, 5 );

        m_stCatalogInfo = new wxStaticText( this, wxID_ANY, _("%u charts total, %u updated, %u new"), wxDefaultPosition, wxDefaultSize, 0 );
        m_stCatalogInfo->Wrap( -1 );
        sbSCharts->Add( m_stCatalogInfo, 0, wxALL, 5 );

        wxBoxSizer* bSizerBtns;
        bSizerBtns = new wxBoxSizer( wxHORIZONTAL );

        m_bHelp = new wxButton( this, wxID_ANY, _("Help"), wxDefaultPosition, wxDefaultSize, 0 );
        bSizerBtns->Add( m_bHelp, 0, wxALL, 5 );


        bSizerBtns->Add( 0, 0, 1, wxEXPAND, 5 );

        m_bDnldCharts = new wxButton( this, wxID_ANY, _("Download selected charts"), wxDefaultPosition, wxDefaultSize, 0 );
        bSizerBtns->Add( m_bDnldCharts, 1, wxALIGN_CENTER|wxALL, 5 );

        m_bShowLocal = new wxButton( this, wxID_ANY, _("Show local files"), wxDefaultPosition, wxDefaultSize, 0 );
        bSizerBtns->Add( m_bShowLocal, 0, wxALL, 5 );


        sbSCharts->Add( bSizerBtns, 0, wxEXPAND, 5 );


        bSizerChartDlg->Add( sbSCharts, 1, wxALL|wxEXPAND, 5 );


        this->SetSizer( bSizerChartDlg );
        this->Layout();
        bSizerChartDlg->Fit( this );

        // Connect Events
        this->Connect( wxEVT_PAINT, wxPaintEventHandler( ChartDldrPanel::OnPaint ) );
        m_lbChartSources->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( ChartDldrPanel::OnLeftDClick ), NULL, this );
        m_lbChartSources->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( ChartDldrPanel::SelectSource ), NULL, this );
        m_bAddSource->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::AddSource ), NULL, this );
        m_bDeleteSource->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::DeleteSource ), NULL, this );
        m_bEditSource->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::EditSource ), NULL, this );
        m_bUpdateChartList->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::UpdateChartList ), NULL, this );
        m_bUpdateAllCharts->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::UpdateAllCharts ), NULL, this );
        m_clCharts->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( ChartDldrPanel::OnContextMenu ), NULL, this );
        m_bHelp->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::DoHelp ), NULL, this );
        m_bDnldCharts->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::OnDownloadCharts ), NULL, this );
        m_bShowLocal->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::OnShowLocalDir ), NULL, this );
}
#endif


ChartDldrPanel::~ChartDldrPanel()
{
	// Disconnect Events
	this->Disconnect( wxEVT_PAINT, wxPaintEventHandler( ChartDldrPanel::OnPaint ) );
	m_lbChartSources->Disconnect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( ChartDldrPanel::OnLeftDClick ), NULL, this );
	m_lbChartSources->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( ChartDldrPanel::SelectSource ), NULL, this );
	m_bAddSource->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::AddSource ), NULL, this );
	m_bDeleteSource->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::DeleteSource ), NULL, this );
	m_bEditSource->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::EditSource ), NULL, this );
	m_bUpdateChartList->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::UpdateChartList ), NULL, this );
	m_bUpdateAllCharts->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::UpdateAllCharts ), NULL, this );
	m_clCharts->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( ChartDldrPanel::OnContextMenu ), NULL, this );
	m_bHelp->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::DoHelp ), NULL, this );
	m_bDnldCharts->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::OnDownloadCharts ), NULL, this );
	m_bShowLocal->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::OnShowLocalDir ), NULL, this );

}

ChartDldrPrefsDlg::ChartDldrPrefsDlg( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
        wxFont *pFont = OCPNGetFont(_T("Dialog"), 0);
        if( pFont ) SetFont( *pFont );
                                   
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );

	wxBoxSizer* bSizerPrefsMain;
	bSizerPrefsMain = new wxBoxSizer( wxVERTICAL );

	wxStaticBoxSizer* sbSizerPaths;
	sbSizerPaths = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Default Path to Charts") ), wxVERTICAL );

//	m_dpDefaultDir = new wxDirPickerCtrl( this, wxID_ANY, wxEmptyString, _("Select a root folder for your charts"), wxDefaultPosition, wxDefaultSize, wxDIRP_USE_TEXTCTRL );
//	sbSizerPaths->Add( m_dpDefaultDir, 0, wxALL|wxEXPAND, 5 );


        m_tcDefaultDir = new wxTextCtrl(this, wxID_ANY,_T(""), wxDefaultPosition, wxSize(-1, -1), wxHSCROLL );
        sbSizerPaths->Add( m_tcDefaultDir, 3, wxALL|wxEXPAND, 5 );

        m_buttonChartDirectory = new wxButton(this, wxID_ANY,_("Select a folder"));
        sbSizerPaths->Add( m_buttonChartDirectory, 1, wxALIGN_RIGHT|wxALL, 5 );


	bSizerPrefsMain->Add( sbSizerPaths, 0, wxALL|wxEXPAND, 5 );

	wxStaticBoxSizer* sbSizerBehavior;
	sbSizerBehavior = new wxStaticBoxSizer( new wxStaticBox( this, wxID_ANY, _("Preferences") ), wxVERTICAL );

        sbSizerBehavior->SetMinSize(-1, 200);

	m_stPreselect = new wxStaticText( this, wxID_ANY, _("After catalog update select for download"), wxDefaultPosition, wxDefaultSize, 0 );
	m_stPreselect->Wrap( -1 );
	sbSizerBehavior->Add( m_stPreselect, 0, wxALL, 5 );

	m_cbSelectUpdated = new wxCheckBox( this, wxID_ANY, _("All updated charts"), wxDefaultPosition, wxDefaultSize, 0 );
	m_cbSelectUpdated->SetValue(true);
	sbSizerBehavior->Add( m_cbSelectUpdated, 0, wxALL, 5 );

	m_cbSelectNew = new wxCheckBox( this, wxID_ANY, _("All new charts"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerBehavior->Add( m_cbSelectNew, 0, wxALL, 5 );

	m_staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	sbSizerBehavior->Add( m_staticline1, 0, wxEXPAND | wxALL, 5 );

	m_cbBulkUpdate = new wxCheckBox( this, wxID_ANY, _("Allow bulk update of all configured chart sources and charts"), wxDefaultPosition, wxDefaultSize, 0 );
	sbSizerBehavior->Add( m_cbBulkUpdate, 0, wxALL, 5 );


	bSizerPrefsMain->Add( sbSizerBehavior, 1, wxALL|wxEXPAND, 5 );

	m_sdbSizerBtns = new wxStdDialogButtonSizer();
	m_sdbSizerBtnsOK = new wxButton( this, wxID_OK );
	m_sdbSizerBtns->AddButton( m_sdbSizerBtnsOK );
	m_sdbSizerBtnsCancel = new wxButton( this, wxID_CANCEL );
	m_sdbSizerBtns->AddButton( m_sdbSizerBtnsCancel );
	m_sdbSizerBtns->Realize();

	bSizerPrefsMain->Add( m_sdbSizerBtns, 0, wxALL|wxEXPAND, 5 );


	this->SetSizer( bSizerPrefsMain );
	this->Layout();

        this->Fit();
	this->Centre( wxBOTH );

	// Connect Events
	m_sdbSizerBtnsCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPrefsDlg::OnCancelClick ), NULL, this );
	m_sdbSizerBtnsOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPrefsDlg::OnOkClick ), NULL, this );
        m_buttonChartDirectory->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPrefsDlg::OnDirSelClick ), NULL, this );

}

void ChartDldrPrefsDlg::OnDirSelClick( wxCommandEvent& event )
{
    wxString dir_spec;
    int response = PlatformDirSelectorDialog( this, &dir_spec, _("Choose Chart File Directory"), m_tcDefaultDir->GetValue());

    if( response == wxID_OK ) {
        m_tcDefaultDir->SetValue(dir_spec);
    }
}


ChartDldrPrefsDlg::~ChartDldrPrefsDlg()
{
	// Disconnect Events
	m_sdbSizerBtnsCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPrefsDlg::OnCancelClick ), NULL, this );
	m_sdbSizerBtnsOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPrefsDlg::OnOkClick ), NULL, this );
        m_buttonChartDirectory->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPrefsDlg::OnDirSelClick ), NULL, this );

}
