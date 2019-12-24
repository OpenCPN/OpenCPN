///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun  6 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "chartdldr_pi.h"
#include "chartdldrgui.h"
#include <wx/msgdlg.h>
#include <wx/scrolwin.h>

///////////////////////////////////////////////////////////////////////////

#ifdef __OCPN__ANDROID__

QString qtStyleSheet = "QScrollBar:horizontal {\
border: 0px solid grey;\
background-color: rgb(240, 240, 240);\
height: 50px;\
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
width: 50px;\
margin: 1px 0px 1px 0px;\
}\
QScrollBar::handle:vertical {\
background-color: rgb(200, 200, 200);\
min-height: 50px;\
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
#include "qdebug.h"
#endif

void AddSourceDlg::applyStyle()
{
    #ifdef __OCPN__ANDROID__
    m_panelPredefined->GetHandle()->setStyleSheet( qtStyleSheet);
    QScroller::ungrabGesture(m_panelPredefined->GetHandle());
///    QScroller::ungrabGesture(m_treeCtrlPredefSrcs->GetHandle());
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

        m_treeCtrlPredefSrcs = new wxTreeCtrl( m_panelPredefined, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT|wxTR_HAS_BUTTONS );
	bSizerPredefTree->Add( m_treeCtrlPredefSrcs, 1, wxALL|wxEXPAND, 5 );
#ifdef __OCPN__ANDROID__
        m_treeCtrlPredefSrcs->SetScrollRate(0,1);
#endif
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
	m_sdbSizerBtnsCancel = new wxButton( this, wxID_CANCEL, _("Cancel") );
	m_sdbSizerBtns->AddButton( m_sdbSizerBtnsCancel );
	m_sdbSizerBtns->Realize();

	bSizerMain->Add( m_sdbSizerBtns, 0, wxALL|wxEXPAND, 5 );


	this->Layout();

    m_sourceswin = NULL;

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


#if 1

ChartDldrPanel::ChartDldrPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
    
    int border_size = 2;
    
    //   Main Sizer
    wxBoxSizer* mainSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( mainSizer );
    
    mainSizer->AddSpacer(GetCharHeight());
    
    m_DLoadNB = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP);
    mainSizer->Add(m_DLoadNB, 1, wxEXPAND);
    
    // Catalogs
    wxPanel* catalogPanel = new wxPanel(m_DLoadNB, wxID_ANY, wxDefaultPosition, wxDefaultSize );
    m_DLoadNB->AddPage(catalogPanel, _("Select Catalog..."));
    
    wxBoxSizer* catalogPanelBoxSizer = new wxBoxSizer(wxVERTICAL);
    catalogPanel->SetSizer(catalogPanelBoxSizer);
    
    wxStaticText *catalogLabel = new wxStaticText( catalogPanel, wxID_ANY, _("Chart Catalogs Available") );
    catalogLabel->Wrap( -1 );
    catalogPanelBoxSizer->Add( catalogLabel, 0, wxTOP | wxRIGHT | wxLEFT, border_size );
    
    //   Sources list box
    wxBoxSizer* activeListSizer = new wxBoxSizer( wxVERTICAL );
    catalogPanelBoxSizer->Add( activeListSizer, 0, wxALL | wxEXPAND, 5 );
    
    m_lbChartSources = new wxListCtrl( catalogPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_NO_SORT_HEADER|wxLC_REPORT|wxLC_SINGLE_SEL );
    activeListSizer->Add( m_lbChartSources, 1, wxEXPAND | wxRIGHT, 60 );
    ///m_lbChartSources->SetMinSize( wxSize( -1, 10 * GetCharHeight() ) );
    
    
    
    //  Buttons
    
    wxBoxSizer* bSizerCatalogBtns = new wxBoxSizer( wxHORIZONTAL );
    catalogPanelBoxSizer->Add( bSizerCatalogBtns, 0, wxALL | wxEXPAND, border_size );
    
    m_bAddSource = new wxButton( catalogPanel, wxID_ANY, _("Add Catalog"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    m_bAddSource->SetToolTip( _("Add a new chart catalog.") );
    
    bSizerCatalogBtns->Add( m_bAddSource, 0, wxALL|wxEXPAND, 5 );
    
    m_bDeleteSource = new wxButton( catalogPanel, wxID_ANY, _("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
    m_bDeleteSource->SetToolTip( _("Delete the chart catalog. The downloaded charts are not deleted.") );
    
    bSizerCatalogBtns->Add( m_bDeleteSource, 0, wxALL|wxEXPAND, 5 );
    
    m_bEditSource = new wxButton( catalogPanel, wxID_ANY, _("Edit..."), wxDefaultPosition, wxDefaultSize, 0 );
    bSizerCatalogBtns->Add( m_bEditSource, 0, wxALL|wxEXPAND, 5 );
    
    m_bUpdateChartList = new wxButton( catalogPanel, wxID_ANY, _("Update"), wxDefaultPosition, wxDefaultSize, 0 );
    m_bUpdateChartList->SetDefault();
    m_bUpdateChartList->SetToolTip( _("Download the current chart catalog and update the data.") );
    
    bSizerCatalogBtns->Add( m_bUpdateChartList, 0, wxALL|wxEXPAND, 5 );
    
    m_bUpdateAllCharts = new wxButton( catalogPanel, wxID_ANY, _("Update All"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizerCatalogBtns->Add( m_bUpdateAllCharts, 0, wxALL|wxEXPAND, 5 );
    
    
    // Charts
    wxPanel* chartsPanel = new wxPanel(m_DLoadNB, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBG_STYLE_ERASE);
    m_DLoadNB->AddPage(chartsPanel, _("Download Charts..."));
    
    wxBoxSizer* chartsPanelBoxSizer = new wxBoxSizer(wxVERTICAL);
    chartsPanel->SetSizer(chartsPanelBoxSizer);

    //  Chart list
    m_chartsLabel = new wxStaticText( chartsPanel, wxID_ANY, _("Charts") );
    chartsPanelBoxSizer->Add( m_chartsLabel, 0, wxALL, 4 * border_size );
    
#ifdef NEW_LIST
    m_scrollWinChartList = new wxScrolledWindow( chartsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL);
    chartsPanelBoxSizer->Add( m_scrollWinChartList, 0, wxEXPAND );
    m_scrollWinChartList->SetScrollRate(5, 5);
    m_scrollWinChartList->SetMinSize( wxSize( -1,12 * GetCharHeight() ) );
    
    m_boxSizerCharts = new wxBoxSizer(wxVERTICAL);
    m_scrollWinChartList->SetSizer(m_boxSizerCharts);
    
#else
    m_clCharts = new wxCheckedListCtrl(chartsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL);
    m_clCharts->SetMinSize( wxSize( 100,10 * GetCharHeight() ) );
#endif    

    //  Buttons
    
    chartsPanelBoxSizer->AddSpacer( GetCharHeight());

    m_bDnldCharts = new wxButton( chartsPanel, wxID_ANY, _("Download selected charts"), wxDefaultPosition, wxDefaultSize, 0 );
    chartsPanelBoxSizer->Add( m_bDnldCharts, 0, wxALIGN_LEFT|wxALL, 5 );
    
    m_stCatalogInfo = new wxStaticText( chartsPanel, wxID_ANY, _("%u charts total, %u updated, %u new"), wxDefaultPosition, wxDefaultSize, 0 );
    chartsPanelBoxSizer->Add( m_stCatalogInfo, 1, wxEXPAND| wxALL, 5 );
    /// mainSizer->Add( m_stCatalogInfo, 0, wxEXPAND| wxALL, 5 );
    
    this->Layout();
    
    //     m_lbChartSources->GetHandle()->setStyleSheet( qtStyleSheet);
    //     m_clCharts->GetHandle()->setStyleSheet( qtStyleSheet);
    
    // Connect Events
    this->Connect( wxEVT_PAINT, wxPaintEventHandler( ChartDldrPanel::OnPaint ) );
    m_lbChartSources->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( ChartDldrPanel::OnLeftDClick ), NULL, this );
    m_lbChartSources->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( ChartDldrPanel::SelectSource ), NULL, this );
    m_bAddSource->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::AddSource ), NULL, this );
    m_bDeleteSource->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::DeleteSource ), NULL, this );
    m_bEditSource->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::EditSource ), NULL, this );
    m_bUpdateChartList->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::UpdateChartList ), NULL, this );
    m_bUpdateAllCharts->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::UpdateAllCharts ), NULL, this );
