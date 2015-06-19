/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  About Dialog
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 *
 *
 */

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
      #include <wx/wx.h>
#endif

#include <wx/textfile.h>
#include <wx/ffile.h>
#include <wx/clipbrd.h>
#include <wx/html/htmlwin.h>

#include <version.h>

#include "about.h"
#include "chart1.h"
#include "chcanv.h"
#include "styles.h"
#include "version.h"
#include "OCPNPlatform.h"
#include "FontMgr.h"

wxString str_version_start = wxT("\n      Version ");
wxString str_version_major = wxString::Format(wxT("%i"),VERSION_MAJOR);
wxString str_version_minor = wxString::Format(wxT("%i"),VERSION_MINOR);
wxString str_version_patch = wxString::Format(wxT("%i"),VERSION_PATCH);
wxString str_version_date(VERSION_DATE, wxConvUTF8);
wxString OpenCPNVersion = str_version_start + str_version_major + wxT(".") + str_version_minor + wxT(".") + str_version_patch + wxT(" Build ") + str_version_date;

extern OCPNPlatform     *g_Platform;
extern MyFrame          *gFrame;
extern ocpnStyle::StyleManager* g_StyleManager;
extern about            *g_pAboutDlg;
extern bool             g_bresponsive;


char AboutText[] =
{
    "<br>OpenCPN<br>\
    (c) 2000-2015 The OpenCPN Authors<br><br>"
};

char OpenCPNInfo[] = {"<br><br>\
OpenCPN is a Free Software project, built by sailors. \
It is freely available to download and distribute \
without charge at opencpn.org.<br><br>\
If you use OpenCPN, please consider contributing \
or donating funds to the project.<br><br>\
For more information, visit http://opencpn.org<br><br>"
};


char AuthorText[] =
{
"   David S Register\n\
      OpenCPN Lead Developer\n\n\
    Pavel Kalian\n\
      S52 Rasterization Improvements\n\n\
    Sean D'Epagnier\n\
      OpenGL Architecture\n\n\
    J.P. Joubert\n\
      GRIB PlugIn enhancements\n\n\
    Thomas Höckne\n\
      Documentation and Wiki support\n\n\
    Caesar Schinas\n\
      User Interface and OS X improvements\n\n\
    Jesper Weissglas\n\
      Vector Chart Rendering\n\n\
    Jean-Eudes Onfray\n\
      Dashboard and Dialog enhancements\n\n\
    Kathleen Boswell\n\
      Icon design\n\n\
    Flavius Bindea\n\
      CM93 Offset and AIS enhancements\n\n\
    Gunther Pilz\n\
      Windows Installer enhancements\n\n\
    Alan Bleasby\n\
      Garmin jeeps module\n\n\
    Piotr Carlson\n\
      General usability enhancements\n\n\
    Anders Lund\n\
      RouteManagerDialog\n\n\
    Gordon Mau\n\
      OpenCPN Documentation\n\n\
    Tim Francis\n\
      OpenCPN Documentation\n\n\
    Mark A Sikes\n\
      OpenCPN CoDeveloper\n\n\
    Thomas Haller\n\
      GPX Import/Export Implementation\n\n\
    Will Kamp\n\
      Toolbar Icon design\n\n\
    Richard Smith\n\
      OpenCPN CoDeveloper, MacOSX\n\n\
    David Herring\n\
      OpenCPN CoDeveloper, MacOSX\n\n\
    Philip Lange\n\
      OpenCPN Documentation\n\n\
    Ron Kuris\n\
      wxWidgets Support\n\n\
    Julian Smart, Robert Roebling et al\n\
      wxWidgets Authors\n\n\
    Sylvain Duclos\n\
      S52 Presentation Library code\n\n\
    Manish P. Pagey\n\
      Serial Port Library\n\n\
    David Flater\n\
      XTIDE tide and current code\n\n\
    Frank Warmerdam\n\
      GDAL Class Library\n\n\
    Mike Higgins\n\
      BSB Chart Format Detail\n\n\
    Samuel R. Blackburn\n\
      NMEA0183 Class Library\n\n\
    Atul Narkhede\n\
      Polygon Graphics utilities\n\n\
    Jan C. Depner\n\
      WVS Chart Library\n\n\
    Stuart Cunningham, et al\n\
      BSB Chart Georeferencing Algorithms\n\n\
    John F. Waers\n\
      UTM Conversion Algorithms\n\n\
    Carsten Tschach\n\
      UTM Conversion Algorithms\n\n\
    Ed Williams\n\
      Great Circle Formulary\n\n\
    Philippe Bekaert\n\
      CIE->RGB Color Conversion Matrix\n\n\
    Robert Lipe\n\
      Garmin USB GPS Interface\n"

};

IMPLEMENT_DYNAMIC_CLASS( about, wxDialog )

