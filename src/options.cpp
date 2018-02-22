/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Options Dialog
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   bdbcat@yahoo.com   *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *
 *
 */

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"


#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/generic/progdlgg.h"
#include "wx/sound.h"
#include <wx/radiobox.h>
#include <wx/listbox.h>
#include <wx/imaglist.h>
#include <wx/display.h>
#include <wx/choice.h>

#include "dychart.h"
#include "chart1.h"
#include "options.h"

#include "navutil.h"

#ifdef USE_S57
#include "s52plib.h"
#include "s52utils.h"
#include "cm93.h"
#endif

wxString GetOCPNKnownLanguage(wxString lang_canonical, wxString *lang_dir);


extern bool             g_bShowPrintIcon;
extern bool             g_bShowOutlines;
extern bool             g_bShowDepthUnits;
extern bool             g_bskew_comp;

extern wxString         *pNMEADataSource;
extern wxString         g_NMEABaudRate;
extern wxString         *pNMEA_AP_Port;
extern FontMgr          *pFontMgr;
extern wxString         *pAIS_Port;
extern wxString         *pInit_Chart_Dir;
extern bool             g_bAutoAnchorMark;
extern ColorScheme      global_color_scheme;
extern bool             g_bGarminPersistance;
extern bool             g_bGarminHost;
extern bool             g_bfilter_cogsog;
extern int              g_COGFilterSec;
extern int              g_SOGFilterSec;

extern PlugInManager    *g_pi_manager;

extern wxString         g_SData_Locn;

extern bool             g_bDisplayGrid;

//    AIS Global configuration
extern bool             g_bCPAMax;
extern double           g_CPAMax_NM;
extern bool             g_bCPAWarn;
extern double           g_CPAWarn_NM;
extern bool             g_bTCPA_Max;
extern double           g_TCPA_Max;
extern bool             g_bMarkLost;
extern double           g_MarkLost_Mins;
extern bool             g_bRemoveLost;
extern double           g_RemoveLost_Mins;
extern bool             g_bShowCOG;
extern double           g_ShowCOG_Mins;
extern bool             g_bAISShowTracks;
extern double           g_AISShowTracks_Mins;
extern bool             g_bShowMoored;
extern double           g_ShowMoored_Kts;
extern bool             g_bAIS_CPA_Alert;
extern bool             g_bAIS_CPA_Alert_Audio;
extern wxString         g_sAIS_Alert_Sound_File;
extern bool             g_bAIS_CPA_Alert_Suppress_Moored;

extern bool             g_bNavAidShowRadarRings;
extern int              g_iNavAidRadarRingsNumberVisible;
extern float            g_fNavAidRadarRingsStep;
extern int              g_pNavAidRadarRingsStepUnits;
extern bool             g_bWayPointPreventDragging;

extern bool             g_bPreserveScaleOnX;
extern bool             g_bPlayShipsBells;   // pjotrc 2010.02.09
extern bool             g_bFullscreenToolbar;
extern bool             g_bTransparentToolbar;
extern bool             g_bShowLayers;

extern bool             g_bEnableZoomToCursor;
extern bool             g_bShowTrackIcon;
extern bool             g_bTrackDaily;
extern double           g_TrackIntervalSeconds;
extern double           g_TrackDeltaDistance;
extern double           g_TrackDeltaDistance;
extern bool             g_bTrackTime;
extern bool             g_bTrackDistance;
extern int              g_iSDMMFormat;

extern int              g_cm93_zoom_factor;
extern CM93DSlide       *pCM93DetailSlider;
extern bool             g_bShowCM93DetailSlider;

extern TTYWindow        *g_NMEALogWindow;
extern int              g_NMEALogWindow_x, g_NMEALogWindow_y;
extern int              g_NMEALogWindow_sx, g_NMEALogWindow_sy;

extern bool             g_bUseRaster;
extern bool             g_bUseVector;
extern bool             g_bUseCM93;
extern int              g_COGAvgSec;

extern bool             g_bCourseUp;
extern bool             g_bLookAhead;

extern double           g_ownship_predictor_minutes;

extern PlugInManager    *g_pi_manager;

extern bool             g_bAISRolloverShowClass;
extern bool             g_bAISRolloverShowCOG;
extern bool             g_bAISRolloverShowCPA;

extern bool             g_bAIS_ACK_Timeout;
extern double           g_AckTimeout_Mins;

extern bool             g_bQuiltEnable;
extern bool             g_bFullScreenQuilt;
extern int              g_iSDMMFormat;

extern wxLocale         locale_def_lang;

#ifdef USE_WIFI_CLIENT
extern wxString         *pWIFIServerName;
#endif

#ifdef USE_S57
extern s52plib          *ps52plib;
#endif

extern wxString         g_locale;

//    Some constants
#define ID_CHOICE_NMEA  wxID_HIGHEST + 1

extern wxArrayString *EnumerateSerialPorts(void);           // in chart1.cpp

IMPLEMENT_DYNAMIC_CLASS( options, wxDialog )

BEGIN_EVENT_TABLE( options, wxDialog )
//    BUGBUG DSR Must use wxID_TREECTRL to capture tree events.

    EVT_TREE_SEL_CHANGED( wxID_TREECTRL, options::OnDirctrlSelChanged )
    EVT_CHECKBOX( ID_DEBUGCHECKBOX1, options::OnDebugcheckbox1Click )
    EVT_TREE_SEL_CHANGED( ID_DIRCTRL, options::OnDirctrlSelChanged )
    EVT_BUTTON( ID_BUTTONADD, options::OnButtonaddClick )
    EVT_BUTTON( ID_BUTTONDELETE, options::OnButtondeleteClick )
    EVT_BUTTON( xID_OK, options::OnXidOkClick )
    EVT_BUTTON( wxID_CANCEL, options::OnCancelClick )
    EVT_BUTTON( ID_BUTTONFONTCHOOSE, options::OnChooseFont )
    EVT_NOTEBOOK_PAGE_CHANGED(ID_NOTEBOOK, options::OnPageChange)
    EVT_CHOICE( ID_CHOICE_NMEA, options::OnNMEASourceChoice )
    EVT_COMBOBOX( ID_CHOICE_NMEA, options::OnNMEASourceChoice )
    EVT_RADIOBOX(ID_RADIOBOX, options::OnDisplayCategoryRadioButton )
    EVT_BUTTON( ID_CLEARLIST, options::OnButtonClearClick )
    EVT_BUTTON( ID_SELECTLIST, options::OnButtonSelectClick )
    EVT_BUTTON( ID_AISALERTSELECTSOUND, options::OnButtonSelectSound )
    EVT_BUTTON( ID_AISALERTTESTSOUND, options::OnButtonTestSound )
    EVT_CHECKBOX( ID_SHOWGPSWINDOW, options::OnShowGpsWindowCheckboxClick )

END_EVENT_TABLE()


options::options( )
{
      Init();
}