#ifdef NEW_LIST
    m_scrollWinChartList->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( ChartDldrPanel::OnContextMenu ), NULL, this );
#else
    m_clCharts->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( ChartDldrPanel::OnContextMenu ), NULL, this );
#endif    
    //m_bHelp->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::DoHelp ), NULL, this );
    m_bDnldCharts->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::OnDownloadCharts ), NULL, this );
    //m_bShowLocal->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::OnShowLocalDir ), NULL, this );
    
    this->Connect( wxEVT_SIZE, wxSizeEventHandler( ChartDldrPanel::OnSize ) );
    
}
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
#ifdef NEW_LIST
    m_scrollWinChartList->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( ChartDldrPanel::OnContextMenu ), NULL, this );
#else
    m_clCharts->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( ChartDldrPanel::OnContextMenu ), NULL, this );
#endif
    //m_bHelp->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::DoHelp ), NULL, this );
    m_bDnldCharts->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::OnDownloadCharts ), NULL, this );
    //m_bShowLocal->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPanel::OnShowLocalDir ), NULL, this );
    
    }
    

void ChartDldrPanel::OnSize( wxSizeEvent& event )
{
//     wxSize newSize = event.GetSize();
//     qDebug() << "Size: " << newSize.x << newSize.y;
//     
//     int width, height;
//     ::wxDisplaySize(&width, &height);
//     qDebug() << "wxDisplaySize: " << width << height;
    
    // We want the variable sized windows to to be small enough so that
    // no vertical scroll bar is necessary to access the control buttons.
    // Use this method to estimate the available vertical size to allow this
    wxWindow *win = GetOCPNCanvasWindow();
    wxSize sz = win->GetSize();
    //qDebug() << "CanvasSize: " << sz.x << sz.y;    
    
    if(GetGrandParent())
        sz = GetGrandParent()->GetSize();
    //qDebug() << "MetricSize: " << sz.x << sz.y;    

    int yAvail = sz.y - (8 * GetCharHeight());          // Roughly 8 chars of decoration on-screen for this dialog.
    double ratio = 0.7;
    if(sz.y > sz.x)                     // Portait mode
        ratio = 0.8;
    
    m_lbChartSources->SetMinSize( wxSize( -1, yAvail * ratio ));
#ifdef NEW_LIST
    m_scrollWinChartList->SetMinSize( wxSize( -1, yAvail * ratio ));
#endif
    Layout();
    
    event.Skip();
}