BEGIN_EVENT_TABLE( about, wxDialog )
    EVT_BUTTON( xID_OK, about::OnXidOkClick )
    EVT_NOTEBOOK_PAGE_CHANGED(ID_NOTEBOOK_HELP, about::OnPageChange)
    EVT_BUTTON( ID_DONATE, about::OnDonateClick)
    EVT_BUTTON( ID_COPYINI, about::OnCopyClick)
    EVT_BUTTON( ID_COPYLOG, about::OnCopyClick)
    EVT_CLOSE( about::OnClose )
END_EVENT_TABLE()

about::about( )
{
}

about::about( wxWindow* parent,wxString Data_Locn, wxWindowID id, const wxString& caption,
                  const wxPoint& pos, const wxSize& size, long style)
{
  m_DataLocn = Data_Locn;
#ifdef __WXOSX__
  style |= wxSTAY_ON_TOP;
#endif
  Create(parent, id, caption, pos, size, style);
  m_parent = parent;
}


bool about::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos,
        const wxSize& size, long style )
{
    SetExtraStyle( GetExtraStyle() | wxWS_EX_BLOCK_EVENTS );
    wxDialog::Create( parent, id, caption, pos, size, style );

    m_parent = parent;

    m_btips_loaded = false;

    wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
    SetFont( *qFont );
    
    m_displaySize = g_Platform->getDisplaySize();
    CreateControls();
    
    Update();
    
    //Set the maximum size of the entire settings dialog
    SetSizeHints( -1, -1, m_displaySize.x-100, m_displaySize.y-100 );

    if( g_bresponsive )
        SetSize( wxSize( m_displaySize.x-100, m_displaySize.y-100 ) );
    else{
        GetSizer()->Fit( this );
        wxSize esize;
        esize.x = GetCharHeight() * 40;
        esize.y = GetCharHeight() * 30;
        SetSize( esize );
    }
    
        
    Centre();

    return TRUE;
}

void about::SetColorScheme( void )
{
    DimeControl( this );
    wxColor bg = GetBackgroundColour();
    pAboutHTMLCtl->SetBackgroundColour( bg );
    SetBackgroundColour( bg );                  // This looks like non-sense, but is needed for __WXGTK__
                                                // to get colours to propagate down the control's family tree.
    
    #ifdef __WXQT__    
    //  wxQT has some trouble clearing the background of HTML window...
    wxBitmap tbm( GetSize().x, GetSize().y, -1 );
    wxMemoryDC tdc( tbm );
    //    wxColour cback = GetGlobalColor( _T("YELO1") );
    tdc.SetBackground( bg );
    tdc.Clear();
    pAboutHTMLCtl->SetBackgroundImage(tbm);
    #endif
    
}

void about::Update()
{

    wxColor bg = GetBackgroundColour();
    wxColor fg = wxColour(0,0,0); //FontMgr::Get().GetFontColor( _("Dialog") );

    wxString aboutText;
    aboutText.Printf( _T("<html><body bgcolor=#%02x%02x%02x><font color=#%02x%02x%02x>"),
                   bg.Red(), bg.Blue(), bg.Green(), fg.Red(), fg.Blue(), fg.Green() );
    
    wxFont *dFont = FontMgr::Get().GetFont( _("Dialog") );
    wxString face = dFont->GetFaceName();
    
    #ifdef __WXOSX__
    int points = dFont->GetPointSize();
    #else
    int points = dFont->GetPointSize() + 1;
    #endif
    
    int sizes[7];
    for ( int i=-2; i<5; i++ ) {
        sizes[i+2] = points + i + (i>0?i:0);
    }
    pAboutHTMLCtl->SetFonts(face, face, sizes);
    
    if(wxFONTSTYLE_ITALIC == dFont->GetStyle())
        aboutText += _T("<i>");
    
    wxString *pAboutString = new wxString( AboutText, wxConvUTF8 );
    pAboutString->Append( OpenCPNVersion );
    pAboutString->Append( wxString( OpenCPNInfo, wxConvUTF8 ) );
    
    // Show the user where the log file is going to be
    wxString log = _T("    Logfile location: ");
    log.Append( g_Platform->GetLogFileName() );
    pAboutString->Append(log);
    
    // Show the user where the config file is going to be
    wxString conf = _T("<br><br>    Config file location: ");
    conf.Append( g_Platform->GetConfigFileName() );
    pAboutString->Append(conf);
    
    aboutText << *pAboutString;
    delete pAboutString;
    
    aboutText << _T("</font>");
    if(wxFONTSTYLE_ITALIC == dFont->GetStyle())
        aboutText << _T("</i>");
    
    aboutText << _T("</body></html>");
    
    pAboutHTMLCtl->SetPage( aboutText );

    
    pAuthorTextCtl->Clear();
    wxString *pAuthorsString = new wxString( AuthorText, wxConvUTF8 );
    pAuthorTextCtl->WriteText( *pAuthorsString );
    pAuthorTextCtl->SetInsertionPoint( 0 );
    
    delete pAuthorsString;

    pLicenseTextCtl->Clear();
    wxString license_loc(m_DataLocn );
    license_loc.Append( _T("license.txt") );

    wxTextFile license_file( license_loc );

    if( license_file.Open() ) {
        wxString str;
        str = license_file.GetFirstLine();
        pLicenseTextCtl->WriteText( str );

        while( !license_file.Eof() ) {
            str = license_file.GetNextLine();
            str.Append( _T("\n") );
            pLicenseTextCtl->AppendText( str );
        }
        license_file.Close();
    } else {
        wxString msg( _T("Could not open License file: ") );
        msg.Append( license_loc );
        wxLogMessage( msg );
    }
    pLicenseTextCtl->SetInsertionPoint( 0 );

    SetColorScheme();
}