options::options( MyFrame* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{
      Init();

      pParent = parent;

      //    As a display optimization....
      //    if current color scheme is other than DAY,
      //    Then create the dialog ..WITHOUT.. borders and title bar.
      //    This way, any window decorations set by external themes, etc
      //    will not detract from night-vision

      long wstyle = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER ;//| wxVSCROLL;
//      if(global_color_scheme != GLOBAL_COLOR_SCHEME_DAY)
//            wstyle |= (wxNO_BORDER);

//      SetLayoutAdaptationLevel(2);

//      Create(parent, id, caption, pos, size, wstyle);

      SetExtraStyle(wxWS_EX_BLOCK_EVENTS);

      wxDialog::Create( parent, id, caption, pos, size,wstyle );

      CreateControls();

      //    Figure out the "best" size for the dialog.
      //    Logic:  Find the minimum size of each page as if it were not a scrolling dialog.
      //            Then, set the virtual size of all of the pages to be that size
      //            Result is that if space allows, no scrolling will happen.
      //            On a smaller physical screen, all pages will have scrolling enabled if necessary
      //            depending, of course, on their content.
      wxSize largest_unscrolled_page_size(0,0);
      wxSize pagesize(0,0);

      for (size_t i = 0; i < itemNotebook4->GetPageCount(); i++)
      {
            wxWindow* page = itemNotebook4->GetPage(i);
//            if(ID_PANELPIM == page->GetId())
//                  continue;

            wxScrolledWindow* scrolledWindow = wxDynamicCast(page, wxScrolledWindow);
            if (scrolledWindow)
            {
                  scrolledWindow->SetScrollRate(0, 0);
                  pagesize = scrolledWindow->GetBestSize();
                  scrolledWindow->SetScrollRate(1, 1);
            }
            else
                  pagesize = page->GetBestSize();


            largest_unscrolled_page_size = wxSize(wxMax(largest_unscrolled_page_size.x, pagesize.x), wxMax(largest_unscrolled_page_size.y, pagesize.y));

      }


      for (size_t i = 0; i < itemNotebook4->GetPageCount(); i++)
      {
            wxWindow* page = itemNotebook4->GetPage(i);

            wxSizer* pageSizer = page->GetSizer();
            page->SetMinSize(largest_unscrolled_page_size);
            pageSizer->SetMinSize(largest_unscrolled_page_size);
            pageSizer->SetVirtualSizeHints(page);
      }

      SetMaxSize(wxDisplay(wxDisplay::GetFromWindow(this)).GetClientArea().GetSize());

      Fit();
      //    Not sure why, but we need to account for some decorations
      wxSize now_size = GetSize();
      now_size.IncBy(8);
      SetSize(now_size);

      Centre();
}

options::~options( )
{
      delete m_pSerialArray;

}

void options::Init()
{
    pDirCtl = NULL;
    m_pWorkDirList = NULL;

    pDebugShowStat = NULL;
    pSelCtl = NULL;
    pListBox = NULL;
    ps57Ctl = NULL;
    ps57CtlListBox = NULL;
    pDispCat = NULL;
    m_pSerialArray = NULL;
    pUpdateCheckBox = NULL;
    k_charts = 0;

    itemStaticBoxSizer11 = NULL;
    pDirCtl = NULL;;
    itemActiveChartStaticBox = NULL;

    m_bVisitLang = false;
    m_itemFontElementListBox = NULL;

    m_pSerialArray = EnumerateSerialPorts();

    itemNotebook4 = NULL;
}


bool options::Create( MyFrame* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
{

    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );


    CreateControls();


    Fit();
    Centre();

    return TRUE;
}

wxWindow* options::GetContentWindow() const
{
      return NULL;//itemNotebook4;
}

wxScrolledWindow *CreatePanel(wxNotebook *parent, wxString title)
{
#if 0
      wxPanel *ppanel = new wxPanel( parent, ID_PANELAIS, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER);
      parent->AddPage(ppanel, title);
      wxSizer* newSizer1 = new wxBoxSizer(wxVERTICAL);
      ppanel->SetSizer(newSizer1);

      wxScrolledWindow *Window = new wxScrolledWindow( ppanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, /*wxSUNKEN_BORDER|*/wxTAB_TRAVERSAL|wxVSCROLL );
      Window->SetScrollRate(0, 1);
      newSizer1->Add(Window,1, wxEXPAND, 0);
#endif
      wxScrolledWindow *Window = new wxScrolledWindow( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, /*wxSUNKEN_BORDER|*/wxTAB_TRAVERSAL|wxVSCROLL );
      Window->SetScrollRate(1, 1);
      parent->AddPage(Window, title);

//      newSizer1->Add(Window,1, wxEXPAND, 0);


      return Window;
}


void options::CreateControls()
{
    unsigned int iPortIndex;

    int border_size = 4;
    int check_spacing = 4;
    int check_spacing_2 = 2;
    int group_item_spacing = 1;           // use for items within one group, with Add(...wxALL)

    wxFont *qFont = wxTheFontList->FindOrCreateFont ( 10, wxFONTFAMILY_DEFAULT,
                wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
    SetFont(*qFont);

    int font_size_y, font_descent, font_lead;
    GetTextExtent(_T("0"), NULL, &font_size_y, &font_descent, &font_lead);
    wxSize small_button_size(-1, (int)(1.4 * (font_size_y + font_descent + font_lead)));


    //      Check the display size.
    //      If "small", adjust some factors to squish out some more white space
    int width, height;
    ::wxDisplaySize(&width, &height);

    if(height < 800)
    {
          border_size = 2;
          check_spacing = 2;
          group_item_spacing = 1;

          wxFont *sFont = wxTheFontList->FindOrCreateFont ( 8, wxFONTFAMILY_DEFAULT,
                      wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
          SetFont(*sFont);

          int font_size_y, font_descent, font_lead;
          GetTextExtent(_T("0"), NULL, &font_size_y, &font_descent, &font_lead);
          small_button_size = wxSize(-1, (int)(1.5 * (font_size_y + font_descent + font_lead)));
    }

    options* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    itemNotebook4 = new wxNotebook( itemDialog1, ID_NOTEBOOK, wxDefaultPosition,
            wxSize(-1, -1), wxNB_TOP );
    itemBoxSizer2->Add(itemNotebook4, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, border_size);

   //      Add Invariant Notebook buttons
    wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer2->Add(itemBoxSizer28, 0, wxALIGN_RIGHT|wxALL, border_size);

    m_OKButton = new wxButton( itemDialog1, xID_OK, _("Ok") );
    m_OKButton->SetDefault();
    itemBoxSizer28->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, border_size);

    m_CancelButton = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel") );
    itemBoxSizer28->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, border_size);


    //  Create "Settings" panel

    itemPanel5 = CreatePanel(itemNotebook4, _("Settings"));


    wxBoxSizer* itemBoxSizer6 = new wxBoxSizer(wxVERTICAL);
    itemPanel5->SetSizer(itemBoxSizer6);


    //  Debug checkbox
    wxStaticBox* itemStaticBoxSizerDebugStatic = new wxStaticBox(itemPanel5, wxID_ANY, _("Navigation Data"));
    wxStaticBoxSizer* itemStaticBoxSizerDebug = new wxStaticBoxSizer(itemStaticBoxSizerDebugStatic, wxVERTICAL);
    itemBoxSizer6->Add(itemStaticBoxSizerDebug, 0, wxEXPAND|wxALL, border_size);
    pDebugShowStat = new wxCheckBox( itemPanel5, ID_DEBUGCHECKBOX1, _("Show Status Bar") );
    pDebugShowStat->SetValue(FALSE);
    itemStaticBoxSizerDebug->Add(pDebugShowStat, 1, wxALIGN_LEFT|wxALL, 2);

    //  Printing checkbox
    wxStaticBox* itemStaticBoxSizerPrintStatic = new wxStaticBox(itemPanel5, wxID_ANY, _("Printing"));
    wxStaticBoxSizer* itemStaticBoxSizerPrint = new wxStaticBoxSizer(itemStaticBoxSizerPrintStatic, wxVERTICAL);
    itemBoxSizer6->Add(itemStaticBoxSizerPrint, 0, wxEXPAND|wxALL, 5);
    pPrintShowIcon = new wxCheckBox( itemPanel5, ID_PRINTCHECKBOX1, _("Show Printing Icon"));
    pPrintShowIcon->SetValue(FALSE);
    itemStaticBoxSizerPrint->Add(pPrintShowIcon, 1, wxALIGN_LEFT|wxALL, 2);

    // Chart Display Options Box
    wxStaticBox* itemStaticBoxSizerCDOStatic = new wxStaticBox(itemPanel5, wxID_ANY, _("Chart Display Options"));
    wxStaticBoxSizer* itemStaticBoxSizerCDO = new wxStaticBoxSizer(itemStaticBoxSizerCDOStatic, wxVERTICAL);
    itemBoxSizer6->Add(itemStaticBoxSizerCDO, 0, wxEXPAND|wxALL, border_size);

    //  Quilting checkbox
    pCDOQuilting = new wxCheckBox( itemPanel5, ID_QUILTCHECKBOX1, _("Enable Chart Quilting"));
    itemStaticBoxSizerCDO->Add(pCDOQuilting, 1, wxALIGN_LEFT|wxALL, 2);

    //  Full Screen Quilting Disable checkbox
    pFullScreenQuilt = new wxCheckBox( itemPanel5, ID_FULLSCREENQUILT, _("Disable Full Screen Quilting"));
    itemStaticBoxSizerCDO->Add(pFullScreenQuilt, 1, wxALIGN_LEFT|wxALL, 2);

    //  "Course Up" checkbox
    pCBCourseUp = new wxCheckBox( itemPanel5, ID_COURSEUPCHECKBOX, _("Course UP Mode"));
    itemStaticBoxSizerCDO->Add(pCBCourseUp, 1, wxALIGN_LEFT|wxALL, 2);

    //  "LookAhead" checkbox
    pCBLookAhead = new wxCheckBox( itemPanel5, ID_CHECK_LOOKAHEAD, _("Look Ahead Mode"));
    itemStaticBoxSizerCDO->Add(pCBLookAhead, 1, wxALIGN_LEFT|wxALL, 2);

    //  Chart Outlines checkbox
    pCDOOutlines = new wxCheckBox( itemPanel5, ID_OUTLINECHECKBOX1, _("Show Chart Outlines"));
    itemStaticBoxSizerCDO->Add(pCDOOutlines, 1, wxALIGN_LEFT|wxALL, 2);

    //  Grid display  checkbox
    pSDisplayGrid = new wxCheckBox( itemPanel5, ID_CHECK_DISPLAYGRID, _("Show Grid"));
    itemStaticBoxSizerCDO->Add(pSDisplayGrid, 1, wxALIGN_LEFT|wxALL, 2);

    //  Depth Unit checkbox
    pSDepthUnits = new wxCheckBox( itemPanel5, ID_SHOWDEPTHUNITSBOX1, _("Show Depth Units"));
    itemStaticBoxSizerCDO->Add(pSDepthUnits, 1, wxALIGN_LEFT|wxALL, 2);

    //  Skewed Raster compenstation checkbox
    pSkewComp = new wxCheckBox( itemPanel5, ID_SKEWCOMPBOX, _("Show skewed raster charts as North-Up"));
    itemStaticBoxSizerCDO->Add(pSkewComp, 1, wxALIGN_LEFT|wxALL, 2);




    //  Auto Anchor Mark
//    pAutoAnchorMark = new wxCheckBox( itemPanel5, ID_AUTOANCHORMARKBOX1, _("Automatic Anchor Mark"));
//    itemStaticBoxSizerCDO->Add(pAutoAnchorMark, 1, wxALIGN_LEFT|wxALL, 2);

#if 0
    //  Chart Types
    wxStaticBox* itemStaticBoxSizerTypesStatic = new wxStaticBox(itemPanel5, wxID_ANY, _("Select Chart Types for Quilting"));
    wxStaticBoxSizer* itemStaticBoxSizerTypes = new wxStaticBoxSizer(itemStaticBoxSizerTypesStatic, wxVERTICAL);
    itemBoxSizer6->Add(itemStaticBoxSizerTypes, 0, wxEXPAND|wxALL, 5);

    pCBRaster = new wxCheckBox( itemPanel5, ID_RASTERCHECKBOX1, _("Raster"));
    itemStaticBoxSizerTypes->Add(pCBRaster, 1, wxALIGN_LEFT|wxALL, 2);

    pCBVector = new wxCheckBox( itemPanel5, ID_VECTORCHECKBOX1, _("ENC/Vector"));
    itemStaticBoxSizerTypes->Add(pCBVector, 1, wxALIGN_LEFT|wxALL, 2);

    pCBCM93 = new wxCheckBox( itemPanel5, ID_CM93CHECKBOX1, _("CM93"));
    itemStaticBoxSizerTypes->Add(pCBCM93, 1, wxALIGN_LEFT|wxALL, 2);
#endif


      //      OwnShip Display options
    wxStaticBox* itemStaticBoxOSDisplay = new wxStaticBox(itemPanel5, wxID_ANY, _("OwnShip Display Options"));
    wxStaticBoxSizer* itemStaticBoxSizerOSDisplay= new wxStaticBoxSizer(itemStaticBoxOSDisplay, wxVERTICAL);
    itemBoxSizer6->Add(itemStaticBoxSizerOSDisplay, 0, wxTOP|wxALL|wxEXPAND, border_size);

    wxFlexGridSizer *pOSDisplayGrid = new wxFlexGridSizer(2);
    pOSDisplayGrid->AddGrowableCol(1);
    itemStaticBoxSizerOSDisplay->Add(pOSDisplayGrid, 0, wxALL|wxEXPAND, border_size);

    wxStaticText *pStatic_OSCOG_Predictor = new wxStaticText( itemPanel5, -1, _("OwnShip COG arrow predictor length (Minutes):"));
    pOSDisplayGrid->Add(pStatic_OSCOG_Predictor, 1, wxALIGN_LEFT|wxALL, group_item_spacing);

    m_pText_OSCOG_Predictor = new wxTextCtrl(itemPanel5, -1);
    pOSDisplayGrid->Add(m_pText_OSCOG_Predictor, 1, wxALIGN_RIGHT, group_item_spacing);


    // Flav: for CM93Offset
    //      CM93Offset Display options
#ifdef FLAV
    {
      wxStaticBox* itemStaticBoxCM93OffsetDisplay = new wxStaticBox(itemPanel5, wxID_ANY, _("CM93 Offset Display"));
      wxStaticBoxSizer *itemStaticBoxSizerCM93OffsetDisplay= new wxStaticBoxSizer(itemStaticBoxCM93OffsetDisplay, wxVERTICAL);
      itemBoxSizer6->Add(itemStaticBoxSizerCM93OffsetDisplay, 0, wxTOP|wxALL|wxEXPAND, border_size);

      //  Activate CM93Offset checkbox
      pSActivateCM93Offset = new wxCheckBox( itemPanel5, ID_ACTIVATECM93OFFSET, _("Activate CM93 Offset"));
      itemStaticBoxSizerCM93OffsetDisplay->Add(pSActivateCM93Offset, 1, wxALIGN_LEFT|wxALL, 2);

      wxFlexGridSizer *pCM93OffsetDisplayGrid = new wxFlexGridSizer(2);
      pCM93OffsetDisplayGrid->AddGrowableCol(1);
      itemStaticBoxSizerCM93OffsetDisplay->Add(pCM93OffsetDisplayGrid, 0, wxALL|wxEXPAND, border_size);

      wxStaticText *pStatic_CM93OffsetX = new wxStaticText( itemPanel5, -1, _("X Offset (Positive moves map to East) (NMi Projected) :"));
      pCM93OffsetDisplayGrid->Add(pStatic_CM93OffsetX, 1, wxALIGN_LEFT|wxALL, group_item_spacing);

      m_pText_CM93OffsetX = new wxTextCtrl(itemPanel5, -1);
      pCM93OffsetDisplayGrid->Add(m_pText_CM93OffsetX, 1, wxALIGN_RIGHT, group_item_spacing);

      wxStaticText *pStatic_CM93OffsetY = new wxStaticText( itemPanel5, -1, _("Y Offset (Positive moves map to North) (NMi Projected) :"));
      pCM93OffsetDisplayGrid->Add(pStatic_CM93OffsetY, 1, wxALIGN_LEFT|wxALL, group_item_spacing);

      m_pText_CM93OffsetY = new wxTextCtrl(itemPanel5, -1);
      pCM93OffsetDisplayGrid->Add(m_pText_CM93OffsetY, 1, wxALIGN_RIGHT, group_item_spacing);

      itemStaticBoxSizerCM93OffsetDisplay->Show(g_CM93Maps_Offset_Enable);
    }
#endif

#ifdef USE_WIFI_CLIENT
//    Add WiFi Options Box
      wxStaticBox* itemWIFIStaticBox = new wxStaticBox(itemPanel5, wxID_ANY, _("WiFi Options"));
      wxStaticBoxSizer* itemWIFIStaticBoxSizer = new wxStaticBoxSizer(itemWIFIStaticBox, wxVERTICAL);
      itemBoxSizer6->Add(itemWIFIStaticBoxSizer, 0, wxEXPAND|wxALL, border_size);

//    Add WiFi TCP/IP Server address
      m_itemWIFI_TCPIP_StaticBox = new wxStaticBox(itemPanel5, wxID_ANY, _("TCP/IP WiFi Data Server"));
      m_itemWIFI_TCPIP_StaticBoxSizer = new wxStaticBoxSizer(m_itemWIFI_TCPIP_StaticBox, wxVERTICAL);
      itemWIFIStaticBoxSizer->Add(m_itemWIFI_TCPIP_StaticBoxSizer, 0, wxEXPAND|wxALL, border_size);

      m_itemWIFI_TCPIP_Source = new wxTextCtrl(itemPanel5, wxID_ANY);
      m_itemWIFI_TCPIP_StaticBoxSizer->Add(m_itemWIFI_TCPIP_Source, 0, wxEXPAND|wxALL, border_size);

      m_itemWIFI_TCPIP_StaticBox->Enable(1);
      m_itemWIFI_TCPIP_Source->Enable(1);

      wxString ip;
      ip = pWIFIServerName->Mid(7);
      m_itemWIFI_TCPIP_Source->WriteText(ip);
#endif



    //  Create "GPS" panel

      wxScrolledWindow *itemPanelGPS = CreatePanel(itemNotebook4,  _("GPS"));

//      itemPanelGPS = new wxPanel( itemNotebook4, -1, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
//      itemNotebook4->AddPage(itemPanelGPS, _("GPS"));

      wxBoxSizer* itemBoxSizerGPS = new wxBoxSizer(wxVERTICAL);
      itemPanelGPS->SetSizer(itemBoxSizerGPS);


//    Add NMEA Options Box
      wxStaticBox* itemNMEAStaticBox = new wxStaticBox(itemPanelGPS, wxID_ANY, _("NMEA Options"));
      wxStaticBoxSizer* itemNMEAStaticBoxSizer = new wxStaticBoxSizer(itemNMEAStaticBox, wxVERTICAL);
      itemBoxSizerGPS->Add(itemNMEAStaticBoxSizer, 0, wxEXPAND|wxALL, 4);


//    Add NMEA data source controls
      wxStaticBox* itemNMEASourceStaticBox = new wxStaticBox(itemPanelGPS, wxID_ANY, _("NMEA Data Source"));
      wxStaticBoxSizer* itemNMEASourceStaticBoxSizer = new wxStaticBoxSizer(itemNMEASourceStaticBox, wxVERTICAL);
      itemNMEAStaticBoxSizer->Add(itemNMEASourceStaticBoxSizer, 0, wxEXPAND|wxALL, 4);

      m_itemNMEAListBox = new wxComboBox(itemPanelGPS, ID_CHOICE_NMEA);
      itemNMEASourceStaticBoxSizer->Add(m_itemNMEAListBox, 0, wxEXPAND|wxALL, border_size);


#if !defined( OCPN_NO_SOCKETS) || defined(BUILD_WITH_LIBGPS)

//    Add NMEA TCP/IP Server address
      m_itemNMEA_TCPIP_StaticBox = new wxStaticBox(itemPanelGPS, wxID_ANY, _("GPSD Data Server"));
      m_itemNMEA_TCPIP_StaticBoxSizer = new wxStaticBoxSizer(m_itemNMEA_TCPIP_StaticBox, wxVERTICAL);
      itemNMEAStaticBoxSizer->Add(m_itemNMEA_TCPIP_StaticBoxSizer, 0, wxEXPAND|wxALL, 4);

      m_itemNMEA_TCPIP_Source = new wxTextCtrl(itemPanelGPS, wxID_ANY);
      m_itemNMEA_TCPIP_StaticBoxSizer->Add(m_itemNMEA_TCPIP_Source, 0, wxEXPAND|wxALL, border_size);

      m_itemNMEA_TCPIP_StaticBox->Enable(false);
      m_itemNMEA_TCPIP_Source->Enable(false);

      m_itemNMEA_TCPIP_Source->Clear();
      m_itemNMEA_TCPIP_Source->WriteText(_T("localhost"));

#endif


      m_itemNMEAListBox->Append( _T("None"));

      //    Fill in the listbox with all detected serial ports
      for (iPortIndex=0 ; iPortIndex < m_pSerialArray->GetCount() ; iPortIndex++)
            m_itemNMEAListBox->Append( m_pSerialArray->Item(iPortIndex) );


//    Search the string array looking for "GARMIN"
      bool bfound_garmin_string = false;
      for (iPortIndex=0 ; iPortIndex < m_pSerialArray->GetCount() ; iPortIndex++)
      {
            if(   m_pSerialArray->Item(iPortIndex).Contains(_T("GARMIN")))
            {
                  bfound_garmin_string = true;
                  break;
            }
      }

        //  Garmin persistence logic:
        //  Make sure "GARMIN" is in the list if the persistence flag is set.
        //  This covers the situation where Garmin is desired, but the
        //  device is not connected yet.
        //  n.b. Hot-plugging is not supported.  Opencpn must be
        //  restarted with device inserted to enable this option.
      if(g_bGarminPersistance && !bfound_garmin_string)
            m_itemNMEAListBox->Append( _T("GARMIN"));

#ifdef BUILD_WITH_LIBGPS
      m_itemNMEAListBox->Append( _T("Network LIBGPS"));
#endif

#ifndef OCPN_NO_SOCKETS
      m_itemNMEAListBox->Append( _T("Network GPSD"));
#endif

      m_itemNMEAListBox->Append( _T("AIS Port (Shared)"));


//    Activate the proper selections
//    n.b. Hard coded indices
      int scidx;
      wxString source;
      source = (*pNMEADataSource);
      if(source.Upper().Contains(_T("SERIAL")))
      {
            wxString sourcex = source.Mid(7);
            scidx = m_itemNMEAListBox->FindString(sourcex);
      }
      else if(source.Upper().Contains(_T("NONE")))
            scidx = 0;

      else if(source.Upper().Contains(_T("GARMIN")))
            scidx = m_itemNMEAListBox->FindString(_T("GARMIN"));

      else if(source.Upper().Contains(_T("AIS")))
            scidx = m_itemNMEAListBox->FindString(_T("AIS Port (Shared)"));

#ifdef BUILD_WITH_LIBGPS
      else if(source.Upper().Contains(_T("LIBGPS")))
      {
            scidx = m_itemNMEAListBox->FindString(_T("Network LIBGPS"));
            wxString ip;
            if(source.StartsWith(_T("LIBGPS")) )
                  ip = source.AfterFirst(':');
            else
                  ip = _T("localhost");

            m_itemNMEA_TCPIP_Source->Clear();
            m_itemNMEA_TCPIP_Source->WriteText(ip);
            m_itemNMEA_TCPIP_StaticBox->Enable(true);
            m_itemNMEA_TCPIP_Source->Enable(true);
      }
#endif

#ifndef OCPN_NO_SOCKETS
      else if(source.Upper().Contains(_T("GPSD")))
      {
            scidx = m_itemNMEAListBox->FindString(_T("Network GPSD"));
            wxString ip;
            if(source.StartsWith(_T("GPSD")) )
                  ip = source.AfterFirst(':');
            else
                  ip = _T("localhost");

            m_itemNMEA_TCPIP_Source->Clear();
            m_itemNMEA_TCPIP_Source->WriteText(ip);
            m_itemNMEA_TCPIP_StaticBox->Enable(true);
            m_itemNMEA_TCPIP_Source->Enable(true);
      }
#endif
      else
            scidx = 0;                                 // malformed selection


      if(scidx ==  wxNOT_FOUND)                  // user specified in ComboBox
      {
            wxString nsource = source.AfterFirst(':');
            m_itemNMEAListBox->Append( nsource );
            scidx = m_itemNMEAListBox->FindString(nsource);
      }


      m_itemNMEAListBox->SetSelection(scidx);


      //    NMEA Baud Rate
      wxStaticBox* itemNMEABaudStaticBox = new wxStaticBox(itemPanelGPS, wxID_ANY, _("NMEA Baud Rate"));
      wxStaticBoxSizer* itemNMEABaudStaticBoxSizer = new wxStaticBoxSizer(itemNMEABaudStaticBox, wxVERTICAL);
      itemNMEAStaticBoxSizer->Add(itemNMEABaudStaticBoxSizer, 0, wxEXPAND|wxALL, 4);

      m_itemNMEABaudListBox = new wxComboBox(itemPanelGPS, ID_CHOICE_NMEA_BAUD);
      m_itemNMEABaudListBox->Append( _T("38400"));
      m_itemNMEABaudListBox->Append( _T("9600"));
      m_itemNMEABaudListBox->Append( _T("4800"));

      itemNMEABaudStaticBoxSizer->Add(m_itemNMEABaudListBox, 0, wxEXPAND|wxALL, border_size);

      scidx = m_itemNMEABaudListBox->FindString(g_NMEABaudRate);      // entry value
      m_itemNMEABaudListBox->SetSelection(scidx);

      //    Garmin Host Mode
      pGarminHost = new wxCheckBox( itemPanelGPS, ID_GARMINHOST, _("Use Garmin GRMN/GRMN (Host) Mode for Waypoint and Route uploads."));
      itemNMEAStaticBoxSizer->Add(pGarminHost, 1, wxALIGN_LEFT|wxALL, 2);

      //    NMEA Monitor window
      pShowGPSWin = new wxCheckBox( itemPanelGPS, ID_SHOWGPSWINDOW, _("Show GPS/NMEA data stream window"));
      itemNMEAStaticBoxSizer->Add(pShowGPSWin, 1, wxALIGN_LEFT|wxALL, 2);

      //    NMEA Data Filtering
      pFilterNMEA = new wxCheckBox( itemPanelGPS, ID_FILTERNMEA, _("Filter NMEA Course and Speed data"));
      itemNMEAStaticBoxSizer->Add(pFilterNMEA, 1, wxALIGN_LEFT|wxALL, 2);

      wxFlexGridSizer *pFilterGrid = new wxFlexGridSizer(2);
      pFilterGrid->AddGrowableCol(1);
      itemNMEAStaticBoxSizer->Add(pFilterGrid, 0, wxALL|wxEXPAND, border_size);

      wxStaticText* itemStaticTextFilterSecs = new wxStaticText( itemPanelGPS, wxID_STATIC, _("Filter Period (Sec.)"));
      pFilterGrid->Add(itemStaticTextFilterSecs, 0, wxALIGN_LEFT|wxADJUST_MINSIZE, border_size);

      pFilterSecs = new wxTextCtrl( itemPanelGPS, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxSize(100, -1) );
      pFilterGrid->Add(pFilterSecs, 0, wxALIGN_RIGHT, 2);

      //    Course Up display update period
      wxFlexGridSizer *pCOGUPFilterGrid = new wxFlexGridSizer(2);
      pCOGUPFilterGrid->AddGrowableCol(1);
      itemNMEAStaticBoxSizer->Add(pCOGUPFilterGrid, 0, wxALL|wxEXPAND, border_size);

      wxStaticText* itemStaticTextCOGUPFilterSecs = new wxStaticText( itemPanelGPS, wxID_STATIC, _("Course-Up Mode Display Update Period (Sec.)"));
      pCOGUPFilterGrid->Add(itemStaticTextCOGUPFilterSecs, 0, wxALIGN_LEFT|wxADJUST_MINSIZE, border_size);

      pCOGUPUpdateSecs = new wxTextCtrl( itemPanelGPS, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxSize(100, -1) );
      pCOGUPFilterGrid->Add(pCOGUPUpdateSecs, 0, wxALIGN_RIGHT, 2);

//    Add Autopilot serial output port controls
      wxStaticBox* itemNMEAAutoStaticBox = new wxStaticBox(itemPanelGPS, wxID_ANY, _("Autopilot Output Port"));
      wxStaticBoxSizer* itemNMEAAutoStaticBoxSizer = new wxStaticBoxSizer(itemNMEAAutoStaticBox, wxVERTICAL);
      itemNMEAStaticBoxSizer->Add(itemNMEAAutoStaticBoxSizer, 0, wxEXPAND|wxALL, 10);

      m_itemNMEAAutoListBox = new wxComboBox(itemPanelGPS, ID_CHOICE_AP);
      m_itemNMEAAutoListBox->Append( _T("None"));

      //    Fill in the listbox with all detected serial ports
      for (iPortIndex=0 ; iPortIndex < m_pSerialArray->GetCount() ; iPortIndex++)
            m_itemNMEAAutoListBox->Append( m_pSerialArray->Item(iPortIndex) );

      wxString ap_com;
      if(pNMEA_AP_Port->Contains(_T("Serial")))
            ap_com = pNMEA_AP_Port->Mid(7);
      else
            ap_com = _T("None");

      int sapidx = m_itemNMEAAutoListBox->FindString(ap_com);
      m_itemNMEAAutoListBox->SetSelection(sapidx);

      itemNMEAAutoStaticBoxSizer->Add(m_itemNMEAAutoListBox, 0, wxEXPAND|wxALL, border_size);






//    Build "Charts" page

      itemPanel9 = CreatePanel(itemNotebook4,  _("Charts"));

      itemBoxSizer10 = new wxBoxSizer(wxVERTICAL);
      itemPanel9->SetSizer(itemBoxSizer10);

      CreateChartsPage();

    //      Build S57 Options page

    wxScrolledWindow *ps57Ctl = CreatePanel(itemNotebook4, _("Vector Charts"));

    wxBoxSizer* itemBoxSizer25 = new wxBoxSizer(wxVERTICAL);
    ps57Ctl->SetSizer(itemBoxSizer25);

    wxStaticBox* itemStaticBoxSizer26Static = new wxStaticBox(ps57Ctl, wxID_ANY, _("Chart Display Filters"));
    wxStaticBoxSizer* itemStaticBoxSizer26 = new wxStaticBoxSizer(itemStaticBoxSizer26Static, wxHORIZONTAL);
    itemBoxSizer25->Add(itemStaticBoxSizer26, 0, wxALL|wxEXPAND, 4);

    wxStaticBox* itemStaticBoxSizer57Static = new wxStaticBox(ps57Ctl, wxID_ANY, _("Mariner's Standard"));
    wxStaticBoxSizer* itemStaticBoxSizer57 = new wxStaticBoxSizer(itemStaticBoxSizer57Static, wxVERTICAL);
    itemStaticBoxSizer26->Add(itemStaticBoxSizer57, 0, wxALL|wxEXPAND, 2);

    wxString* ps57CtlListBoxStrings = NULL;
    ps57CtlListBox = new wxCheckListBox( ps57Ctl, ID_CHECKLISTBOX, wxDefaultPosition, wxSize(-1, 180), 0,
                                         ps57CtlListBoxStrings, wxLB_SINGLE );
    itemStaticBoxSizer57->Add(ps57CtlListBox, 0, wxALIGN_LEFT|wxALL, border_size);

 //   wxBoxSizer* itemBoxSizer57 = new wxBoxSizer(wxVERTICAL);
//    itemStaticBoxSizer57->Add(itemBoxSizer57, 1, wxALL|wxEXPAND, 2);

    itemButtonClearList = new wxButton( ps57Ctl, ID_CLEARLIST, _("Clear All"));
    itemStaticBoxSizer57->Add(itemButtonClearList, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);

    itemButtonSelectList = new wxButton( ps57Ctl, ID_SELECTLIST, _("Select All"));
    itemButtonSelectList->SetDefault();
    itemStaticBoxSizer57->Add(itemButtonSelectList, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);



    wxBoxSizer* itemBoxSizer75 = new wxBoxSizer(wxVERTICAL);
    itemStaticBoxSizer26->Add(itemBoxSizer75, 0, wxALL, border_size);

    wxString pDispCatStrings[] = {
        _("Base"),
        _("Standard"),
        _("Other"),
        _("Mariners Standard")
    };
    pDispCat = new wxRadioBox( ps57Ctl, ID_RADIOBOX, _("Display Category"), wxDefaultPosition, wxDefaultSize,
                               4, pDispCatStrings, 1, wxRA_SPECIFY_COLS );
    itemBoxSizer75->Add(pDispCat, 0, wxALL|wxEXPAND, 2);



    pCheck_SOUNDG = new wxCheckBox( ps57Ctl, ID_SOUNDGCHECKBOX, _("Show Soundings"));
    pCheck_SOUNDG->SetValue(FALSE);
    itemBoxSizer75->Add(pCheck_SOUNDG, 1, wxALIGN_LEFT|wxALL|wxEXPAND, check_spacing_2);

    pCheck_META = new wxCheckBox( ps57Ctl, ID_METACHECKBOX, _("META Objects"));
    pCheck_META->SetValue(FALSE);
    itemBoxSizer75->Add(pCheck_META, 1, wxALIGN_LEFT|wxALL|wxEXPAND, check_spacing_2);

    pCheck_SHOWIMPTEXT = new wxCheckBox( ps57Ctl, ID_IMPTEXTCHECKBOX, _("Show Important Text Only") );
    pCheck_SHOWIMPTEXT->SetValue(FALSE);
    itemBoxSizer75->Add(pCheck_SHOWIMPTEXT, 1, wxALIGN_LEFT|wxALL|wxEXPAND, check_spacing_2);

    pCheck_SCAMIN = new wxCheckBox( ps57Ctl, ID_SCAMINCHECKBOX, _("Use SCAMIN"));
    pCheck_SCAMIN->SetValue(FALSE);
    itemBoxSizer75->Add(pCheck_SCAMIN, 1, wxALIGN_LEFT|wxALL|wxEXPAND, check_spacing_2);

    pCheck_ATONTEXT = new wxCheckBox( ps57Ctl, ID_ATONTEXTCHECKBOX, _("Show ATON labels"));
    pCheck_SCAMIN->SetValue(FALSE);
    itemBoxSizer75->Add(pCheck_ATONTEXT, 1, wxALIGN_LEFT|wxALL|wxEXPAND, check_spacing_2);

    pCheck_LDISTEXT = new wxCheckBox( ps57Ctl, ID_LDISTEXTCHECKBOX, _("Show Light Descriptions"));
    pCheck_LDISTEXT->SetValue(FALSE);
    itemBoxSizer75->Add(pCheck_LDISTEXT, 1, wxALIGN_LEFT|wxALL|wxEXPAND, check_spacing_2);

    pCheck_DECLTEXT = new wxCheckBox( ps57Ctl, ID_DECLTEXTCHECKBOX, _("De-Clutter Text (exp.)"));
    pCheck_DECLTEXT->SetValue(FALSE);
    itemBoxSizer75->Add(pCheck_DECLTEXT, 1, wxALIGN_LEFT|wxALL|wxEXPAND, check_spacing_2);

    wxStaticBox* itemStaticBoxSizer83Static = new wxStaticBox(ps57Ctl, wxID_ANY, _("Chart Display Style"));
    wxStaticBoxSizer* itemStaticBoxSizer83 = new wxStaticBoxSizer(itemStaticBoxSizer83Static, wxVERTICAL);
    itemStaticBoxSizer26->Add(itemStaticBoxSizer83, 1, wxALL|wxEXPAND, 2);

    wxString pPointStyleStrings[] = {
        _("Paper Chart"),
        _("Simplified"),
    };
    pPointStyle = new wxRadioBox( ps57Ctl, ID_RADIOBOX, _("Points"), wxDefaultPosition, wxDefaultSize,
                                  2, pPointStyleStrings, 1, wxRA_SPECIFY_COLS );
    itemStaticBoxSizer83->Add(pPointStyle, 0, wxALL|wxEXPAND, check_spacing_2);

    wxString pBoundStyleStrings[] = {
        _("Plain"),
        _("Symbolized"),
    };
    pBoundStyle = new wxRadioBox( ps57Ctl, ID_RADIOBOX, _("Boundaries"), wxDefaultPosition, wxDefaultSize,
                                              2, pBoundStyleStrings, 1, wxRA_SPECIFY_COLS );
    itemStaticBoxSizer83->Add(pBoundStyle, 0, wxALL|wxEXPAND, check_spacing_2);

    wxString pColorNumStrings[] = {
          _("2 Color"),
          _("4 Color"),
    };
    p24Color = new wxRadioBox( ps57Ctl, ID_RADIOBOX, _("Colors"), wxDefaultPosition, wxDefaultSize,
                               2, pColorNumStrings, 1, wxRA_SPECIFY_COLS );
    itemStaticBoxSizer83->Add(p24Color, 0, wxALL|wxEXPAND, check_spacing_2);


#ifdef USE_S57
    wxStaticBox *pslStatBox = new wxStaticBox(ps57Ctl, wxID_ANY, _("CM93 Zoom Detail"));
    wxStaticBoxSizer* itemStaticBoxSizersl = new wxStaticBoxSizer(pslStatBox, wxVERTICAL);
    itemStaticBoxSizer83->Add(itemStaticBoxSizersl, 0, wxALL|wxEXPAND, border_size);
    m_pSlider_CM93_Zoom = new wxSlider(ps57Ctl, ID_CM93ZOOM , 0, -CM93_ZOOM_FACTOR_MAX_RANGE, CM93_ZOOM_FACTOR_MAX_RANGE,
                                       wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);
    itemStaticBoxSizersl->Add(m_pSlider_CM93_Zoom, 0, wxALL|wxEXPAND, border_size);
#endif

    wxStaticBox*  pdepth_static = new wxStaticBox(ps57Ctl, wxID_ANY, _("Depth Settings"));
    wxStaticBoxSizer *pdepth_sizer = new wxStaticBoxSizer(pdepth_static, wxHORIZONTAL);
    itemBoxSizer25->Add(pdepth_sizer, 0, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 2);

    wxStaticBox* itemStaticBoxSizer27Static = new wxStaticBox(ps57Ctl, wxID_ANY, _("Depth Settings(m)"));
    wxStaticBoxSizer* itemStaticBoxSizer27 = new wxStaticBoxSizer(itemStaticBoxSizer27Static, wxVERTICAL);
    pdepth_sizer/*itemBoxSizer25*/->Add(itemStaticBoxSizer27, 0, wxTOP|wxALL|wxEXPAND, group_item_spacing);

    wxStaticText* itemStaticText4 = new wxStaticText( ps57Ctl, wxID_STATIC, _("Shallow Depth"));
    itemStaticBoxSizer27->Add(itemStaticText4, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, group_item_spacing);

    m_ShallowCtl = new wxTextCtrl( ps57Ctl, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxSize(120, -1), 0 );
    itemStaticBoxSizer27->Add(m_ShallowCtl, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, group_item_spacing);

    wxStaticText* itemStaticText5 = new wxStaticText( ps57Ctl, wxID_STATIC, _("Safety Depth"));
    itemStaticBoxSizer27->Add(itemStaticText5, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, group_item_spacing);

    m_SafetyCtl = new wxTextCtrl( ps57Ctl, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxSize(120, -1), 0 );
    itemStaticBoxSizer27->Add(m_SafetyCtl, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, group_item_spacing);

    wxStaticText* itemStaticText6 = new wxStaticText( ps57Ctl, wxID_STATIC, _("Deep Depth"));
    itemStaticBoxSizer27->Add(itemStaticText6, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, group_item_spacing);

    m_DeepCtl = new wxTextCtrl( ps57Ctl, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxSize(120, -1), 0 );
    itemStaticBoxSizer27->Add(m_DeepCtl, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, group_item_spacing);



    wxString pDepthUnitStrings[] = {
             _("Feet"),
             _("Metres"),
             _("Fathoms"),
    };

    pDepthUnitSelect = new wxRadioBox( ps57Ctl, ID_RADIOBOX, _("Chart Depth Units"), wxDefaultPosition, wxDefaultSize,
                               3, pDepthUnitStrings, 1, wxRA_SPECIFY_COLS );
    pdepth_sizer->Add(pDepthUnitSelect, 0, wxALIGN_TOP | wxALL, group_item_spacing);

#ifdef FLAV
       //      CM93Offset Enable
    wxStaticBox* itemStaticBoxCM93OffsetEnable = new wxStaticBox(ps57Ctl, wxID_ANY, _("CM93 Offsets"));
    wxStaticBoxSizer* itemStaticBoxSizerCM93OffsetEnable= new wxStaticBoxSizer(itemStaticBoxCM93OffsetEnable, wxVERTICAL);
    itemBoxSizer25->Add(itemStaticBoxSizerCM93OffsetEnable, 0, wxTOP|wxALL|wxEXPAND, border_size);

    //  Activate CM93Offset checkbox
    pSEnableCM93Offset = new wxCheckBox( ps57Ctl, ID_ENABLECM93OFFSET, _("Enable Manual CM93 Offsets"));
    itemStaticBoxSizerCM93OffsetEnable->Add(pSEnableCM93Offset, 1, wxALIGN_LEFT|wxALL, 2);
#endif


        //  Create "AIS" panel

    wxScrolledWindow *itemPanelAIS = CreatePanel(itemNotebook4, _("AIS"));

    wxBoxSizer* itemBoxSizer6AIS = new wxBoxSizer(wxVERTICAL);
    itemPanelAIS->SetSizer(itemBoxSizer6AIS);


   //      General
    wxStaticBox* itemStaticBoxAISGeneral = new wxStaticBox(itemPanelAIS, wxID_ANY, _("AIS General"));
    wxStaticBoxSizer* itemStaticBoxSizerAISGeneral= new wxStaticBoxSizer(itemStaticBoxAISGeneral, wxVERTICAL);
    itemBoxSizer6AIS->Add(itemStaticBoxSizerAISGeneral, 0, wxTOP|wxALL|wxEXPAND, border_size);

    //    Add AIS Data Input controls
    wxStaticBox* itemAISStaticBox = new wxStaticBox(itemPanelAIS, wxID_ANY, _("AIS Data Port"));
    wxStaticBoxSizer* itemAISStaticBoxSizer = new wxStaticBoxSizer(itemAISStaticBox, wxVERTICAL);
    itemStaticBoxSizerAISGeneral->Add(itemAISStaticBoxSizer, 0, wxEXPAND|wxALL, 4);

    m_itemAISListBox = new wxComboBox(itemPanelAIS, ID_CHOICE_AIS);
    m_itemAISListBox->Append( _T("None"));

      //    Fill in the listbox with all detected serial ports
    for (iPortIndex=0 ; iPortIndex < m_pSerialArray->GetCount() ; iPortIndex++)
          m_itemAISListBox->Append( m_pSerialArray->Item(iPortIndex) );


    int sidx;
    if(pAIS_Port->Upper().Contains(_T("SERIAL")))
    {
          wxString ais_com = pAIS_Port->Mid(7);
          sidx = m_itemAISListBox->FindString(ais_com);
    }
    else if(pAIS_Port->Upper().Contains(_T("NONE")))
          sidx = 0;
    else
          sidx = 0;                                 // malformed selection


    if(sidx ==  wxNOT_FOUND)                  // user specified in ComboBox
    {
          wxString nport = pAIS_Port->AfterFirst(':');
          m_itemAISListBox->Append( nport );
          sidx = m_itemAISListBox->FindString(nport);
    }

    m_itemAISListBox->SetSelection(sidx);

    itemAISStaticBoxSizer->Add(m_itemAISListBox, 0, wxEXPAND|wxALL, border_size);

    //      CPA Box
    wxStaticBox* itemStaticBoxCPA = new wxStaticBox(itemPanelAIS, wxID_ANY, _("CPA Calculation"));
    wxStaticBoxSizer* itemStaticBoxSizerCPA= new wxStaticBoxSizer(itemStaticBoxCPA, wxVERTICAL);
    itemBoxSizer6AIS->Add(itemStaticBoxSizerCPA, 0, wxALL|wxEXPAND, border_size);

    wxFlexGridSizer *pCPAGrid = new wxFlexGridSizer(2);
    pCPAGrid->AddGrowableCol(1);
    itemStaticBoxSizerCPA->Add(pCPAGrid, 0, wxALL|wxEXPAND, border_size);

    m_pCheck_CPA_Max = new wxCheckBox( itemPanelAIS, -1, _("No CPA Calculation if target range is greater than (NMi):"));
    pCPAGrid->Add(m_pCheck_CPA_Max, 0, wxALIGN_LEFT|wxALL, group_item_spacing);

    m_pText_CPA_Max = new wxTextCtrl(itemPanelAIS, -1);
    pCPAGrid->Add(m_pText_CPA_Max, 0, wxALIGN_RIGHT, group_item_spacing);

    m_pCheck_CPA_Warn = new wxCheckBox( itemPanelAIS, -1, _("Warn if CPA less than (NMi):"));
    pCPAGrid->Add(m_pCheck_CPA_Warn, 0, wxALIGN_LEFT|wxALL, group_item_spacing);

    m_pText_CPA_Warn = new wxTextCtrl(itemPanelAIS, -1, _T(""), wxDefaultPosition, wxSize(-1, -1));
    pCPAGrid->Add(m_pText_CPA_Warn, 0, wxALIGN_RIGHT, 2);

    m_pCheck_CPA_WarnT = new wxCheckBox( itemPanelAIS, -1, _("  ..And TCPA is less than (Minutes):"));
    pCPAGrid->Add(m_pCheck_CPA_WarnT, 0, wxALIGN_LEFT|wxALL, group_item_spacing);

    m_pText_CPA_WarnT = new wxTextCtrl(itemPanelAIS, -1);
    pCPAGrid->Add(m_pText_CPA_WarnT, 0, wxALIGN_RIGHT, group_item_spacing);

   //      Lost Targets
    wxStaticBox* itemStaticBoxLostTargets = new wxStaticBox(itemPanelAIS, wxID_ANY, _("Lost Targets"));
    wxStaticBoxSizer* itemStaticBoxSizerLostTargets= new wxStaticBoxSizer(itemStaticBoxLostTargets, wxVERTICAL);
    itemBoxSizer6AIS->Add(itemStaticBoxSizerLostTargets, 0, wxALL|wxEXPAND, 3);

    wxFlexGridSizer *pLostGrid = new wxFlexGridSizer(2);
    pLostGrid->AddGrowableCol(1);
    itemStaticBoxSizerLostTargets->Add(pLostGrid, 0, wxALL|wxEXPAND, border_size);

    m_pCheck_Mark_Lost = new wxCheckBox( itemPanelAIS, -1, _("Mark targets as lost after (Minutes:):"));
    pLostGrid->Add(m_pCheck_Mark_Lost, 1, wxALIGN_LEFT|wxALL, group_item_spacing);

    m_pText_Mark_Lost = new wxTextCtrl(itemPanelAIS, -1);
    pLostGrid->Add(m_pText_Mark_Lost, 1, wxALIGN_RIGHT, group_item_spacing);

    m_pCheck_Remove_Lost = new wxCheckBox( itemPanelAIS, -1, _("Remove lost targets after (Minutes:):"));
    pLostGrid->Add(m_pCheck_Remove_Lost, 1, wxALIGN_LEFT|wxALL, group_item_spacing);

    m_pText_Remove_Lost = new wxTextCtrl(itemPanelAIS, -1);
    pLostGrid->Add(m_pText_Remove_Lost, 1, wxALIGN_RIGHT, group_item_spacing);

   //      Display
    wxStaticBox* itemStaticBoxDisplay = new wxStaticBox(itemPanelAIS, wxID_ANY, _("Display"));
    wxStaticBoxSizer* itemStaticBoxSizerDisplay= new wxStaticBoxSizer(itemStaticBoxDisplay, wxHORIZONTAL);
    itemBoxSizer6AIS->Add(itemStaticBoxSizerDisplay, 0, wxALL|wxEXPAND, 3);

    wxFlexGridSizer *pDisplayGrid = new wxFlexGridSizer(2);
    pDisplayGrid->AddGrowableCol(1);
    itemStaticBoxSizerDisplay->Add(pDisplayGrid, 0, wxALL|wxEXPAND, border_size);

    m_pCheck_Show_COG = new wxCheckBox( itemPanelAIS, -1, _("Show target COG arrows"));
    pDisplayGrid->Add(m_pCheck_Show_COG, 1, wxALIGN_LEFT|wxALL, group_item_spacing);

    wxStaticText *pStatic_Dummy1 = new wxStaticText( itemPanelAIS, -1, _T(""));
    pDisplayGrid->Add(pStatic_Dummy1, 1, wxALIGN_RIGHT|wxALL, group_item_spacing);

    wxStaticText *pStatic_COG_Predictor = new wxStaticText( itemPanelAIS, -1, _("      COG arrow predictor length (Minutes):"));
    pDisplayGrid->Add(pStatic_COG_Predictor, 1, wxALIGN_LEFT|wxALL, group_item_spacing);

    m_pText_COG_Predictor = new wxTextCtrl(itemPanelAIS, -1);
    pDisplayGrid->Add(m_pText_COG_Predictor, 1, wxALIGN_RIGHT, group_item_spacing);

    m_pCheck_Show_Tracks = new wxCheckBox( itemPanelAIS, -1, _("Show target tracks"));
    pDisplayGrid->Add(m_pCheck_Show_Tracks, 1, wxALIGN_LEFT|wxALL, group_item_spacing);

    wxStaticText *pStatic_Dummy7 = new wxStaticText( itemPanelAIS, -1, _T(""));
    pDisplayGrid->Add(pStatic_Dummy7, 1, wxALIGN_RIGHT|wxALL, group_item_spacing);

    wxStaticText *pStatic_Track_Length = new wxStaticText( itemPanelAIS, -1, _("      Target track length (Minutes):"));
    pDisplayGrid->Add(pStatic_Track_Length, 1, wxALIGN_LEFT|wxALL, group_item_spacing);

    m_pText_Track_Length = new wxTextCtrl(itemPanelAIS, -1);
    pDisplayGrid->Add(m_pText_Track_Length, 1, wxALIGN_RIGHT, group_item_spacing);


    m_pCheck_Show_Moored = new wxCheckBox( itemPanelAIS, -1, _("Supress anchored/moored targets"));
    pDisplayGrid->Add(m_pCheck_Show_Moored, 1, wxALIGN_LEFT|wxALL, group_item_spacing);

    wxStaticText *pStatic_Dummy3 = new wxStaticText( itemPanelAIS, -1, _T(""));
    pDisplayGrid->Add(pStatic_Dummy3, 1, wxALIGN_RIGHT|wxALL, group_item_spacing);

    wxStaticText *pStatic_Moored_Speed = new wxStaticText( itemPanelAIS, -1, _("      Max moored target speed (Kts.):"));
    pDisplayGrid->Add(pStatic_Moored_Speed, 1, wxALIGN_LEFT|wxALL, group_item_spacing);

    m_pText_Moored_Speed = new wxTextCtrl(itemPanelAIS, -1);
    pDisplayGrid->Add(m_pText_Moored_Speed, 1, wxALIGN_RIGHT, group_item_spacing);

    // Rollover
    wxStaticBox* itemStaticBoxRollover = new wxStaticBox(itemPanelAIS, wxID_ANY, _("Rollover"));
    wxStaticBoxSizer* itemStaticBoxSizerRollover= new wxStaticBoxSizer(itemStaticBoxRollover, wxVERTICAL);
    itemStaticBoxSizerDisplay->Add(itemStaticBoxSizerRollover, 0, wxALL|wxEXPAND, 3);

    wxStaticText *pStatic_Dummy4 = new wxStaticText( itemPanelAIS, -1, _("      \"Ship Name\" MMSI (Call Sign)"));
    itemStaticBoxSizerRollover->Add(pStatic_Dummy4, 1, wxALIGN_LEFT|wxALL, group_item_spacing);

    m_pCheck_Rollover_Class = new wxCheckBox( itemPanelAIS, -1, _("[Class] Type (Status)"));
    itemStaticBoxSizerRollover->Add(m_pCheck_Rollover_Class, 1, wxALIGN_LEFT|wxALL, group_item_spacing);

    m_pCheck_Rollover_COG = new wxCheckBox( itemPanelAIS, -1, _("SOG COG"));
    itemStaticBoxSizerRollover->Add(m_pCheck_Rollover_COG, 1, wxALIGN_LEFT|wxALL, group_item_spacing);

    m_pCheck_Rollover_CPA = new wxCheckBox( itemPanelAIS, -1, _("CPA TCPA"));
    itemStaticBoxSizerRollover->Add(m_pCheck_Rollover_CPA, 1, wxALIGN_LEFT|wxALL, group_item_spacing);

        //      Alert Box
    wxStaticBox* itemStaticBoxAlert = new wxStaticBox(itemPanelAIS, wxID_ANY, _("CPA/TCPA Alerts"));
    wxStaticBoxSizer* itemStaticBoxSizerAlert= new wxStaticBoxSizer(itemStaticBoxAlert, wxVERTICAL);
    itemBoxSizer6AIS->Add(itemStaticBoxSizerAlert, 0, wxALL|wxEXPAND, border_size);

    wxFlexGridSizer *pAlertGrid = new wxFlexGridSizer(2);
    pAlertGrid->AddGrowableCol(1);
    itemStaticBoxSizerAlert->Add(pAlertGrid, 0, wxALL|wxEXPAND, border_size);

    m_pCheck_AlertDialog = new wxCheckBox( itemPanelAIS, ID_AISALERTDIALOG, _("Show CPA/TCPA Alert Dialog"));
    pAlertGrid->Add(m_pCheck_AlertDialog, 0, wxALIGN_LEFT|wxALL, group_item_spacing);

    wxButton *m_SelSound = new wxButton( itemPanelAIS, ID_AISALERTSELECTSOUND, _("Select Alert Sound"), wxDefaultPosition, small_button_size, 0 );
    pAlertGrid->Add(m_SelSound, 0, wxALIGN_RIGHT|wxALL, group_item_spacing);

    m_pCheck_AlertAudio = new wxCheckBox( itemPanelAIS, ID_AISALERTAUDIO, _("Play Sound on CPA/TCPA Alerts"));
    pAlertGrid->Add(m_pCheck_AlertAudio, 0, wxALIGN_LEFT|wxALL, group_item_spacing);

    wxButton *m_pPlay_Sound = new wxButton( itemPanelAIS, ID_AISALERTTESTSOUND, _("Test Alert Sound"), wxDefaultPosition, small_button_size, 0);
    pAlertGrid->Add(m_pPlay_Sound, 0, wxALIGN_RIGHT|wxALL, group_item_spacing);

    m_pCheck_Alert_Moored = new wxCheckBox( itemPanelAIS, -1, _("Supress Alerts for anchored/moored targets"));
    pAlertGrid->Add(m_pCheck_Alert_Moored, 1, wxALIGN_LEFT|wxALL, group_item_spacing);

    wxStaticText *pStatic_Dummy2 = new wxStaticText( itemPanelAIS, -1, _T(""));
    pAlertGrid->Add(pStatic_Dummy2, 1, wxALIGN_RIGHT|wxALL, group_item_spacing);

    m_pCheck_Ack_Timout = new wxCheckBox( itemPanelAIS, -1, _("Enable Target Alert Acknowledge timeout (minutes)"));
    pAlertGrid->Add(m_pCheck_Ack_Timout, 1, wxALIGN_LEFT|wxALL, group_item_spacing);

    m_pText_ACK_Timeout = new wxTextCtrl(itemPanelAIS, -1);
    pAlertGrid->Add(m_pText_ACK_Timeout, 1, wxALIGN_RIGHT, group_item_spacing);



    //      Build Language/Fonts panel

    itemPanelFont = CreatePanel(itemNotebook4,  _("Language/Fonts"));

    m_itemBoxSizerFontPanel = new wxBoxSizer(wxVERTICAL);
    itemPanelFont->SetSizer(m_itemBoxSizerFontPanel);


    //      Build Etc. Page

    wxScrolledWindow *itemPanelAdvanced = CreatePanel(itemNotebook4, _("Etc."));

    wxBoxSizer* itemBoxSizerAdvancedPanel = new wxBoxSizer(wxVERTICAL);
    itemPanelAdvanced->SetSizer(itemBoxSizerAdvancedPanel);

    //  Tracks
    wxStaticBox* itemStaticBoxSizerTrackStatic = new wxStaticBox(itemPanelAdvanced, wxID_ANY, _("Tracks"));
    wxStaticBoxSizer* itemStaticBoxSizerTrack = new wxStaticBoxSizer(itemStaticBoxSizerTrackStatic, wxVERTICAL);
    itemBoxSizerAdvancedPanel->Add(itemStaticBoxSizerTrack, 0, wxGROW|wxALL, border_size);
    pTrackShowIcon = new wxCheckBox( itemPanelAdvanced, ID_TRACKCHECKBOX, _("Show Track icon"));
    itemStaticBoxSizerTrack->Add(pTrackShowIcon, 1, wxALIGN_LEFT|wxALL, border_size);
    pTrackDaily = new wxCheckBox( itemPanelAdvanced, ID_DAILYCHECKBOX, _("Automatic Daily Tracks"));
    itemStaticBoxSizerTrack->Add(pTrackDaily, 1, wxALIGN_LEFT|wxALL, border_size);

    wxFlexGridSizer *pTrackGrid = new wxFlexGridSizer(2);
    pTrackGrid->AddGrowableCol(1);
    itemStaticBoxSizerTrack->Add(pTrackGrid, 0, wxALL|wxEXPAND, border_size);

    m_pCheck_Trackpoint_time = new wxRadioButton( itemPanelAdvanced, -1, _("Place Trackpoints at time interval (Seconds):"),
                wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    pTrackGrid->Add(m_pCheck_Trackpoint_time, 0, wxALIGN_LEFT|wxALL, 2);

    m_pText_TP_Secs = new wxTextCtrl(itemPanelAdvanced, -1);
    pTrackGrid->Add(m_pText_TP_Secs, 0, wxALIGN_RIGHT, 2);

    m_pCheck_Trackpoint_distance = new wxRadioButton( itemPanelAdvanced, -1, _("Place Trackpoints at distance interval (NMi):"));
    pTrackGrid->Add(m_pCheck_Trackpoint_distance, 0, wxALIGN_LEFT|wxALL, 2);

    m_pText_TP_Dist = new wxTextCtrl(itemPanelAdvanced, -1, _T(""));
    pTrackGrid->Add(m_pText_TP_Dist, 0, wxALIGN_RIGHT, 2);

    // Radar rings
    wxStaticBox* itemStaticBoxSizerRadarRingsStatic = new wxStaticBox(itemPanelAdvanced, wxID_ANY, _("Radar rings"));
    wxStaticBoxSizer* itemStaticBoxSizerRadarRings = new wxStaticBoxSizer(itemStaticBoxSizerRadarRingsStatic, wxVERTICAL);
    itemBoxSizerAdvancedPanel->Add(itemStaticBoxSizerRadarRings, 0, wxGROW|wxALL, border_size);

    pNavAidShowRadarRings = new wxCheckBox( itemPanelAdvanced, ID_GPXCHECKBOX, _("Show radar rings") );
    pNavAidShowRadarRings->SetValue(FALSE);
    itemStaticBoxSizerRadarRings->Add(pNavAidShowRadarRings, 1, wxALIGN_LEFT|wxALL, border_size);

    wxFlexGridSizer *pRadarGrid = new wxFlexGridSizer(2);
    pRadarGrid->AddGrowableCol(1);
    itemStaticBoxSizerRadarRings->Add(pRadarGrid, 0, wxALL|wxEXPAND, border_size);


    wxStaticText* itemStaticTextNumberRadarRings = new wxStaticText( itemPanelAdvanced, wxID_STATIC, _("Number of radar rings shown"));
    pRadarGrid->Add(itemStaticTextNumberRadarRings, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, border_size);

    pNavAidRadarRingsNumberVisible = new wxTextCtrl( itemPanelAdvanced, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxSize(100, -1), 0 );
    pRadarGrid->Add(pNavAidRadarRingsNumberVisible, 0, wxALIGN_RIGHT, 2);

    wxStaticText* itemStaticTextStepRadarRings = new wxStaticText( itemPanelAdvanced, wxID_STATIC, _("Distance between rings"));
    pRadarGrid->Add(itemStaticTextStepRadarRings, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, border_size);

    pNavAidRadarRingsStep = new wxTextCtrl( itemPanelAdvanced, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxSize(100, -1), 0 );
    pRadarGrid->Add(pNavAidRadarRingsStep, 0, wxALIGN_RIGHT, 2);

    wxString pDistUnitsStrings[] = {
          _("&Nautical miles"),
          _("&Kilometers")
    };
    m_itemNavAidRadarRingsStepUnitsRadioBox = new wxRadioBox( itemPanelAdvanced, ID_RADIOBOX, _("Units"), wxDefaultPosition, wxDefaultSize,
          2, pDistUnitsStrings, 1, wxRA_SPECIFY_ROWS );
    pRadarGrid->Add(m_itemNavAidRadarRingsStepUnitsRadioBox, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxBOTTOM, border_size);



    //  Disable/enable dragging waypoints while mark properties invisible
    wxStaticBox* itemStaticBoxSizerWptDraggingStatic = new wxStaticBox(itemPanelAdvanced, wxID_ANY, _("Waypoint Locking"));
    wxStaticBoxSizer* itemStaticBoxSizerWptDragging = new wxStaticBoxSizer(itemStaticBoxSizerWptDraggingStatic, wxVERTICAL);
    itemBoxSizerAdvancedPanel->Add(itemStaticBoxSizerWptDragging, 0, wxGROW|wxALL, border_size);
    pWayPointPreventDragging = new wxCheckBox( itemPanelAdvanced, ID_DRAGGINGCHECKBOX, _("Lock all waypoints unless a waypoint property dialog is visible"));
    pWayPointPreventDragging->SetValue(FALSE);
    itemStaticBoxSizerWptDragging->Add(pWayPointPreventDragging, 1, wxALIGN_LEFT|wxALL, border_size);


    //  Various GUI options
    wxStaticBox* itemStaticBoxSizerGUIOptionsStatic = new wxStaticBox(itemPanelAdvanced, wxID_ANY, _("GUI Options"));
    wxStaticBoxSizer* itemStaticBoxSizerGUIOption = new wxStaticBoxSizer(itemStaticBoxSizerGUIOptionsStatic, wxVERTICAL);
    itemBoxSizerAdvancedPanel->Add(itemStaticBoxSizerGUIOption, 0, wxGROW|wxALL, border_size);

    pEnableZoomToCursor = new wxCheckBox( itemPanelAdvanced, ID_ZTCCHECKBOX, _("Enable Wheel-Zoom-to-Cursor") );
    pEnableZoomToCursor->SetValue(FALSE);
    itemStaticBoxSizerGUIOption->Add(pEnableZoomToCursor, 1, wxALIGN_LEFT|wxALL, border_size);

    pPreserveScale = new wxCheckBox( itemPanelAdvanced, ID_PRESERVECHECKBOX, _("Preserve scale when switching charts"));
    itemStaticBoxSizerGUIOption->Add(pPreserveScale, 1, wxALIGN_LEFT|wxALL, border_size);

    pPlayShipsBells = new wxCheckBox( itemPanelAdvanced, ID_BELLSCHECKBOX, _("Play ships bells"));   // pjotrc 2010.02.09
    itemStaticBoxSizerGUIOption->Add(pPlayShipsBells, 1, wxALIGN_LEFT|wxALL, border_size);           // pjotrc 2010.02.09

    pFullScreenToolbar = new wxCheckBox( itemPanelAdvanced, ID_FSTOOLBARCHECKBOX, _("Show toolbar in fullscreen mode"));
    itemStaticBoxSizerGUIOption->Add(pFullScreenToolbar, 1, wxALIGN_LEFT|wxALL, border_size);

    pTransparentToolbar = new wxCheckBox( itemPanelAdvanced, ID_TRANSTOOLBARCHECKBOX, _("Enable transparent toolbar"));
    itemStaticBoxSizerGUIOption->Add(pTransparentToolbar, 1, wxALIGN_LEFT|wxALL, border_size);

    pShowLayers = new wxCheckBox( itemPanelAdvanced, ID_SHOWLAYERSCHECKBOX, _("Show layers initially"));
    itemStaticBoxSizerGUIOption->Add(pShowLayers, 1, wxALIGN_LEFT|wxALL, border_size);

    wxFlexGridSizer *pFormatGrid = new wxFlexGridSizer(2);
    pFormatGrid->AddGrowableCol(1);
    itemStaticBoxSizerGUIOption->Add(pFormatGrid, 0, wxALL|wxEXPAND, border_size);

    wxStaticText* itemStaticTextSDMMFormat = new wxStaticText( itemPanelAdvanced, wxID_STATIC, _("Show Lat/Long as"));
    pFormatGrid->Add(itemStaticTextSDMMFormat, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, border_size);

    wxString pSDMMFormats[] = {
          _("Degrees, Decimal minutes"),
          _("Decimal degrees"),
          _("Degrees, Minutes, Seconds")
    };
    int m_SDMMFormatsNChoices = sizeof( pSDMMFormats ) / sizeof( wxString );
    pSDMMFormat = new wxChoice( itemPanelAdvanced, ID_SDMMFORMATCHOICE, wxDefaultPosition, wxDefaultSize, m_SDMMFormatsNChoices,  pSDMMFormats);
    pFormatGrid->Add(pSDMMFormat, 0, wxALIGN_RIGHT, 2);



    //      Build the PlugIn Manager Panel
    m_pPlugInCtrl = new PluginListPanel( itemNotebook4, ID_PANELPIM, wxDefaultPosition, wxDefaultSize, g_pi_manager->GetPlugInArray() );
    m_pPlugInCtrl->SetScrollRate(1, 1);

    itemNotebook4->AddPage( m_pPlugInCtrl, _("PlugIns") );


    //      PlugIns can add panels, too
    if(g_pi_manager)
          g_pi_manager->AddAllPlugInToolboxPanels( itemNotebook4 );


    pSettingsCB1 = pDebugShowStat;

    SetColorScheme((ColorScheme)0);

}


void options::SetColorScheme(ColorScheme cs)
{
      SetBackgroundColour(GetGlobalColor(_T("DILG0")));

      wxColour back_color = GetGlobalColor(_T("DILG2"));
      wxColour text_color = GetGlobalColor(_T("DILG3"));

      SetControlColors(itemNotebook4, cs);
      SetControlColors(itemPanel5, cs);
      SetControlColors(itemPanel9, cs);
      SetControlColors(ps57Ctl, cs);
      SetControlColors(itemPanelFont, cs);
//      SetControlColors(pDirCtl, cs);
//      SetControlColors(pSelCtl, cs);
//      SetControlColors(pListBox, cs);

      SetControlColors(m_CancelButton, cs);
      SetControlColors(m_OKButton, cs);

}


void options::SetInitialSettings()
{
      wxString s;
      wxString dirname;

//      if(m_pCurrentDirList)
      {
            int nDir = m_CurrentDirList.GetCount();

            for(int i=0 ; i<nDir ; i++)
            {
                  dirname = m_CurrentDirList.Item(i).fullpath;
                  if(!dirname.IsEmpty())
                  {
                      if (pListBox)
                      {
                          pListBox->Append(dirname);
                      }
                  }
            }
      }

//    Settings too

      if(m_pConfig)
            pSettingsCB1->SetValue(m_pConfig->m_bShowDebugWindows);

      if(g_NMEALogWindow)
            pShowGPSWin->SetValue(true);

      if(g_bGarminHost)
            pGarminHost->SetValue(true);

      pFilterNMEA->SetValue(g_bfilter_cogsog);

      s.Printf(_T("%d"), g_COGFilterSec);
      pFilterSecs->SetValue(s);

      s.Printf(_T("%d"), g_COGAvgSec);
      pCOGUPUpdateSecs->SetValue(s);

      pPrintShowIcon->SetValue(g_bShowPrintIcon);
      pCDOOutlines->SetValue(g_bShowOutlines);
      pCDOQuilting->SetValue(g_bQuiltEnable);
      pFullScreenQuilt->SetValue(!g_bFullScreenQuilt);
      pSDepthUnits->SetValue(g_bShowDepthUnits);
      pSkewComp->SetValue(g_bskew_comp);
      pSDisplayGrid->SetValue(g_bDisplayGrid);

      pCBCourseUp->SetValue(g_bCourseUp);
      pCBLookAhead->SetValue(g_bLookAhead);

      if(fabs(wxRound(g_ownship_predictor_minutes) - g_ownship_predictor_minutes) > 1e-4)
            s.Printf(_T("%6.2f"), g_ownship_predictor_minutes);
      else
            s.Printf(_T("%4.0f"), g_ownship_predictor_minutes);
      m_pText_OSCOG_Predictor->SetValue(s);

      pNavAidShowRadarRings->SetValue(g_bNavAidShowRadarRings);   // toh, 2009.02.24
      wxString buf;
      buf.Printf(_T("%d"),g_iNavAidRadarRingsNumberVisible);
      pNavAidRadarRingsNumberVisible->SetValue(buf);  // toh, 2009.02.24
      buf.Printf(_T("%.3f"),g_fNavAidRadarRingsStep);
      pNavAidRadarRingsStep->SetValue(buf);     // toh, 2009.02.24
      m_itemNavAidRadarRingsStepUnitsRadioBox->SetSelection(g_pNavAidRadarRingsStepUnits);      // toh, 2009.02.24
      pWayPointPreventDragging->SetValue(g_bWayPointPreventDragging);   // toh, 2009.02.24

      pEnableZoomToCursor->SetValue(g_bEnableZoomToCursor);
      pPreserveScale->SetValue(g_bPreserveScaleOnX);
      pPlayShipsBells->SetValue(g_bPlayShipsBells);   // pjotrc 2010.02.09
      pFullScreenToolbar->SetValue(g_bFullscreenToolbar);
      pTransparentToolbar->SetValue(g_bTransparentToolbar);
      pShowLayers->SetValue(g_bShowLayers);
      pSDMMFormat->Select(g_iSDMMFormat);

      pTrackShowIcon->SetValue(g_bShowTrackIcon);
      pTrackDaily->SetValue(g_bTrackDaily);

      s.Printf(_T("%4.0f"),g_TrackIntervalSeconds);
      m_pText_TP_Secs->SetValue(s);
      s.Printf(_T("%5.2f"),g_TrackDeltaDistance);
      m_pText_TP_Dist->SetValue(s);

      m_pCheck_Trackpoint_time->SetValue(g_bTrackTime);
      m_pCheck_Trackpoint_distance->SetValue(g_bTrackDistance);



//    AIS Parameters

      //      CPA Box
      m_pCheck_CPA_Max->SetValue(g_bCPAMax);

      s.Printf(_T("%4.1f"),g_CPAMax_NM);
      m_pText_CPA_Max->SetValue(s);

      m_pCheck_CPA_Warn->SetValue(g_bCPAWarn);

      s.Printf(_T("%4.1f"),g_CPAWarn_NM);
      m_pText_CPA_Warn->SetValue(s);

      m_pCheck_CPA_WarnT->SetValue(g_bTCPA_Max);

      s.Printf(_T("%4.0f"),g_TCPA_Max);
      m_pText_CPA_WarnT->SetValue(s);

   //      Lost Targets
      m_pCheck_Mark_Lost->SetValue(g_bMarkLost);

      s.Printf(_T("%4.0f"),g_MarkLost_Mins);
      m_pText_Mark_Lost->SetValue(s);

      m_pCheck_Remove_Lost->SetValue(g_bRemoveLost);

      s.Printf(_T("%4.0f"),g_RemoveLost_Mins);
      m_pText_Remove_Lost->SetValue(s);

   //      Display
      m_pCheck_Show_COG->SetValue(g_bShowCOG);

      s.Printf(_T("%4.0f"),g_ShowCOG_Mins);
      m_pText_COG_Predictor->SetValue(s);


      m_pCheck_Show_Tracks->SetValue(g_bAISShowTracks);

      s.Printf(_T("%4.0f"),g_AISShowTracks_Mins);
      m_pText_Track_Length->SetValue(s);


      m_pCheck_Show_Moored->SetValue(!g_bShowMoored);

      s.Printf(_T("%4.1f"),g_ShowMoored_Kts);
      m_pText_Moored_Speed->SetValue(s);

         //      Alerts
      m_pCheck_AlertDialog->SetValue(g_bAIS_CPA_Alert);
      m_pCheck_AlertAudio->SetValue(g_bAIS_CPA_Alert_Audio);
      m_pCheck_Alert_Moored->SetValue(g_bAIS_CPA_Alert_Suppress_Moored);

      m_pCheck_Ack_Timout->SetValue(g_bAIS_ACK_Timeout);
      s.Printf(_T("%4.0f"),g_AckTimeout_Mins);
      m_pText_ACK_Timeout->SetValue(s);


      // Rollover
      m_pCheck_Rollover_Class->SetValue(g_bAISRolloverShowClass);
      m_pCheck_Rollover_COG->SetValue(g_bAISRolloverShowCOG);
      m_pCheck_Rollover_CPA->SetValue(g_bAISRolloverShowCPA);


#ifdef USE_S57
      m_pSlider_CM93_Zoom->SetValue(g_cm93_zoom_factor);

//    S52 Primary Filters
      ps57CtlListBox->Clear();

      for(unsigned int iPtr = 0 ; iPtr < ps52plib->pOBJLArray->GetCount() ; iPtr++)
      {
            OBJLElement *pOLE = (OBJLElement *)(ps52plib->pOBJLArray->Item(iPtr));

            ps57CtlListBox->Append(wxString(pOLE->OBJLName, wxConvUTF8));
            ps57CtlListBox->Check(ps57CtlListBox->GetCount()-1, !(pOLE->nViz == 0));
      }

//    Diplay Category
      if(ps52plib)
      {
            int nset = 2;                             // default OTHER

            switch(ps52plib->m_nDisplayCategory)
            {
            case (DISPLAYBASE):
                  nset = 0;
                  break;
            case (STANDARD):
                  nset = 1;
                  break;
            case (OTHER):
                  nset = 2;
                  break;
            case (MARINERS_STANDARD):
                  nset = 3;
                  break;
            default:
                    nset = 3;
                    break;
            }

            pDispCat->SetSelection(nset);


      ps57CtlListBox->Enable(MARINERS_STANDARD == ps52plib->m_nDisplayCategory);
      itemButtonClearList->Enable(MARINERS_STANDARD == ps52plib->m_nDisplayCategory);
      itemButtonSelectList->Enable(MARINERS_STANDARD == ps52plib->m_nDisplayCategory);

      //  Other Display Filters
      pCheck_SOUNDG->SetValue(ps52plib->m_bShowSoundg);
      pCheck_META->SetValue(ps52plib->m_bShowMeta);
      pCheck_SHOWIMPTEXT->SetValue(ps52plib->m_bShowS57ImportantTextOnly);
      pCheck_SCAMIN->SetValue(ps52plib->m_bUseSCAMIN);
      pCheck_ATONTEXT->SetValue(ps52plib->m_bShowAtonText);
      pCheck_LDISTEXT->SetValue(ps52plib->m_bShowLdisText);
      pCheck_DECLTEXT->SetValue(ps52plib->m_bDeClutterText);

     // Chart Display Style
      if(ps52plib->m_nSymbolStyle == PAPER_CHART)
          pPointStyle->SetSelection(0);
      else
          pPointStyle->SetSelection(1);

      if(ps52plib->m_nBoundaryStyle == PLAIN_BOUNDARIES)
          pBoundStyle->SetSelection(0);
      else
          pBoundStyle->SetSelection(1);

      if(S52_getMarinerParam(S52_MAR_TWO_SHADES) == 1.0)
            p24Color->SetSelection(0);
      else
            p24Color->SetSelection(1);

      wxString s;
      s.Printf(_T("%6.2f"),S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR));
      m_SafetyCtl->SetValue(s);

      s.Printf(_T("%6.2f"),S52_getMarinerParam(S52_MAR_SHALLOW_CONTOUR));
      m_ShallowCtl->SetValue(s);

      s.Printf(_T("%6.2f"),S52_getMarinerParam(S52_MAR_DEEP_CONTOUR));
      m_DeepCtl->SetValue(s);

      pDepthUnitSelect->SetSelection(ps52plib->m_nDepthUnitDisplay);
    }
#endif


}

void options::OnShowGpsWindowCheckboxClick( wxCommandEvent& event )
{
      if(!pShowGPSWin->GetValue())
      {
            if(g_NMEALogWindow)
                  g_NMEALogWindow->Destroy();
      }
      else
      {
            g_NMEALogWindow = new TTYWindow(pParent, 35);
            wxString com_string(m_itemNMEAListBox->GetValue());
            g_NMEALogWindow->SetTitle(com_string);
            g_NMEALogWindow->SetSize(g_NMEALogWindow_x,  g_NMEALogWindow_y,
                                     g_NMEALogWindow_sx, g_NMEALogWindow_sy);
            g_NMEALogWindow->Show();
      }
}

void options::OnDisplayCategoryRadioButton( wxCommandEvent& event)
{
   int select = pDispCat->GetSelection();

    if(3 == select)
    {
        ps57CtlListBox->Enable();
        itemButtonClearList->Enable();
        itemButtonSelectList->Enable();
    }

    else
    {
        ps57CtlListBox->Disable();
        itemButtonClearList->Disable();
        itemButtonSelectList->Disable();
    }

    event.Skip();
}

void options::OnButtonClearClick( wxCommandEvent& event )
{
    int nOBJL = ps57CtlListBox->GetCount();
    for( int iPtr = 0 ; iPtr < nOBJL ; iPtr++)
        ps57CtlListBox->Check(iPtr, false);

   event.Skip();
}


void options::OnButtonSelectClick( wxCommandEvent& event )
{
    int nOBJL = ps57CtlListBox->GetCount();
    for( int iPtr = 0 ; iPtr < nOBJL ; iPtr++)
        ps57CtlListBox->Check(iPtr, true);

    event.Skip();
}


void options::OnDirctrlSelChanged( wxTreeEvent& event )
{
    if(pDirCtl)
      {
            wxString SelDir;
            SelDir = pDirCtl->GetPath();
            if(pSelCtl)
            {
                pSelCtl->Clear();
                pSelCtl->AppendText(SelDir);
            }
      }

      event.Skip();
}


bool options::ShowToolTips()
{
    return TRUE;
}

void options::OnButtonaddClick( wxCommandEvent& event )
{
      wxString SelDir;
      SelDir = pDirCtl->GetPath();

      pListBox->Append(SelDir);

      k_charts |= CHANGE_CHARTS;

      event.Skip();
}


void options::UpdateWorkArrayFromTextCtl()
{
      wxString dirname;

      int n = pListBox->GetCount();
      if(m_pWorkDirList)
      {
            m_pWorkDirList->Clear();
            for(int i=0 ; i<n ; i++)
            {
                  dirname = pListBox->GetString(i);
                  if(!dirname.IsEmpty())
                  {
                              //    This is a fix for OSX, which appends EOL to results of GetLineText()
                        while (( dirname.Last() == wxChar(_T('\n')) ) || ( dirname.Last()  == wxChar(_T('\r')) ))
                              dirname.RemoveLast() ;

                              //    scan the current array to find a match
                              //    if found, add the info to the work list, preserving the magic number
                              //    If not found, make a new ChartDirInfo, and add it
                        bool b_added = false;
//                        if(m_pCurrentDirList)
                        {
                              int nDir = m_CurrentDirList.GetCount();

                              for(int i=0 ; i<nDir ; i++)
                              {
                                    if(m_CurrentDirList.Item(i).fullpath == dirname)
                                    {
                                          ChartDirInfo cdi = m_CurrentDirList.Item(i);
                                          m_pWorkDirList->Add(cdi);
                                          b_added = true;
                                          break;
                                    }
                              }
                        }
                        if(!b_added)
                        {
                              ChartDirInfo cdin;
                              cdin.fullpath = dirname;
                              m_pWorkDirList->Add(cdin);
                        }
                  }
            }
      }
}


void options::OnXidOkClick( wxCommandEvent& event )
{
      int iret = 0;

//    Handle Chart Tab
      wxString dirname;

      if(pListBox)
      {
            UpdateWorkArrayFromTextCtl();
      }
      else
      {
//          if(m_pCurrentDirList)
          {
              m_pWorkDirList->Clear();
              int nDir = m_CurrentDirList.GetCount();

              for(int i=0 ; i<nDir ; i++)
              {
                    ChartDirInfo cdi = m_CurrentDirList.Item(i);
                    m_pWorkDirList->Add(cdi);
              }
          }
      }

      int k_force = FORCE_UPDATE;
      if(pUpdateCheckBox)
      {
            if(!pUpdateCheckBox->GetValue())
                  k_force = 0;
      }
      else
            k_force = 0;

      iret |= k_force;

//    Handle Settings Tab

      if(m_pConfig)
           m_pConfig->m_bShowDebugWindows = pSettingsCB1->GetValue();

    g_bGarminHost = pGarminHost->GetValue();

    g_bShowPrintIcon = pPrintShowIcon->GetValue();
    g_bShowOutlines = pCDOOutlines->GetValue();
    g_bDisplayGrid = pSDisplayGrid->GetValue();

    g_bQuiltEnable = pCDOQuilting->GetValue();
    g_bFullScreenQuilt = !pFullScreenQuilt->GetValue();

    g_bShowDepthUnits = pSDepthUnits->GetValue();
    g_bskew_comp = pSkewComp->GetValue();

    g_bfilter_cogsog = pFilterNMEA->GetValue();

    long filter_val = 1;
    pFilterSecs->GetValue().ToLong(&filter_val);
    g_COGFilterSec = wxMin((int)filter_val, MAX_COGSOG_FILTER_SECONDS);
    g_COGFilterSec = wxMax(g_COGFilterSec, 1);
    g_SOGFilterSec = g_COGFilterSec;

    long update_val = 1;
    pCOGUPUpdateSecs->GetValue().ToLong(&update_val);
    g_COGAvgSec = wxMin((int)update_val, MAX_COG_AVERAGE_SECONDS);

//    g_bAutoAnchorMark = pAutoAnchorMark->GetValue();

    g_bCourseUp = pCBCourseUp->GetValue();
    g_bLookAhead = pCBLookAhead->GetValue();

    m_pText_OSCOG_Predictor->GetValue().ToDouble(&g_ownship_predictor_minutes);


    g_bNavAidShowRadarRings = pNavAidShowRadarRings->GetValue();
    wxString buf = pNavAidRadarRingsNumberVisible->GetValue();
    g_iNavAidRadarRingsNumberVisible = atoi(buf.mb_str());
    g_fNavAidRadarRingsStep = atof(pNavAidRadarRingsStep->GetValue().mb_str());
    g_pNavAidRadarRingsStepUnits = m_itemNavAidRadarRingsStepUnitsRadioBox->GetSelection();
    g_bWayPointPreventDragging = pWayPointPreventDragging->GetValue();

    g_bPreserveScaleOnX = pPreserveScale->GetValue();

    g_bPlayShipsBells = pPlayShipsBells->GetValue();   // pjotrc 2010.02.09
    g_bFullscreenToolbar = pFullScreenToolbar->GetValue();
    g_bTransparentToolbar = pTransparentToolbar->GetValue();
    g_bShowLayers = pShowLayers->GetValue();
    g_iSDMMFormat = pSDMMFormat->GetSelection();

    g_bShowTrackIcon = pTrackShowIcon->GetValue();
    m_pText_TP_Secs->GetValue().ToDouble(&g_TrackIntervalSeconds);
    m_pText_TP_Dist->GetValue().ToDouble(&g_TrackDeltaDistance);
    g_bTrackTime = m_pCheck_Trackpoint_time->GetValue();
    g_bTrackDistance = m_pCheck_Trackpoint_distance->GetValue();

    g_bTrackDaily = pTrackDaily->GetValue();

    g_bEnableZoomToCursor = pEnableZoomToCursor->GetValue();

    //    AIS Parameters
      //      CPA Box
    g_bCPAMax = m_pCheck_CPA_Max->GetValue();
    m_pText_CPA_Max->GetValue().ToDouble(&g_CPAMax_NM);
    g_bCPAWarn = m_pCheck_CPA_Warn->GetValue();
    m_pText_CPA_Warn->GetValue().ToDouble(&g_CPAWarn_NM);
    g_bTCPA_Max = m_pCheck_CPA_WarnT->GetValue();
    m_pText_CPA_WarnT->GetValue().ToDouble(&g_TCPA_Max);

   //      Lost Targets
    g_bMarkLost = m_pCheck_Mark_Lost->GetValue();
    m_pText_Mark_Lost->GetValue().ToDouble(&g_MarkLost_Mins);
    g_bRemoveLost = m_pCheck_Remove_Lost->GetValue();
    m_pText_Remove_Lost->GetValue().ToDouble(&g_RemoveLost_Mins);

   //      Display
    g_bShowCOG = m_pCheck_Show_COG->GetValue();
    m_pText_COG_Predictor->GetValue().ToDouble(&g_ShowCOG_Mins);

    g_bAISShowTracks = m_pCheck_Show_Tracks->GetValue();
    m_pText_Track_Length->GetValue().ToDouble(&g_AISShowTracks_Mins);

    g_bShowMoored = !m_pCheck_Show_Moored->GetValue();
    m_pText_Moored_Speed->GetValue().ToDouble(&g_ShowMoored_Kts);

    //      Alert
    g_bAIS_CPA_Alert = m_pCheck_AlertDialog->GetValue();
    g_bAIS_CPA_Alert_Audio = m_pCheck_AlertAudio->GetValue();
    g_bAIS_CPA_Alert_Suppress_Moored = m_pCheck_Alert_Moored->GetValue();

    g_bAIS_ACK_Timeout = m_pCheck_Ack_Timout->GetValue();
    m_pText_ACK_Timeout->GetValue().ToDouble(&g_AckTimeout_Mins);

      // Rollover
      g_bAISRolloverShowClass = m_pCheck_Rollover_Class->GetValue();
      g_bAISRolloverShowCOG = m_pCheck_Rollover_COG->GetValue();
      g_bAISRolloverShowCPA = m_pCheck_Rollover_CPA->GetValue();


//    NMEA Options

// Source
//      wxString sel(m_itemNMEAListBox->GetStringSelection());
      wxString sel(m_itemNMEAListBox->GetValue());

      if(sel.Contains(_T("COM")))
          sel.Prepend(_T("Serial:"));

      else if(sel.Contains(_T("/dev")))
          sel.Prepend(_T("Serial:"));

      else if(sel.Contains(_T("AIS")))
            sel.Prepend(_T("Serial:"));

      else if(sel.Contains(_T("LIBGPS")))
      {
            sel.Empty();
            sel.Append(_T("LIBGPS:"));
            sel.Append(m_itemNMEA_TCPIP_Source->GetLineText(0));
      }

      else if(sel.Contains(_T("GPSD")))
      {
            sel.Empty();
            sel.Append(_T("GPSD:"));
            sel.Append(m_itemNMEA_TCPIP_Source->GetLineText(0));
      }
    *pNMEADataSource = sel;

      //    If the selection is anything other than "GARMIN",
      //    then disable semipermanently the option to select GARMIN in future.
      //    Note if GARMIN device is found in the future, the option will be
      //    re-enabled.
      if(!sel.Contains(_T("GARMIN")))
            g_bGarminPersistance = false;

      //    Primary NMEA Baud Rate
      g_NMEABaudRate = m_itemNMEABaudListBox->GetValue();

// AP Output
    wxString selp(m_itemNMEAAutoListBox->GetStringSelection());
    if(selp.Contains(_T("COM")))
        selp.Prepend(_T("Serial:"));
    else if(selp.Contains(_T("/dev")))
        selp.Prepend(_T("Serial:"));
    *pNMEA_AP_Port = selp;

// AIS Input
//    wxString selais(m_itemAISListBox->GetStringSelection());
    wxString selais(m_itemAISListBox->GetValue());
    if(selais.Contains(_T("COM")))
        selais.Prepend(_T("Serial:"));
    else if(selais.Contains(_T("/dev")))
        selais.Prepend(_T("Serial:"));
    else if(selais.Contains(_T("fifo")))
          selais.Prepend(_T("Serial:"));
    *pAIS_Port = selais;

#ifdef USE_WIFI_CLIENT
// WiFi
    wxString WiFiSource;
    WiFiSource.Empty();
    WiFiSource.Append(_T("TCP/IP:"));
    WiFiSource.Append(m_itemWIFI_TCPIP_Source->GetLineText(0));
    *pWIFIServerName = WiFiSource;
#endif


#ifdef USE_S57
      //    Handle Vector Charts Tab

      g_cm93_zoom_factor = m_pSlider_CM93_Zoom->GetValue();

      int nOBJL = ps57CtlListBox->GetCount();

      for( int iPtr = 0 ; iPtr < nOBJL ; iPtr++)
      {
            OBJLElement *pOLE = (OBJLElement *)(ps52plib->pOBJLArray->Item(iPtr));

            pOLE->nViz = ps57CtlListBox->IsChecked(iPtr);
    }


      if(ps52plib)
      {
            enum _DisCat nset = OTHER;
            switch(pDispCat->GetSelection())
            {
            case 0:
                  nset = DISPLAYBASE;
                  break;
            case 1:
                  nset = STANDARD;
                  break;
            case 2:
                  nset = OTHER;
                  break;
            case 3:
                  nset = MARINERS_STANDARD;
                  break;
            }
            ps52plib->m_nDisplayCategory = nset;

            ps52plib->m_bShowSoundg =  pCheck_SOUNDG->GetValue();
            ps52plib->m_bShowMeta =    pCheck_META->GetValue();
            ps52plib->m_bShowS57ImportantTextOnly = pCheck_SHOWIMPTEXT->GetValue();
            ps52plib->m_bUseSCAMIN =   pCheck_SCAMIN->GetValue();
            ps52plib->m_bShowAtonText= pCheck_ATONTEXT->GetValue();
            ps52plib->m_bShowLdisText= pCheck_LDISTEXT->GetValue();
            ps52plib->m_bDeClutterText= pCheck_DECLTEXT->GetValue();


            if(0 == pPointStyle->GetSelection())
                ps52plib->m_nSymbolStyle = PAPER_CHART;
            else
                ps52plib->m_nSymbolStyle = SIMPLIFIED;

            if(0 == pBoundStyle->GetSelection())
                ps52plib->m_nBoundaryStyle = PLAIN_BOUNDARIES;
            else
                ps52plib->m_nBoundaryStyle = SYMBOLIZED_BOUNDARIES;

            if(0 == p24Color->GetSelection())
                  S52_setMarinerParam(S52_MAR_TWO_SHADES, 1.0);
            else
                  S52_setMarinerParam(S52_MAR_TWO_SHADES, 0.0);

            double dval;

            if((m_SafetyCtl->GetValue()).ToDouble(&dval))
            {
                  S52_setMarinerParam(S52_MAR_SAFETY_DEPTH, dval);            // controls sounding display
                  S52_setMarinerParam(S52_MAR_SAFETY_CONTOUR, dval);          // controls colour
            }

            if((m_ShallowCtl->GetValue()).ToDouble(&dval))
                  S52_setMarinerParam(S52_MAR_SHALLOW_CONTOUR, dval);

            if((m_DeepCtl->GetValue()).ToDouble(&dval))
                  S52_setMarinerParam(S52_MAR_DEEP_CONTOUR, dval);


            ps52plib->UpdateMarinerParams();

            ps52plib->m_nDepthUnitDisplay = pDepthUnitSelect->GetSelection();

            ps52plib->GenerateStateHash();
      }
#endif

//      Capture and store the currently selected chart tree path
    if(pDirCtl != NULL)
    {
        wxString cur_path = pDirCtl->GetPath();
        pInit_Chart_Dir->Clear();
        pInit_Chart_Dir->Append(cur_path);
    }



//    Language Panel
    if(m_bVisitLang)
    {
            wxString new_canon = _T("en_US");
            wxString lang_sel = m_itemLangListBox->GetValue();

            int nLang = sizeof(lang_list)/sizeof(int);
            for( int it = 0 ; it < nLang ; it++)
            {
                  wxString lang_canonical = wxLocale::GetLanguageInfo(lang_list[it])->CanonicalName;
                  wxString test_string = GetOCPNKnownLanguage(lang_canonical, NULL);
                  if(lang_sel == test_string)
                  {
                        new_canon = lang_canonical;
                        break;
                  }
            }

            wxString locale_old = g_locale;
            g_locale = new_canon;

            if(g_locale != locale_old)
                  iret |= LOCALE_CHANGED;
    }

    //      PlugIn Manager Panel

    //      Pick up any changes to selections
    bool bnew_settings = g_pi_manager->UpdatePlugIns();
    if(bnew_settings)
          iret |= TOOLBAR_CHANGED;


    //      And keep config in sync
    g_pi_manager->UpdateConfig();

    //      PlugIns may have added panels
    if(g_pi_manager)
          g_pi_manager->CloseAllPlugInPanels( (int) wxOK );


    //      Could be a lot smarter here
    iret |= GENERIC_CHANGED;
    iret |= S52_CHANGED;
    iret |= k_charts;

    EndModal(iret);

}


void options::OnButtondeleteClick( wxCommandEvent& event )
{

      wxString dirname;

      wxArrayInt pListBoxSelections;
      pListBox->GetSelections(pListBoxSelections);
      int nSelections = pListBoxSelections.GetCount();
      for (int i=0 ; i<nSelections ; i++)
      {
            pListBox->Delete(pListBoxSelections.Item((nSelections - i) - 1));
      }

      UpdateWorkArrayFromTextCtl();

      if(m_pWorkDirList)
      {
            pListBox->Clear();

            int nDir = m_pWorkDirList->GetCount();
            for(int id=0 ; id<nDir ; id++)
            {
                  dirname = m_pWorkDirList->Item(id).fullpath;
                  if(!dirname.IsEmpty())
                  {
                        pListBox->Append(dirname);
                  }
            }
      }

      k_charts |= CHANGE_CHARTS;

      event.Skip();
}


void options::OnDebugcheckbox1Click( wxCommandEvent& event )
{
    event.Skip();
}



void options::OnCancelClick( wxCommandEvent& event )
{
      EndModal(0);
}


void options::OnChooseFont( wxCommandEvent& event )
{
      wxString sel_text_element = m_itemFontElementListBox->GetStringSelection();

      wxFont *psfont;
      wxFontData font_data;

      wxFont *pif = pFontMgr->GetFont(sel_text_element);
      wxColour init_color = pFontMgr->GetFontColor(sel_text_element);

      wxFontData init_font_data;
      if(pif)
            init_font_data.SetInitialFont(*pif);
      init_font_data.SetColour(init_color);


#ifdef __WXX11__
      X11FontPicker dg(pParent, init_font_data);
#else
      wxFontDialog dg(pParent, init_font_data);
#endif
      int retval = dg.ShowModal();
      if(wxID_CANCEL != retval)
      {
            font_data = dg.GetFontData();
            wxFont font = font_data.GetChosenFont();
            psfont = new wxFont(font);
            wxColor color = font_data.GetColour();
            pFontMgr->SetFont(sel_text_element, psfont, color);

            pParent->UpdateAllFonts();
      }

      event.Skip();
}


void options::CreateChartsPage()
{
      int display_width, display_height;
      wxDisplaySize(&display_width, &display_height);

      itemBoxSizer10->Clear(true);
      pSelCtl = NULL;
      pDirCtl = NULL;

          //    "Available" tree control and selection
      wxStaticBox* itemStaticBoxSizer11Static = new wxStaticBox(itemPanel9, wxID_ANY,
                  _("Available Chart Directories"));
      itemStaticBoxSizer11 = new wxStaticBoxSizer(itemStaticBoxSizer11Static, wxVERTICAL);
      itemBoxSizer10->Add(itemStaticBoxSizer11, 1, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 5);

      if(display_height < 800)
            itemStaticBoxSizer11->SetMinSize(wxSize(-1, 220));
      else
            itemStaticBoxSizer11->SetMinSize(wxSize(-1, 260));


          //    "Active" list
      itemActiveChartStaticBox = new wxStaticBox(itemPanel9, wxID_ANY, _("Active Chart Directories"));
      wxStaticBoxSizer* itemStaticBoxSizer16 = new wxStaticBoxSizer(itemActiveChartStaticBox, wxVERTICAL);
      itemBoxSizer10->Add(itemStaticBoxSizer16, 1, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 5);


      wxString* pListBoxStrings = NULL;
      pListBox = new wxListBox(itemPanel9, ID_LISTBOX, wxDefaultPosition, wxSize(-1, -1), 0,
                               pListBoxStrings, wxLB_MULTIPLE);

      if(display_height < 1024)
            pListBox->SetMinSize(wxSize(-1, 100));
      else
            pListBox->SetMinSize(wxSize(-1, 150));

      itemStaticBoxSizer16->Add(pListBox, 1, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 2);

      wxButton* itemButton18 = new wxButton( itemPanel9, ID_BUTTONDELETE, _("Delete Selection") );
      itemStaticBoxSizer16->Add(itemButton18, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 2);


      wxStaticBox* itemStaticBoxUpdateStatic = new wxStaticBox(itemPanel9, wxID_ANY, _("Update Control"));
      wxStaticBoxSizer* itemStaticBoxSizerUpdate = new wxStaticBoxSizer(itemStaticBoxUpdateStatic, wxVERTICAL);
      itemBoxSizer10->Add(itemStaticBoxSizerUpdate, 0, wxGROW|wxALL, 5);
      pUpdateCheckBox = new wxCheckBox( itemPanel9, ID_UPDCHECKBOX, _("Force Full Database Rebuild") );
      itemStaticBoxSizerUpdate->Add(pUpdateCheckBox, 1, wxALIGN_LEFT|wxALL, 5);


          //      Establish control colors on deferred creation
      SetControlColors(pSelCtl, (ColorScheme)0);
      SetControlColors(pListBox, (ColorScheme)0);

      itemBoxSizer10->Layout();
}

void options::PopulateChartsPage()
{
      itemStaticBoxSizer11->Clear(true);


      pDirCtl = new wxGenericDirCtrl( itemPanel9, ID_DIRCTRL, m_init_chart_dir, wxDefaultPosition,
                                      wxDefaultSize, 0, _("All files (*.*)|*.*"), 0 );
      itemStaticBoxSizer11->Add(pDirCtl, 1, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 5);


      pSelCtl = new wxTextCtrl( itemPanel9, ID_TEXTCTRL1, _T(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
      itemStaticBoxSizer11->Add(pSelCtl, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 2);

      wxBoxSizer* itemBoxSizer14 = new wxBoxSizer(wxHORIZONTAL);
      itemStaticBoxSizer11->Add(itemBoxSizer14, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);
      wxButton* itemButton15 = new wxButton( itemPanel9, ID_BUTTONADD, _("Add Selection") );
      itemBoxSizer14->Add(itemButton15, 0, wxALIGN_CENTER_VERTICAL|wxALL, 2);


          //      Establish control colors on deferred creation
      SetControlColors(pDirCtl, (ColorScheme)0);

          //        Fill in the control variable data

          //        Currently selected chart dirs
      wxString dirname;
      if(/*m_pCurrentDirList && */pListBox)
      {
            pListBox->Clear();
            int nDir = m_CurrentDirList.GetCount();
            for(int i=0 ; i<nDir ; i++)
            {
                  dirname = m_CurrentDirList.Item(i).fullpath;
                  if(!dirname.IsEmpty())
                        pListBox->Append(dirname);
            }
      }

          //        Selected Directory
      wxString SelDir;
      SelDir = pDirCtl->GetPath();
      pSelCtl->Clear();
      pSelCtl->SetInsertionPoint(0);
 //     pSelCtl->AppendText(SelDir);

      itemBoxSizer10->Layout();
}





void options::OnPageChange(wxNotebookEvent& event)
{
      int i = event.GetSelection();

      //    User selected Chart Page?
      //    If so, build the "Charts" page variants
      if(2 == i)                        // 2 is the index of "Charts" page
      {
            if(!k_charts)
                  PopulateChartsPage();

            k_charts = VISIT_CHARTS;
      }


      else if(5 == i)                        // 5 is the index of "Language/Fonts" page
      {
            if(!m_bVisitLang)
            {
                 ::wxBeginBusyCursor();

                  int border_size = 4;

                  wxStaticBox* itemLangStaticBox = new wxStaticBox(itemPanelFont, wxID_ANY, _("Language"));
                  wxStaticBoxSizer* itemLangStaticBoxSizer = new wxStaticBoxSizer(itemLangStaticBox, wxVERTICAL);
                  m_itemBoxSizerFontPanel->Add(itemLangStaticBoxSizer, 0, wxEXPAND|wxALL, border_size);

                  m_itemLangListBox = new wxComboBox(itemPanelFont, ID_CHOICE_LANG);

                  int current_language = locale_def_lang.GetLanguage();
//                  wxString oldLocale/*wxMB2WXbuf oldLocale*/ = wxSetlocale(LC_ALL, wxEmptyString);  //2.9.1
                  wxString current_sel = wxLocale::GetLanguageName(current_language);

                  current_sel = GetOCPNKnownLanguage(g_locale, NULL);


                  int nLang = sizeof(lang_list)/sizeof(int);


#ifdef __WXMSW__
                   // always add us english
                  m_itemLangListBox->Append(_T("English (U.S.)"));

                  wxString lang_dir = g_SData_Locn + _T("share/locale/");
                  for( int it = 1 ; it < nLang ; it++)
                  {
                        if(wxLocale::IsAvailable(lang_list[it]))
                        {
                              wxLocale ltest(lang_list[it], 0);
                              ltest.AddCatalog(_T("opencpn"));
                              if(!ltest.IsLoaded(_T("opencpn")))
                                    continue;

            // Defaults
                              wxString loc_lang_name = wxLocale::GetLanguageName(lang_list[it]);
                              wxString widgets_lang_name = loc_lang_name;
                              wxString lang_canonical = wxLocale::GetLanguageInfo(lang_list[it])->CanonicalName;

            //  Make opencpn substitutions
                              wxString lang_suffix;
                              loc_lang_name = GetOCPNKnownLanguage(lang_canonical, &lang_suffix);

            //  Look explicitely to see if .mo is available
                              wxString test_dir = lang_dir + lang_suffix;
                              if(!wxDir::Exists(test_dir))
                                    continue;

                              m_itemLangListBox->Append(loc_lang_name);
                        }
                  }
#else
                  wxArrayString lang_array;

                   // always add us english
                  lang_array.Add(_T("en_US"));

                  for( int it = 0 ; it < nLang ; it++)
                  {
//                        if(wxLocale::IsAvailable(lang_list[it]))
                        {
                              wxLocale ltest(lang_list[it], 0);
                              ltest.AddCatalog(_T("opencpn"));

                              if(ltest.IsLoaded(_T("opencpn")))
                              {
                                    wxString s0 = wxLocale::GetLanguageInfo(lang_list[it])->CanonicalName;
                                    wxString sl = wxLocale::GetLanguageName(lang_list[it]);
                                    if(wxNOT_FOUND == lang_array.Index(s0))
                                          lang_array.Add(s0);

                              }
                        }
                  }

                  for(unsigned int i=0 ; i < lang_array.GetCount() ; i++)
                  {
                        //  Make opencpn substitutions
                        wxString loc_lang_name = GetOCPNKnownLanguage(lang_array[i], NULL);
                        m_itemLangListBox->Append( loc_lang_name );
                  }
#endif


                  //    Reset current language
                  setlocale(LC_NUMERIC,"C");

//                  wxSetlocale(LC_ALL, oldLocale);
//                  wxSetlocale(LC_NUMERIC,wxString(_T("C")));

                  m_itemLangListBox->SetStringSelection(current_sel); //SetSelection(0);

                  itemLangStaticBoxSizer->Add(m_itemLangListBox, 0, wxALL, border_size);


                  wxStaticBox* itemFontStaticBox = new wxStaticBox(itemPanelFont, wxID_ANY, _("Font Options"));
                  wxStaticBoxSizer* itemFontStaticBoxSizer = new wxStaticBoxSizer(itemFontStaticBox, wxVERTICAL);
                  m_itemBoxSizerFontPanel->Add(itemFontStaticBoxSizer, 0, wxEXPAND|wxALL, border_size);

                  wxStaticBox* itemFontElementStaticBox = new wxStaticBox(itemPanelFont, wxID_ANY, _("Text Element"));
                  wxStaticBoxSizer* itemFontElementStaticBoxSizer = new wxStaticBoxSizer(itemFontElementStaticBox, wxVERTICAL);
                  itemFontStaticBoxSizer->Add(itemFontElementStaticBoxSizer, 0, wxEXPAND|wxALL, border_size);

                  m_itemFontElementListBox = new wxComboBox(itemPanelFont, ID_CHOICE_FONTELEMENT);

                  int nFonts = pFontMgr->GetNumFonts();
                  for( int it = 0 ; it < nFonts ; it++)
                  {
                        wxString *t = pFontMgr->GetDialogString(it);
                        m_itemFontElementListBox->Append( *t );
                  }

                  if(nFonts)
                        m_itemFontElementListBox->SetSelection(0);

                  itemFontElementStaticBoxSizer->Add(m_itemFontElementListBox, 0, wxEXPAND|wxALL, border_size);

                  wxButton* itemFontChooseButton = new wxButton( itemPanelFont, ID_BUTTONFONTCHOOSE, _("Choose Font..."),
                              wxDefaultPosition, wxDefaultSize, 0 );
                  itemFontElementStaticBoxSizer->Add(itemFontChooseButton, 0, wxEXPAND|wxALL, border_size);

                  //      Initialize Language tab
                  const wxLanguageInfo *pli = wxLocale::FindLanguageInfo(g_locale);
                  if(pli)
                  {
                        wxString clang = pli->Description;
//                        m_itemLangListBox->SetValue(clang);
                  }

                  m_bVisitLang = true;

                  ::wxEndBusyCursor();

                  m_itemBoxSizerFontPanel->Layout();

            }
      }
}


void options::OnNMEASourceChoice(wxCommandEvent& event)
{
    int i = event.GetSelection();
    wxString src(m_itemNMEAListBox->GetString(i));
    if((src.Upper().Find(_T("GPSD")) != wxNOT_FOUND) || (src.Upper().Find(_T("LIBGPS")) != wxNOT_FOUND))
     {
        m_itemNMEA_TCPIP_StaticBox->Enable();
        m_itemNMEA_TCPIP_Source->Enable();

        m_itemNMEA_TCPIP_Source->Clear();
        m_itemNMEA_TCPIP_Source->WriteText(_T("localhost")); // default

        wxString source;
        source = *pNMEADataSource;
        if(source.Upper().StartsWith(_T("GPSD")) || source.Upper().StartsWith(_T("LIBGPS")))
        {
            wxString ip;
            ip = source.AfterFirst(':');

            if(ip.Len())
            {
                m_itemNMEA_TCPIP_Source->Clear();
                m_itemNMEA_TCPIP_Source->WriteText(ip);
            }
        }
    }
    else
    {
        m_itemNMEA_TCPIP_StaticBox->Disable();
        m_itemNMEA_TCPIP_Source->Disable();
    }
}

void options::SetControlColors(wxWindow *ctrl, ColorScheme cs)
{
      if(NULL != ctrl)
      {
            ctrl->SetBackgroundColour(GetGlobalColor(_T("DILG0")));

            wxColour back_color =GetGlobalColor(_T("DILG2"));
            wxColour text_color = GetGlobalColor(_T("DILG3"));

            ctrl->SetForegroundColour(text_color);

            wxWindowList kids = ctrl->GetChildren();
            for(unsigned int i = 0 ; i < kids.GetCount() ; i++)
            {
                  wxWindowListNode *node = kids.Item(i);
                  wxWindow *win = node->GetData();

                  if(win->IsKindOf(CLASSINFO(wxListBox)))
                        win->SetBackgroundColour(back_color);

                  else if(win->IsKindOf(CLASSINFO(wxGenericDirCtrl)))
                        win->SetBackgroundColour(back_color);

                  else if(win->IsKindOf(CLASSINFO(wxTextCtrl)))
                        win->SetBackgroundColour(back_color);

                  else if(win->IsKindOf(CLASSINFO(wxComboBox)))               // note ComboBoxes don't change bg properly on gtk
                        win->SetBackgroundColour(back_color);

                  else
                        win->SetBackgroundColour(GetGlobalColor(_T("DILG0")));      // msw looks better here

                  win->SetForegroundColour(text_color);
            }
      }
}

void options::OnButtonSelectSound(wxCommandEvent& event)
{
      wxString sound_dir = g_SData_Locn;
      sound_dir.Append(_T("sounds"));

      wxFileDialog *openDialog = new wxFileDialog(this, _("Select Sound File"), sound_dir, wxT(""),
                  _("WAV files (*.wav)|*.wav|All files (*.*)|*.*"), wxFD_OPEN);
      int response = openDialog->ShowModal();
      if (response == wxID_OK)
      {
            g_sAIS_Alert_Sound_File = openDialog->GetPath();
      }

}

void options::OnButtonTestSound(wxCommandEvent& event)
{
      wxSound AIS_Sound(g_sAIS_Alert_Sound_File);

      if(AIS_Sound.IsOk())
            AIS_Sound.Play();

}

wxString GetOCPNKnownLanguage(wxString lang_canonical, wxString *lang_dir)
{
      wxString return_string;
      wxString dir_suffix;

      if     (lang_canonical == _T("en_US")) {dir_suffix = _T("en");    return_string = wxString("English (U.S.)", wxConvUTF8);}
      else if(lang_canonical == _T("cs_CZ")) {dir_suffix = _T("cs");    return_string = wxString("Čeština", wxConvUTF8);}
      else if(lang_canonical == _T("da_DK")) {dir_suffix = _T("da");    return_string = wxString("Dansk", wxConvUTF8);}
      else if(lang_canonical == _T("de_DE")) {dir_suffix = _T("de");    return_string = wxString("Deutsch", wxConvUTF8);}
      else if(lang_canonical == _T("et_EE")) {dir_suffix = _T("et");    return_string = wxString("Eesti", wxConvUTF8);}
      else if(lang_canonical == _T("es_ES")) {dir_suffix = _T("es");    return_string = wxString("Español", wxConvUTF8);}
      else if(lang_canonical == _T("fr_FR")) {dir_suffix = _T("fr");    return_string = wxString("Français", wxConvUTF8);}
      else if(lang_canonical == _T("it_IT")) {dir_suffix = _T("it");    return_string = wxString("Italiano", wxConvUTF8);}
      else if(lang_canonical == _T("nl_NL")) {dir_suffix = _T("nl");    return_string = wxString("Nederlands", wxConvUTF8);}
      else if(lang_canonical == _T("pl_PL")) {dir_suffix = _T("pl");    return_string = wxString("Polski", wxConvUTF8);}
      else if(lang_canonical == _T("pt_PT")) {dir_suffix = _T("pt_PT"); return_string = wxString("Português", wxConvUTF8);}
      else if(lang_canonical == _T("pt_BR")) {dir_suffix = _T("pt_BR"); return_string = wxString("Português Brasileiro", wxConvUTF8);}
      else if(lang_canonical == _T("ru_RU")) {dir_suffix = _T("ru");    return_string = wxString("Русский", wxConvUTF8);}
      else if(lang_canonical == _T("sv_SE")) {dir_suffix = _T("sv");    return_string = wxString("Svenska", wxConvUTF8);}
      else if(lang_canonical == _T("fi_FI")) {dir_suffix = _T("fi_FI"); return_string = wxString("Suomi", wxConvUTF8);}
	else if(lang_canonical == _T("nb_NO")) {dir_suffix = _T("nb_NO"); return_string = wxString("Norsk", wxConvUTF8);}
      else if(lang_canonical == _T("tr_TR")) {dir_suffix = _T("tr_TR"); return_string = wxString("Türkçe", wxConvUTF8);}
      else if(lang_canonical == _T("el_GR")) {dir_suffix = _T("el_GR"); return_string = wxString("Ελληνικά", wxConvUTF8);}
      else if(lang_canonical == _T("hu_HU")) {dir_suffix = _T("hu_HU"); return_string = wxString("Magyar", wxConvUTF8);}
      else if(lang_canonical == _T("zh_TW")) {dir_suffix = _T("zh_TW"); return_string = wxString("正體字", wxConvUTF8);}
      else
      {
            dir_suffix = lang_canonical;
            const wxLanguageInfo *info = wxLocale::FindLanguageInfo(lang_canonical);
            return_string = info->Description;
      }

      if(NULL != lang_dir)
            *lang_dir = dir_suffix;

      return return_string;

}