#endif


ChartPanel::ChartPanel(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, wxString Name, wxString stat, wxString latest, ChartDldrPanel *DldrPanel, bool bcheck)
:wxPanel(parent, id, pos, size, wxBORDER_NONE)
{
    wxColour bColor;
    GetGlobalColor(_T("DILG0"), &bColor);
    SetBackgroundColour( bColor );
    
    wxBoxSizer* m_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(m_sizer);
    m_cb = new wxCheckBox(this, wxID_ANY, Name);
    m_cb->SetValue(bcheck);
    m_sizer->Add(m_cb, 0, wxTOP | wxLEFT | wxRIGHT, 4);
    m_cb->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( ChartPanel::OnContextMenu ), NULL, this );
    
    m_stat = stat;
    m_latest = latest;

     wxBoxSizer* statSizer = new wxBoxSizer(wxHORIZONTAL);
     m_sizer->Add(statSizer, 0, wxALIGN_LEFT | wxALL, 1);
    
     m_chartInfo = new wxStaticText( this, wxID_ANY, stat );
     statSizer->Add(m_chartInfo, 0, wxLEFT, 4 * GetCharHeight());
     m_chartInfo->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( ChartPanel::OnContextMenu ), NULL, this );

     m_chartInfo2 = new wxStaticText( this, wxID_ANY, latest );
     statSizer->Add(m_chartInfo2, 0, wxLEFT, 2 * GetCharHeight());
     m_chartInfo2->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( ChartPanel::OnContextMenu ), NULL, this );

//     wxString info = _T("           ") + stat + _T("   ") + latest;
//     m_chartInfo = new wxStaticText( this, wxID_ANY, info );
//     m_chartInfo->Wrap(-1);
//     m_sizer->Add( m_chartInfo, 0, wxALL, 1 );
    
    wxStaticLine *divLine = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
    m_sizer->Add( divLine, 0, wxEXPAND | wxALL, 5 );
    
    m_dldrPanel = DldrPanel;
    Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( ChartPanel::OnContextMenu ), NULL, this );
    
}