void about::CreateControls()
{
    //  Set the nominal vertical size of the embedded controls
    int v_size = 300;
    if(g_bresponsive)
        v_size = -1;
    
    about* itemDialog1 = this;

    wxBoxSizer* aboutSizer = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( aboutSizer );
    wxStaticText *pST1 = new wxStaticText( this, -1, _T("Label"), wxDefaultPosition,
            wxSize( -1, 30/*500, 30*/ ), wxALIGN_CENTRE/* | wxALIGN_CENTER_VERTICAL*/ );
    pST1->SetLabel( _("The Open Source Chart Plotter/Navigator") );
    wxFont *headerFont = wxTheFontList->FindOrCreateFont( 14, wxFONTFAMILY_SWISS,
            wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD );
    pST1->SetFont( *headerFont );
    aboutSizer->Add( pST1, 0, wxALL | wxEXPAND, 8 );

    wxSizer *buttonSizer;
    if( m_displaySize.x < m_displaySize.y)
        buttonSizer = new wxBoxSizer( wxVERTICAL );
    else
        buttonSizer = new wxBoxSizer(wxHORIZONTAL );
    

    wxButton* copyIni = new wxButton( itemDialog1, ID_COPYINI, _("Copy Settings File to Clipboard") );
    buttonSizer->Add( copyIni, 1, wxALL | wxEXPAND, 3 );

    wxButton* copyLog = new wxButton( itemDialog1, ID_COPYLOG, _("Copy Log File to Clipboard") );
    buttonSizer->Add( copyLog, 1, wxALL | wxEXPAND, 3 );

    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

    wxBitmap donate_bmp = style->GetIcon( _T("donate") );

    wxButton* donateButton = new wxBitmapButton( itemDialog1, ID_DONATE, donate_bmp,
            wxDefaultPosition, wxDefaultSize, 0 );

    buttonSizer->Add( donateButton, 1, wxALL | wxEXPAND | wxALIGN_RIGHT, 3 );


    //  Main Notebook
    pNotebook = new wxNotebook( itemDialog1, ID_NOTEBOOK_HELP, wxDefaultPosition,
            wxSize( -1, -1 ), wxNB_TOP );
    pNotebook->InheritAttributes();
    aboutSizer->Add( pNotebook, 1, wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL, 5 );

    aboutSizer->Add( buttonSizer, 0, wxALL, 0 );

    //    About Panel
    itemPanelAbout = new wxPanel( pNotebook, -1, wxDefaultPosition, wxDefaultSize,
            wxSUNKEN_BORDER | wxTAB_TRAVERSAL );
    itemPanelAbout->InheritAttributes();
    pNotebook->AddPage( itemPanelAbout, _("About") );

    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer( wxVERTICAL );
    itemPanelAbout->SetSizer( itemBoxSizer6 );

    
    pAboutHTMLCtl = new wxHtmlWindow( itemPanelAbout, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                wxHW_SCROLLBAR_AUTO | wxHW_NO_SELECTION);
    
    pAboutHTMLCtl->SetBorders( 5 );
    itemBoxSizer6->Add( pAboutHTMLCtl, 1, wxALIGN_CENTER_HORIZONTAL | wxEXPAND | wxALL, 5 );
    
    //     Authors Panel
    itemPanelAuthors = new wxPanel( pNotebook, -1, wxDefaultPosition, wxDefaultSize,
            wxSUNKEN_BORDER | wxTAB_TRAVERSAL );
    itemPanelAuthors->InheritAttributes();
    pNotebook->AddPage( itemPanelAuthors, _("Authors") );

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer( wxVERTICAL );
    itemPanelAuthors->SetSizer( itemBoxSizer7 );

    pAuthorTextCtl = new wxTextCtrl( itemPanelAuthors, -1, _T(""), wxDefaultPosition,
                                     wxSize( -1, v_size ), wxTE_MULTILINE | wxTE_READONLY );
    pAuthorTextCtl->InheritAttributes();
    itemBoxSizer7->Add( pAuthorTextCtl, 1, wxALIGN_CENTER_HORIZONTAL | wxEXPAND | wxALL, 5 );

    //  License Panel
    itemPanelLicense = new wxPanel( pNotebook, -1, wxDefaultPosition, wxDefaultSize,
            wxSUNKEN_BORDER | wxTAB_TRAVERSAL );
    itemPanelLicense->InheritAttributes();
    pNotebook->AddPage( itemPanelLicense, _("License") );

    wxBoxSizer* itemBoxSizer8 = new wxBoxSizer( wxVERTICAL );
    itemPanelLicense->SetSizer( itemBoxSizer8 );

    int tcflags = wxTE_MULTILINE | wxTE_READONLY;

    //    wxX11 TextCtrl is broken in many ways.
    //    Here, the wxTE_DONTWRAP flag creates a horizontal scroll bar
    //    which fails in wxX11 2.8.2....
#ifndef __WXX11__
    tcflags |= wxTE_DONTWRAP;
#endif
    pLicenseTextCtl = new wxTextCtrl( itemPanelLicense, -1, _T(""), wxDefaultPosition,
                                      wxSize( -1, v_size ), tcflags );

    pLicenseTextCtl->InheritAttributes();
    itemBoxSizer8->Add( pLicenseTextCtl, 1, wxALIGN_CENTER_HORIZONTAL | wxEXPAND | wxALL, 5 );

    //     Help Panel
    itemPanelTips = new wxPanel( pNotebook, -1, wxDefaultPosition, wxDefaultSize,
            wxSUNKEN_BORDER | wxTAB_TRAVERSAL );
    itemPanelTips->InheritAttributes();
    pNotebook->AddPage( itemPanelTips, _("Help") );

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer( wxVERTICAL );
    itemPanelTips->SetSizer( itemBoxSizer9 );

    //    Close Button

    wxBoxSizer* itemBoxSizer28 = new wxBoxSizer( wxHORIZONTAL );
    aboutSizer->Add( itemBoxSizer28, 0, wxALIGN_RIGHT | wxALL, 5 );

    wxButton* itemButton29 = new wxButton( itemDialog1, xID_OK, _("Close"), wxDefaultPosition,
            wxDefaultSize, 0 );
    itemButton29->SetDefault();
    itemButton29->InheritAttributes();
    itemBoxSizer28->Add( itemButton29, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );
}


