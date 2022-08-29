/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  X11 font picker widget
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
 **************************************************************************/
#include <X11/Xlib.h>

#include <wx/wxprec.h>


#include <wx/button.h>
#include <wx/dcclient.h>
#include <wx/fontdata.h>
#include <wx/fontdlg.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/tokenzr.h>


#include "x11_font_picker.h"

// This enum lives in the generic implementation only, not used on any
// opencpn platform, duplicated here.
#ifdef X11_FONTDLG_BAD_TYPE
enum
{
    wxID_FONT_UNDERLINE = 3000,
    wxID_FONT_STYLE,
    wxID_FONT_WEIGHT,
    wxID_FONT_FAMILY,
    wxID_FONT_COLOUR,
    wxID_FONT_SIZE
};
#endif



//-----------------------------------------------------------------------------
// helper class - MyFontPreviewer
//-----------------------------------------------------------------------------

class MyFontPreviewer : public wxWindow {
public:
  MyFontPreviewer(wxWindow *parent, const wxSize &sz = wxDefaultSize)
      : wxWindow(parent, wxID_ANY, wxDefaultPosition, sz) {}

private:
  void OnPaint(wxPaintEvent &event);
  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(MyFontPreviewer, wxWindow)
EVT_PAINT(MyFontPreviewer::OnPaint)
END_EVENT_TABLE()

void MyFontPreviewer::OnPaint(wxPaintEvent &WXUNUSED(event)) {
  wxPaintDC dc(this);

  wxSize size = GetSize();
  wxFont font = GetFont();

  dc.SetPen(*wxBLACK_PEN);
  dc.SetBrush(*wxWHITE_BRUSH);
  dc.DrawRectangle(0, 0, size.x, size.y);

  if (font.Ok()) {
    dc.SetFont(font);
    // Calculate vertical centre
    long w, h;
    dc.GetTextExtent(wxT("X"), &w, &h);
    dc.SetTextForeground(GetForegroundColour());
    dc.SetClippingRegion(2, 2, size.x - 4, size.y - 4);
    dc.DrawText(GetName(), 10, size.y / 2 - h / 2);
    dc.DestroyClippingRegion();
  }
}

//-----------------------------------------------------------------------------
// X11FontPicker
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(X11FontPicker, wxDialog)

BEGIN_EVENT_TABLE(X11FontPicker, wxDialog)
EVT_CHECKBOX(wxID_FONT_UNDERLINE, X11FontPicker::OnChangeFont)
EVT_CHOICE(wxID_FONT_STYLE, X11FontPicker::OnChangeFont)
EVT_CHOICE(wxID_FONT_WEIGHT, X11FontPicker::OnChangeFont)
EVT_CHOICE(wxID_FONT_FAMILY, X11FontPicker::OnChangeFace)
EVT_CHOICE(wxID_FONT_COLOUR, X11FontPicker::OnChangeFont)
EVT_CHOICE(wxID_FONT_SIZE, X11FontPicker::OnChangeFont)

EVT_CLOSE(X11FontPicker::OnCloseWindow)
END_EVENT_TABLE()

#define SCALEABLE_SIZES 11
static wxString scaleable_pointsize[SCALEABLE_SIZES] = {
    wxT("6"),  wxT("8"),  wxT("10"), wxT("12"), wxT("14"), wxT("16"),
    wxT("18"), wxT("20"), wxT("24"), wxT("30"), wxT("36")};

#define NUM_COLS 49
static wxString wxColourDialogNames[NUM_COLS] = {wxT("ORANGE"),
                                                 wxT("GOLDENROD"),
                                                 wxT("WHEAT"),
                                                 wxT("SPRING GREEN"),
                                                 wxT("SKY BLUE"),
                                                 wxT("SLATE BLUE"),
                                                 wxT("MEDIUM VIOLET RED"),
                                                 wxT("PURPLE"),

                                                 wxT("RED"),
                                                 wxT("YELLOW"),
                                                 wxT("MEDIUM SPRING GREEN"),
                                                 wxT("PALE GREEN"),
                                                 wxT("CYAN"),
                                                 wxT("LIGHT STEEL BLUE"),
                                                 wxT("ORCHID"),
                                                 wxT("LIGHT MAGENTA"),

                                                 wxT("BROWN"),
                                                 wxT("YELLOW"),
                                                 wxT("GREEN"),
                                                 wxT("CADET BLUE"),
                                                 wxT("MEDIUM BLUE"),
                                                 wxT("MAGENTA"),
                                                 wxT("MAROON"),
                                                 wxT("ORANGE RED"),

                                                 wxT("FIREBRICK"),
                                                 wxT("CORAL"),
                                                 wxT("FOREST GREEN"),
                                                 wxT("AQUARAMINE"),
                                                 wxT("BLUE"),
                                                 wxT("NAVY"),
                                                 wxT("THISTLE"),
                                                 wxT("MEDIUM VIOLET RED"),

                                                 wxT("INDIAN RED"),
                                                 wxT("GOLD"),
                                                 wxT("MEDIUM SEA GREEN"),
                                                 wxT("MEDIUM BLUE"),
                                                 wxT("MIDNIGHT BLUE"),
                                                 wxT("GREY"),
                                                 wxT("PURPLE"),
                                                 wxT("KHAKI"),

                                                 wxT("BLACK"),
                                                 wxT("MEDIUM FOREST GREEN"),
                                                 wxT("KHAKI"),
                                                 wxT("DARK GREY"),
                                                 wxT("SEA GREEN"),
                                                 wxT("LIGHT GREY"),
                                                 wxT("MEDIUM SLATE BLUE"),
                                                 wxT("WHITE") wxT("SIENNA")};

/*
 * Generic X11FontPicker
 */

void X11FontPicker::Init() {
  m_useEvents = false;
  m_previewer = NULL;
  Create(m_parent);
}

X11FontPicker::~X11FontPicker() {}

void X11FontPicker::OnCloseWindow(wxCloseEvent &WXUNUSED(event)) {
  EndModal(wxID_CANCEL);
}

bool X11FontPicker::DoCreate(wxWindow *parent) {
  if (!wxDialog::Create(parent, wxID_ANY, _T ( "Choose Font" ),
                        wxDefaultPosition, wxDefaultSize,
                        wxDEFAULT_DIALOG_STYLE, _T ( "fontdialog" ))) {
    wxFAIL_MSG(wxT("wxFontDialog creation failed"));
    return false;
  }

  InitializeAllAvailableFonts();
  InitializeFont();
  CreateWidgets();

  // sets initial font in preview area
  wxCommandEvent dummy;
  OnChangeFont(dummy);

  return true;
}

int X11FontPicker::ShowModal() {
  int ret = wxDialog::ShowModal();

  if (ret != wxID_CANCEL) {
    dialogFont = *pPreviewFont;
    m_fontData.m_chosenFont = dialogFont;
  }

  return ret;
}

void X11FontPicker::InitializeAllAvailableFonts() {
  // get the Array of all fonts facenames
  wxString pattern;
  pattern.Printf(wxT("-*-*-*-*-*-*-*-*-*-*-*-*-iso8859-1"));

  int nFonts;
  char **list =
      XListFonts((Display *)wxGetDisplay(), pattern.mb_str(), 32767, &nFonts);

  pFaceNameArray = new wxArrayString;
  unsigned int jname;
  for (int i = 0; i < nFonts; i++) {
    wxStringTokenizer st(wxString(list[i]), _T ( "-" ));
    st.GetNextToken();
    st.GetNextToken();
    wxString facename = st.GetNextToken();
    for (jname = 0; jname < pFaceNameArray->GetCount(); jname++) {
      if (facename == pFaceNameArray->Item(jname)) break;
    }
    if (jname >= pFaceNameArray->GetCount()) {
      pFaceNameArray->Add(facename);
    }
  }
}

// This should be application-settable
static bool ShowToolTips() { return false; }

void X11FontPicker::CreateWidgets() {
  // layout

  bool is_pda = (wxSystemSettings::GetScreenType() <= wxSYS_SCREEN_PDA);
  int noCols, noRows;
  if (is_pda) {
    noCols = 2;
    noRows = 3;
  } else {
    noCols = 3;
    noRows = 2;
  }

  wxBoxSizer *itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  this->SetSizer(itemBoxSizer2);
  this->SetAutoLayout(TRUE);

  wxBoxSizer *itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizer2->Add(itemBoxSizer3, 1, wxEXPAND | wxALL, 5);

  wxFlexGridSizer *itemGridSizer4 = new wxFlexGridSizer(noRows, noCols, 0, 0);
  itemBoxSizer3->Add(itemGridSizer4, 0, wxEXPAND, 5);

  wxBoxSizer *itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
  itemGridSizer4->Add(itemBoxSizer5, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND,
                      5);
  wxStaticText *itemStaticText6 =
      new wxStaticText(this, wxID_STATIC, _("&Font family:"), wxDefaultPosition,
                       wxDefaultSize, 0);
  itemBoxSizer5->Add(itemStaticText6, 0,
                     wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5);

  wxChoice *itemChoice7 =
      new wxChoice(this, wxID_FONT_FAMILY, wxDefaultPosition, wxDefaultSize,
                   *pFaceNameArray, 0);
  itemChoice7->SetHelpText(_("The font family."));
  if (ShowToolTips()) itemChoice7->SetToolTip(_("The font family."));
  itemBoxSizer5->Add(itemChoice7, 0, wxALIGN_LEFT | wxALL, 5);

  wxBoxSizer *itemBoxSizer8 = new wxBoxSizer(wxVERTICAL);
  itemGridSizer4->Add(itemBoxSizer8, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND,
                      5);
  wxStaticText *itemStaticText9 = new wxStaticText(
      this, wxID_STATIC, _("&Style:"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer8->Add(itemStaticText9, 0,
                     wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5);

  wxChoice *itemChoice10 =
      new wxChoice(this, wxID_FONT_STYLE, wxDefaultPosition, wxDefaultSize);
  itemChoice10->SetHelpText(_("The font style."));
  if (ShowToolTips()) itemChoice10->SetToolTip(_("The font style."));
  itemBoxSizer8->Add(itemChoice10, 0, wxALIGN_LEFT | wxALL, 5);

  wxBoxSizer *itemBoxSizer11 = new wxBoxSizer(wxVERTICAL);
  itemGridSizer4->Add(itemBoxSizer11, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND,
                      5);
  wxStaticText *itemStaticText12 = new wxStaticText(
      this, wxID_STATIC, _("&Weight:"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer11->Add(itemStaticText12, 0,
                      wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5);

  wxChoice *itemChoice13 =
      new wxChoice(this, wxID_FONT_WEIGHT, wxDefaultPosition, wxDefaultSize);
  itemChoice13->SetHelpText(_("The font weight."));
  if (ShowToolTips()) itemChoice13->SetToolTip(_("The font weight."));
  itemBoxSizer11->Add(itemChoice13, 0, wxALIGN_LEFT | wxALL, 5);

  wxBoxSizer *itemBoxSizer14 = new wxBoxSizer(wxVERTICAL);
  itemGridSizer4->Add(itemBoxSizer14, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND,
                      5);
  if (m_fontData.GetEnableEffects()) {
    wxStaticText *itemStaticText15 = new wxStaticText(
        this, wxID_STATIC, _("C&olour:"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizer14->Add(itemStaticText15, 0,
                        wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5);

    wxSize colourSize = wxDefaultSize;
    if (is_pda) colourSize.x = 100;

    wxChoice *itemChoice16 =
        new wxChoice(this, wxID_FONT_COLOUR, wxDefaultPosition, colourSize,
                     NUM_COLS, wxColourDialogNames, 0);
    itemChoice16->SetHelpText(_("The font colour."));
    if (ShowToolTips()) itemChoice16->SetToolTip(_("The font colour."));
    itemBoxSizer14->Add(itemChoice16, 0, wxALIGN_LEFT | wxALL, 5);
  }

  wxBoxSizer *itemBoxSizer17 = new wxBoxSizer(wxVERTICAL);
  itemGridSizer4->Add(itemBoxSizer17, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND,
                      5);
  wxStaticText *itemStaticText18 =
      new wxStaticText(this, wxID_STATIC, _("&Point size:"), wxDefaultPosition,
                       wxDefaultSize, 0);
  itemBoxSizer17->Add(itemStaticText18, 0,
                      wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5);

  wxChoice *pc =
      new wxChoice(this, wxID_FONT_SIZE, wxDefaultPosition, wxDefaultSize);
  pc->SetHelpText(_("The font point size."));
  if (ShowToolTips()) pc->SetToolTip(_("The font point size."));
  itemBoxSizer17->Add(pc, 0, wxALIGN_LEFT | wxALL, 5);

  if (m_fontData.GetEnableEffects()) {
    wxBoxSizer *itemBoxSizer20 = new wxBoxSizer(wxVERTICAL);
    itemGridSizer4->Add(itemBoxSizer20, 0,
                        wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    wxCheckBox *itemCheckBox21 =
        new wxCheckBox(this, wxID_FONT_UNDERLINE, _("&Underline"),
                       wxDefaultPosition, wxDefaultSize, 0);
    itemCheckBox21->SetValue(FALSE);
    itemCheckBox21->SetHelpText(_("Whether the font is underlined."));
    if (ShowToolTips())
      itemCheckBox21->SetToolTip(_("Whether the font is underlined."));
    itemBoxSizer20->Add(itemCheckBox21, 0, wxALIGN_LEFT | wxALL, 5);
  }

  if (!is_pda)
    itemBoxSizer3->Add(5, 5, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

  wxStaticText *itemStaticText23 = new wxStaticText(
      this, wxID_STATIC, _("Preview:"), wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer3->Add(itemStaticText23, 0,
                     wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5);

  MyFontPreviewer *itemWindow24 = new MyFontPreviewer(this, wxSize(400, 80));
  m_previewer = itemWindow24;
  itemWindow24->SetHelpText(_("Shows the font preview."));
  if (ShowToolTips()) itemWindow24->SetToolTip(_("Shows the font preview."));
  itemBoxSizer3->Add(itemWindow24, 0, wxEXPAND, 5);

  wxBoxSizer *itemBoxSizer25 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer3->Add(itemBoxSizer25, 0, wxEXPAND, 5);
  itemBoxSizer25->Add(5, 5, 1, wxEXPAND | wxALL, 5);

  wxButton *itemButton28 = new wxButton(this, wxID_CANCEL, _("Cancel"),
                                        wxDefaultPosition, wxDefaultSize, 0);

  if (ShowToolTips())
    itemButton28->SetToolTip(_("Click to cancel the font selection."));
  itemBoxSizer25->Add(itemButton28, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  wxButton *itemButton27 = new wxButton(this, wxID_OK, _("&OK"),
                                        wxDefaultPosition, wxDefaultSize, 0);
  itemButton27->SetDefault();
  itemButton27->SetHelpText(_("Click to confirm the font selection."));
  if (ShowToolTips())
    itemButton27->SetToolTip(_("Click to confirm the font selection."));
  itemBoxSizer25->Add(itemButton27, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  familyChoice = (wxChoice *)FindWindow(wxID_FONT_FAMILY);
  styleChoice = (wxChoice *)FindWindow(wxID_FONT_STYLE);
  weightChoice = (wxChoice *)FindWindow(wxID_FONT_WEIGHT);
  colourChoice = (wxChoice *)FindWindow(wxID_FONT_COLOUR);
  pointSizeChoice = (wxChoice *)FindWindow(wxID_FONT_SIZE);
  underLineCheckBox = (wxCheckBox *)FindWindow(wxID_FONT_UNDERLINE);

  //    Get readable font items
  wxString gotfontnative = dialogFont.GetNativeFontInfoDesc();
  wxStringTokenizer st(gotfontnative, _T ( "-" ));
  st.GetNextToken();
  st.GetNextToken();
  wxString facename = st.GetNextToken();
  wxString weight = st.GetNextToken();
  st.GetNextToken();
  st.GetNextToken();
  st.GetNextToken();
  st.GetNextToken();
  wxString pointsize = st.GetNextToken();

  int ptsz = atoi(pointsize.mb_str());
  pointsize.Printf(_T ( "%d" ), ptsz / 10);

  SetChoiceOptionsFromFacename(facename);

  familyChoice->SetStringSelection(facename);
  weightChoice->SetStringSelection(weight);
  pointSizeChoice->SetStringSelection(pointsize);

  m_previewer->SetFont(dialogFont);
  m_previewer->SetName(_T( "ABCDEFGabcdefg12345" ));

  //    m_previewer->Refresh();

  //    familyChoice->SetStringSelection(
  //    wxFontFamilyIntToString(dialogFont.GetFamily()) );
  //    styleChoice->SetStringSelection(wxFontStyleIntToString(dialogFont.GetStyle()));
  //    weightChoice->SetStringSelection(wxFontWeightIntToString(dialogFont.GetWeight()));

  /*
   if (colourChoice)
   {
   wxString name(wxTheColourDatabase->FindName(m_fontData.GetColour()));
   if (name.length())
   colourChoice->SetStringSelection(name);
   else
   colourChoice->SetStringSelection(wxT("BLACK"));
   }

   if (underLineCheckBox)
   {
   underLineCheckBox->SetValue(dialogFont.GetUnderlined());
   }

   //    pointSizeChoice->SetSelection(dialogFont.GetPointSize()-1);
   pointSizeChoice->SetSelection(0);

   #if !defined(__SMARTPHONE__) && !defined(__POCKETPC__)
   GetSizer()->SetItemMinSize(m_previewer, is_pda ? 100 : 430, is_pda ? 40 :
   100); GetSizer()->SetSizeHints(this); GetSizer()->Fit(this);

   Centre(wxBOTH);
   #endif
   */

  // Don't block events any more
  m_useEvents = true;
}

void X11FontPicker::OnChangeFace(wxCommandEvent &WXUNUSED(event)) {
  if (!m_useEvents) return;

  //    Capture the current selections
  wxString facename = familyChoice->GetStringSelection();
  wxString pointsize = pointSizeChoice->GetStringSelection();
  wxString weight = weightChoice->GetStringSelection();

  SetChoiceOptionsFromFacename(facename);

  //    Reset the choices
  familyChoice->SetStringSelection(facename);
  weightChoice->SetStringSelection(weight);
  pointSizeChoice->SetStringSelection(pointsize);

  //    And make the font change
  DoFontChange();
}

void X11FontPicker::SetChoiceOptionsFromFacename(const wxString &facename) {
 
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat="
#pragma GCC diagnostic ignored "-Wformat-overflow"

 //    Get a list of matching fonts
  char face[101];
  strncpy(face, facename.mb_str(), 100);
  face[100] = '\0';

  char pattern[100];
  sprintf(pattern, "-*-%s-*-*-*-*-*-*-*-*-*-*-iso8859-1", face);
  //    wxString pattern;
  //    pattern.Printf(wxT("-*-%s-*-*-*-*-*-*-*-*-*-*-iso8859-1"),
  //    facename.mb_str());

#pragma GCC diagnostic pop

  int nFonts;
  char **list = XListFonts((Display *)wxGetDisplay(), pattern, 32767, &nFonts);

  //    First, look thru all the point sizes looking for "0" to indicate
  //    scaleable (e.g. TrueType) font
  bool scaleable = false;
  for (int i = 0; i < nFonts; i++) {
    wxStringTokenizer st(wxString(list[i]), _T ( "-" ));
    st.GetNextToken();
    st.GetNextToken();
    st.GetNextToken();
    st.GetNextToken();
    st.GetNextToken();
    st.GetNextToken();
    st.GetNextToken();
    wxString pointsize = st.GetNextToken();

    if (pointsize.IsSameAs(_T ( "0" ))) {
      scaleable = true;
      break;
    }
  }

  // make different pointsize selections for scaleable fonts
  wxArrayString PointSizeArray;

  if (scaleable) {
    for (int j = 0; j < SCALEABLE_SIZES; j++)
      PointSizeArray.Add(scaleable_pointsize[j]);
  }

  else {
    // Get the Point Sizes Array
    unsigned int jname;
    for (int i = 0; i < nFonts; i++) {
      //                  printf("%s\n", list[i]);
      wxStringTokenizer st(wxString(list[i]), _T ( "-" ));
      st.GetNextToken();
      st.GetNextToken();
      st.GetNextToken();
      st.GetNextToken();
      st.GetNextToken();
      st.GetNextToken();
      st.GetNextToken();
      wxString pointsize = st.GetNextToken();
      //           printf("%s\n",facename.mb_str());
      for (jname = 0; jname < PointSizeArray.GetCount(); jname++) {
        if (pointsize == PointSizeArray[jname]) break;
      }
      if (jname >= PointSizeArray.GetCount()) {
        PointSizeArray.Add(pointsize);
        //                        printf("Added %s\n", pointsize.mb_str());
      }
    }
  }
  pointSizeChoice->Clear();
  pointSizeChoice->Append(PointSizeArray);
  pointSizeChoice->SetSelection(0);

  // Get the Weight Array
  wxArrayString WeightArray;
  for (int i = 0; i < nFonts; i++) {
    //            printf("%s\n", list[i]);
    wxStringTokenizer st(wxString(list[i]), _T ( "-" ));
    st.GetNextToken();
    st.GetNextToken();
    st.GetNextToken();
    wxString weight = st.GetNextToken();
    //           printf("%s\n",facename.mb_str());
    unsigned int jname;
    for (jname = 0; jname < WeightArray.GetCount(); jname++) {
      if (weight == WeightArray[jname]) break;
    }
    if (jname >= WeightArray.GetCount()) {
      WeightArray.Add(weight);
      //                  printf("Added weight %s\n", weight.mb_str());
    }
  }
  weightChoice->Clear();
  weightChoice->Append(WeightArray);
  weightChoice->SetSelection(0);
}

void X11FontPicker::InitializeFont() {
  int fontFamily = wxSWISS;
  int fontWeight = wxNORMAL;
  int fontStyle = wxNORMAL;
  int fontSize = 12;
  bool fontUnderline = false;

  wxString fontnative;
  if (m_fontData.m_initialFont.Ok()) {
    fontnative = m_fontData.m_initialFont.GetNativeFontInfoDesc();
    fontFamily = m_fontData.m_initialFont.GetFamily();
    fontWeight = m_fontData.m_initialFont.GetWeight();
    fontStyle = m_fontData.m_initialFont.GetStyle();
    fontSize = m_fontData.m_initialFont.GetPointSize();
    fontUnderline = m_fontData.m_initialFont.GetUnderlined();
  }

  //      printf("Init Fetching    %s\n", fontnative.mb_str());

  wxFont tFont =
      wxFont(fontSize, fontFamily, fontStyle, fontWeight, fontUnderline);

  wxFont *pdialogFont = tFont.New(fontnative);

  //    Confirm
  /*
   wxNativeFontInfo *i = (wxNativeFontInfo *)pdialogFont->GetNativeFontInfo();

   XFontStruct *xfont = (XFontStruct*) pdialogFont->GetFontStruct( 1.0, (Display
   *)wxGetDisplay() ); unsigned long ret; XGetFontProperty(xfont, 18, &ret);
   char* x = XGetAtomName((Display *)wxGetDisplay(), ret);
   printf(" Init Got %s\n", x);
   */

  dialogFont = *pdialogFont;
}

void X11FontPicker::OnChangeFont(wxCommandEvent &WXUNUSED(event)) {
  if (!m_useEvents) return;

  DoFontChange();
}

void X11FontPicker::DoFontChange(void) {
  wxString facename = familyChoice->GetStringSelection();
  wxString pointsize = pointSizeChoice->GetStringSelection();
  wxString weight = weightChoice->GetStringSelection();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
  char font_x[200];
  sprintf(font_x, "-*-%s-%s-r-normal-*-*-%s0-*-*-*-*-iso8859-1",
          facename.mb_str(), weight.mb_str(), pointsize.mb_str());
  wxString font_xlfd(font_x, wxConvUTF8);

#pragma GCC diagnostic pop
  //                  printf("Fetching    %s\n", font_xlfd.mb_str());

  XFontStruct *test =
      XLoadQueryFont((Display *)wxGetDisplay(), font_xlfd.mb_str());

  //    Confirm
  /*
   unsigned long ret0;
   if(test)
   {
   XGetFontProperty(test, 18, &ret0);
   char* x = XGetAtomName((Display *)wxGetDisplay(), ret0);
   printf("FGot %s\n", x);
   }
   */
  if (test) {
    font_xlfd.Prepend("0;");
    wxFont *ptf = new wxFont;
    pPreviewFont = ptf->New(font_xlfd);

    /*
     wxNativeFontInfo *i = (wxNativeFontInfo
     *)pPreviewFont->GetNativeFontInfo();

     XFontStruct *xfont = (XFontStruct*) pPreviewFont->GetFontStruct( 1.0,
     (Display *)wxGetDisplay() ); unsigned long ret; XGetFontProperty(xfont, 18,
     &ret); char* x = XGetAtomName((Display *)wxGetDisplay(), ret); printf("Got
     %s\n", x);
     */
    m_previewer->SetName(_T( "ABCDEFGabcdefg12345" ));
    m_previewer->SetFont(*pPreviewFont);
    m_previewer->Refresh();
  }

  else {
    wxString err(_T ( "No Font:" ));
    err.Append(font_xlfd);
    m_previewer->SetName(err);
    m_previewer->SetFont(*pPreviewFont);
    m_previewer->Refresh();
  }

  /*
   int fontFamily = wxFontFamilyStringToInt(WXSTRINGCAST
   familyChoice->GetStringSelection()); int fontWeight =
   wxFontWeightStringToInt(WXSTRINGCAST weightChoice->GetStringSelection()); int
   fontStyle = wxFontStyleStringToInt(WXSTRINGCAST
   styleChoice->GetStringSelection()); int fontSize =
   wxAtoi(pointSizeChoice->GetStringSelection());
   // Start with previous underline setting, we want to retain it even if we
   can't edit it
   // dialogFont is always initialized because of the call to InitializeFont
   int fontUnderline = dialogFont.GetUnderlined();

   if (underLineCheckBox)
   {
   fontUnderline = underLineCheckBox->GetValue();
   }

   dialogFont = wxFont(fontSize, fontFamily, fontStyle, fontWeight,
   (fontUnderline != 0)); m_previewer->SetFont(dialogFont);

   if ( colourChoice )
   {
   if ( !colourChoice->GetStringSelection().empty() )
   {
   wxColour col = wxTheColourDatabase->Find(colourChoice->GetStringSelection());
   if (col.Ok())
   {
   m_fontData.m_fontColour = col;
   }
   }
   }
   // Update color here so that we can also use the color originally passed in
   // (EnableEffects may be false)
   if (m_fontData.m_fontColour.Ok())
   m_previewer->SetForegroundColour(m_fontData.m_fontColour);

   m_previewer->Refresh();
   */
}