ChartPanel::~ChartPanel()
{
    m_cb->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( ChartPanel::OnContextMenu ), NULL, this );
    m_chartInfo->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( ChartPanel::OnContextMenu ), NULL, this );
    m_chartInfo2->Disconnect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( ChartPanel::OnContextMenu ), NULL, this );

    delete m_cb;
    delete m_chartInfo;
    delete m_chartInfo2;
}

void ChartPanel::OnContextMenu( wxMouseEvent& event )
{
    if(m_dldrPanel)
        return m_dldrPanel->OnContextMenu( event );
    event.Skip();
    
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

//      m_dpDefaultDir = new wxDirPickerCtrl( this, wxID_ANY, wxEmptyString, _("Select a root folder for your charts"), wxDefaultPosition, wxDefaultSize, wxDIRP_USE_TEXTCTRL );
//      sbSizerPaths->Add( m_dpDefaultDir, 0, wxALL|wxEXPAND, 5 );


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


    m_buttonDownloadMasterCatalog = new wxButton(this, wxID_ANY, _("Update chart source catalog"), wxDefaultPosition, wxDefaultSize, 0);
    
    sbSizerBehavior->Add( m_buttonDownloadMasterCatalog, 0, wxALL, 5 );
    
        bSizerPrefsMain->Add( sbSizerBehavior, 1, wxALL|wxEXPAND, 5 );

        m_sdbSizerBtns = new wxStdDialogButtonSizer();
        m_sdbSizerBtnsOK = new wxButton( this, wxID_OK );
        m_sdbSizerBtns->AddButton( m_sdbSizerBtnsOK );
        m_sdbSizerBtnsCancel = new wxButton( this, wxID_CANCEL, _("Cancel") );
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
    m_buttonDownloadMasterCatalog->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPrefsDlg::OnDownloadMasterCatalog ), NULL, this );
}

void ChartDldrPrefsDlg::OnDownloadMasterCatalog( wxCommandEvent& event )
{
    wxFileName tfn = wxFileName::CreateTempFileName(_T("chartdldr"));
    wxString url = "https://raw.githubusercontent.com/OpenCPN/OpenCPN/master/plugins/chartdldr_pi/data/chart_sources.xml";
    
    _OCPN_DLStatus ret = OCPN_downloadFile(url, tfn.GetFullPath(), _("Downloading chart sources"), _("Downloading chart sources"), wxNullBitmap, this, OCPN_DLDS_ELAPSED_TIME | OCPN_DLDS_ESTIMATED_TIME | OCPN_DLDS_REMAINING_TIME | OCPN_DLDS_SPEED | OCPN_DLDS_SIZE | OCPN_DLDS_URL | OCPN_DLDS_CAN_PAUSE | OCPN_DLDS_CAN_ABORT | OCPN_DLDS_AUTO_CLOSE, 15);
    wxFileName fn;
    fn.SetPath(*GetpPrivateApplicationDataLocation());
    fn.SetFullName(_T("chartdldr_pi-chart_sources.xml"));

    switch (ret) {
        case OCPN_DL_NO_ERROR: {
            if (!wxCopyFile(tfn.GetFullPath(), fn.GetFullPath())) {
                OCPNMessageBox_PlugIn(this, wxString::Format(_("Failed to save: %s "), fn.GetFullPath().c_str()), _("Chart downloader"), wxOK | wxICON_ERROR);
            }
            break;
        }
        case OCPN_DL_FAILED: {
            OCPNMessageBox_PlugIn(this, 
                         wxString::Format(_("Failed to download: %s \nVerify there is a working Internet connection."), url.c_str()),
                         _("Chart downloader"), wxOK | wxICON_ERROR);
            break;
        }
        case OCPN_DL_USER_TIMEOUT:
        case OCPN_DL_ABORTED: {
            break;
        }
        case OCPN_DL_UNKNOWN:
        case OCPN_DL_STARTED: {
            break;
        }
            
        default:
            wxASSERT(false);  // This should never happen because we handle all possible cases of ret
    }
    if (wxFileExists(tfn.GetFullPath())) wxRemoveFile(tfn.GetFullPath());
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
    m_buttonDownloadMasterCatalog->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( ChartDldrPrefsDlg::OnDownloadMasterCatalog ), NULL, this );
}