void about::OnXidOkClick( wxCommandEvent& event )
{
  Close();
}

void about::OnClose( wxCloseEvent& event )
{
    Destroy();
    g_pAboutDlg = NULL;
}

void about::OnDonateClick( wxCommandEvent& event )
{
      wxLaunchDefaultBrowser(_T("https://sourceforge.net/donate/index.php?group_id=180842"));
}

void about::OnCopyClick( wxCommandEvent& event )
{
    wxString filename = g_Platform->GetConfigFileName();
    if( event.GetId() == ID_COPYLOG )
        filename = g_Platform->GetLogFileName();

    wxFFile file( filename );

    if( ! file.IsOpened() ) {
        wxLogMessage( _T("Failed to open file for Copy to Clipboard.") );
        return;
    }

    wxString fileContent;
    char buf[1024];
    while( ! file.Eof() ) {
        int c = file.Read( &buf, 1024 );
        if( c ) fileContent += wxString( buf, wxConvUTF8, c );
    }

    file.Close();
    int length = fileContent.Length();

    if( event.GetId() == ID_COPYLOG ) {
        wxString lastLogs = fileContent;
        int pos = lastLogs.Find( _T("________") );
        while( pos != wxNOT_FOUND && lastLogs.Length() > 65000 ) {
            lastLogs = lastLogs.Right(lastLogs.Length() - pos - 8);
            pos = lastLogs.Find( _T("________") );
        }
        fileContent = lastLogs;
    }

    ::wxBeginBusyCursor();

    if( wxTheClipboard->Open() ) {
        wxTextDataObject* data = new wxTextDataObject;
        data->SetText( fileContent );
        if( ! wxTheClipboard->SetData( data ) ) {
            wxLogMessage( _T("wxTheClipboard->Open() failed.") );
        }
        wxTheClipboard->Close();
    } else {
        wxLogMessage( _T("wxTheClipboard->Open() failed.") );
    }
    ::wxEndBusyCursor();
}

void about::OnPageChange( wxNotebookEvent& event )
{
    int i = event.GetSelection();

    if( 3 == i ) { // 3 is the index of "Help" page
        gFrame->LaunchLocalHelp();
        pNotebook->ChangeSelection(0);
    }
}
