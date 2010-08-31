/******************************************************************************
 * $Id: sightdialog.cpp,v 1.21 2010/05/20 19:05:13 bdbcat Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Celestial Navigation Support
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2010 by Sean D'Epagnier   *
 *   $EMAIL$   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
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
 * $Log: sightdialog.cpp,v $
 *
 *
 */

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/listimpl.cpp>    // toh, 2009.02.22

#include "wx/wxprec.h"


#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "wx/notebook.h"
#include "wx/datetime.h"
#include "wx/colordlg.h"

#include "../../../include/ocpn_plugin.h"

#include "sightdialog.h"
#include "findbodydialog.h"

#include "sight.h"

#define ID_CHOICE_BODY  wxID_HIGHEST + 1


/*!
 * SightDialog type definition
 */

IMPLEMENT_DYNAMIC_CLASS( SightDialog, wxDialog )

/*!
 * SightDialog event table definition
 */

BEGIN_EVENT_TABLE( SightDialog, wxDialog )


    EVT_CHOICE( ID_SIGHTDIALOG_TYPE_SET, SightDialog::OnSightDialogTypeSet )
    EVT_BUTTON( ID_SIGHTDIALOG_FIND_BODY, SightDialog::OnSightDialogFindBodyClick )
    EVT_BUTTON( ID_SIGHTDIALOG_COLOR_SET, SightDialog::OnSightDialogColorSetClick )
    EVT_BUTTON( ID_SIGHTDIALOG_CANCEL, SightDialog::OnSightDialogCancelClick )
    EVT_BUTTON( ID_SIGHTDIALOG_OK, SightDialog::OnSightDialogOkClick )
    EVT_CLOSE(SightDialog::OnClose )
END_EVENT_TABLE()

/*!
 * SightDialog constructors
 */
SightDialog::SightDialog() {}

SightDialog::SightDialog( wxWindow* parent, wxWindowID id,
                     const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
      Create(parent, id, caption, pos, size, style);
      GetSizer()->SetSizeHints(this);
      Centre();
}


SightDialog::~SightDialog( )
{
}


/*!
 * SightDialog creator
 */

bool SightDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{

    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();

    return TRUE;
}

/*!
 * Control creation for SightDialog
 */

void SightDialog::CreateControls()
{
////@begin SightDialog content construction

    SightDialog* itemDialog1 = this;

    wxBoxSizer* itemBox = new wxBoxSizer(wxVERTICAL);
    itemDialog1->SetSizer(itemBox);

    wxNotebook *itemNotebook = new wxNotebook( itemDialog1, wxID_STATIC, wxDefaultPosition, wxSize(-1, -1), wxNB_TOP );
    itemBox->Add(itemNotebook, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 5);

    wxPanel *itemPanel1 = new wxPanel( itemNotebook, wxID_STATIC, wxDefaultPosition, wxDefaultSize,
                                       wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemNotebook->AddPage(itemPanel1, _("Sight"));

    wxGridSizer* itemGrid1 = new wxGridSizer(3);
    itemPanel1->SetSizer(itemGrid1);

    itemGrid1->Add(new wxStaticText( itemPanel1, wxID_STATIC, _("Type"), wxDefaultPosition, wxDefaultSize, 0 ), 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_typeChoice = new wxChoice( itemPanel1, ID_SIGHTDIALOG_TYPE_SET );
    itemGrid1->Add(m_typeChoice, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_typeChoice->Append(_T("Altitude (Sextant)"));
    m_typeChoice->Append(_T("Azimuth (Compass)"));
    m_typeChoice->Append(_T("Lunar  (angle between 2 bodies)"));

    m_typeChoice->SetSelection(0);

    itemGrid1->Add(new wxStaticText( itemPanel1, wxID_STATIC, _(""), wxDefaultPosition, wxDefaultSize, 0 ), 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_bodiesChoice = MakeChoiceLabelPair( itemPanel1, itemGrid1, _T("First Body" ));

    m_bodiesChoice->Append(_T("Sun"));
    m_bodiesChoice->Append(_T("Moon"));
    m_bodiesChoice->Append(_T("Mercury"));
    m_bodiesChoice->Append(_T("Venus"));
    m_bodiesChoice->Append(_T("Mars"));
    m_bodiesChoice->Append(_T("Jupiter"));
    m_bodiesChoice->Append(_T("Saturn"));

    m_bodiesChoice->Append(_T("Alpheratz"));
    m_bodiesChoice->Append(_T("Ankaa"));
    m_bodiesChoice->Append(_T("Schedar"));
    m_bodiesChoice->Append(_T("Diphda"));
    m_bodiesChoice->Append(_T("Achernar"));
    m_bodiesChoice->Append(_T("Hamal"));
    m_bodiesChoice->Append(_T("Polaris"));
    m_bodiesChoice->Append(_T("Acamar"));
    m_bodiesChoice->Append(_T("Menkar"));
    m_bodiesChoice->Append(_T("Mirfak"));
    m_bodiesChoice->Append(_T("Aldebaran"));
    m_bodiesChoice->Append(_T("Rigel"));
    m_bodiesChoice->Append(_T("Capella"));
    m_bodiesChoice->Append(_T("Bellatrix"));
    m_bodiesChoice->Append(_T("Elnath"));
    m_bodiesChoice->Append(_T("Alnilam"));
    m_bodiesChoice->Append(_T("Betelgeuse"));
    m_bodiesChoice->Append(_T("Canopus"));
    m_bodiesChoice->Append(_T("Sirius"));
    m_bodiesChoice->Append(_T("Adhara"));
    m_bodiesChoice->Append(_T("Procyon"));
    m_bodiesChoice->Append(_T("Pollux"));
    m_bodiesChoice->Append(_T("Avior"));
    m_bodiesChoice->Append(_T("Suhail"));
    m_bodiesChoice->Append(_T("Miaplacidus"));
    m_bodiesChoice->Append(_T("Alphard"));
    m_bodiesChoice->Append(_T("Regulus"));
    m_bodiesChoice->Append(_T("Dubhe"));
    m_bodiesChoice->Append(_T("Denebola"));
    m_bodiesChoice->Append(_T("Gienah"));
    m_bodiesChoice->Append(_T("Acrux"));
    m_bodiesChoice->Append(_T("Gacrux"));
    m_bodiesChoice->Append(_T("Alioth"));
    m_bodiesChoice->Append(_T("Spica"));
    m_bodiesChoice->Append(_T("Alkaid"));
    m_bodiesChoice->Append(_T("Hadar"));
    m_bodiesChoice->Append(_T("Menkent"));
    m_bodiesChoice->Append(_T("Arcturus"));
    m_bodiesChoice->Append(_T("Rigil"));
    m_bodiesChoice->Append(_T("Zubenelgenubi"));
    m_bodiesChoice->Append(_T("Kochab"));
    m_bodiesChoice->Append(_T("Alphecca"));
    m_bodiesChoice->Append(_T("Antares"));
    m_bodiesChoice->Append(_T("Atria"));
    m_bodiesChoice->Append(_T("Sabik"));
    m_bodiesChoice->Append(_T("Shaula"));
    m_bodiesChoice->Append(_T("Rasalhague"));
    m_bodiesChoice->Append(_T("Eltanin"));
    m_bodiesChoice->Append(_T("Kaus"));
    m_bodiesChoice->Append(_T("Vega"));
    m_bodiesChoice->Append(_T("Nunki"));
    m_bodiesChoice->Append(_T("Altair"));
    m_bodiesChoice->Append(_T("Peacock"));
    m_bodiesChoice->Append(_T("Deneb"));
    m_bodiesChoice->Append(_T("Enif"));
    m_bodiesChoice->Append(_T("Al Na'ir"));
    m_bodiesChoice->Append(_T("Fomalhaut"));
    m_bodiesChoice->Append(_T("Markab"));
    
    m_bodiesChoice->SetSelection(0);


    /* for secondary body must be a planet or moon */
    m_bodiesChoice2 = MakeChoiceLabelPair( itemPanel1, itemGrid1, _T("Second Body" ));

    m_bodiesChoice2->Append(_T("Sun"));
    m_bodiesChoice2->Append(_T("Moon"));
    m_bodiesChoice2->Append(_T("Mercury"));
    m_bodiesChoice2->Append(_T("Venus"));
    m_bodiesChoice2->Append(_T("Mars"));
    m_bodiesChoice2->Append(_T("Jupiter"));
    m_bodiesChoice2->Append(_T("Saturn"));
    
    m_bodiesChoice2->SetSelection(0);
    m_bodiesChoice2->Enable(false);

    m_bodyLimbChoice = MakeChoiceLabelPair( itemPanel1, itemGrid1, _T("Limb" ));

    m_bodyLimbChoice->Append(_T("Lower"));
    m_bodyLimbChoice->Append(_T("Center"));
    m_bodyLimbChoice->Append(_T("Upper"));

    m_bodyLimbChoice->SetSelection(0);

    itemGrid1->Add(new wxStaticText( itemPanel1, wxID_STATIC, _("Measurement:"),
                                     wxDefaultPosition, wxDefaultSize, 0 ),
                   0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    m_measurementText = MakeTextLabelPair(itemPanel1, itemGrid1, _T("0"), _T("degrees"));
    m_measurementMinutesText = MakeTextLabelPair(itemPanel1, itemGrid1, _T("0"), _T("minutes"));
    m_measurementCertaintyText = MakeTextLabelPair(itemPanel1, itemGrid1, _T("1"), _T("degrees accuracy"));

    wxBoxSizer* itemBoxSizerAzimuth = new wxBoxSizer(wxHORIZONTAL);
    itemGrid1->Add(itemBoxSizerAzimuth, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL
                   |wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    m_azimuthMagneticNorth = new wxCheckBox( itemPanel1, -1, _("Magnetic North"),
                                             wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizerAzimuth->Add(m_azimuthMagneticNorth, 1, wxALIGN_RIGHT|wxADJUST_MINSIZE, 5);
    m_azimuthMagneticNorth->Enable(false);

    m_FindBodyButton = new wxButton( itemPanel1, ID_SIGHTDIALOG_FIND_BODY, _("Find Body"),
                                     wxDefaultPosition, wxDefaultSize, 0);
    itemGrid1->Add(m_FindBodyButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxPanel *itemPanel2 = new wxPanel( itemNotebook, wxID_STATIC, wxDefaultPosition,
                                       wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemNotebook->AddPage(itemPanel2, _("Time"));

    wxBoxSizer* itemBox2 = new wxBoxSizer(wxVERTICAL);
    itemPanel2->SetSizer(itemBox2);

    wxBoxSizer* itemBox21 = new wxBoxSizer(wxHORIZONTAL);
    itemBox2->Add(itemBox21);

    m_dateCalendar = new wxCalendarCtrl(itemPanel2, ID_SIGHTDIALOG, wxDateTime::Now());
    itemBox21->Add(m_dateCalendar, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL
                   |wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxBoxSizer* itemBox211 = new wxBoxSizer(wxVERTICAL);
    itemBox21->Add(itemBox211);

    m_timeHours = MakeTextLabelPair( itemPanel2, itemBox211,
                                     wxDateTime::Now().Format(_T("%H")),
                                     _T("hours"));
    m_timeMinutes = MakeTextLabelPair( itemPanel2, itemBox211,
                                       wxDateTime::Now().Format(_T("%M")),
                                       _T("minutes"));
    m_timeSeconds = MakeTextLabelPair( itemPanel2, itemBox211,
                                       wxDateTime::Now().Format(_T("%S")),
                                       _T("seconds"));

    wxBoxSizer* itemBox2112 = new wxBoxSizer(wxHORIZONTAL);
    itemBox211->Add(itemBox2112);

    itemBox2112->Add(new wxStaticText( itemPanel2, wxID_STATIC, _("Time Accuracy"),
                                       wxDefaultPosition, wxDefaultSize, 0), 0,
                     wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 3);

    m_timeCertaintyText = MakeTextLabelPair( itemPanel2, itemBox2112, _T("1"), _T("seconds"));

    wxPanel *itemPanel3 = new wxPanel( itemNotebook, wxID_STATIC, wxDefaultPosition, wxDefaultSize,
                                       wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
    itemNotebook->AddPage(itemPanel3, _("Setup"));

    wxGridSizer* itemGrid3 = new wxGridSizer(2);
    itemPanel3->SetSizer(itemGrid3);

    itemGrid3->Add(new wxStaticText( itemPanel3, wxID_STATIC, _("Height of Eye"),
                                     wxDefaultPosition, wxDefaultSize, 0), 0,
                   wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 3);

    m_heightText = MakeTextLabelPair( itemPanel3, itemGrid3, _T("2"), _T("meters"));

    wxStaticText* itemStaticText5 = new wxStaticText( itemPanel3, wxID_STATIC,
                                                      _("Temperature"), wxDefaultPosition,
                                                      wxDefaultSize, 0);
    itemGrid3->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL
                   |wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    m_temperatureText = MakeTextLabelPair(itemPanel3, itemGrid3, _T("25"), _T("C"));

    itemGrid3->Add(new wxStaticText( itemPanel3, wxID_STATIC, _("Pressure"),
                                     wxDefaultPosition, wxDefaultSize, 0), 0,
                   wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    m_pressureText = MakeTextLabelPair(itemPanel3, itemGrid3, _T("1013"), _T("millibars"));

    static unsigned s_lastsightcolor;
    itemGrid3->Add(new wxStaticText( itemPanel3, wxID_STATIC, _("Color"),
                                     wxDefaultPosition, wxDefaultSize, 0 ), 0,
                   wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    m_Color = new wxStaticText( itemPanel3, wxID_STATIC, _(" OOOO "),
                                wxDefaultPosition, wxDefaultSize, 0 );
    itemGrid3->Add(m_Color, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    const wxColour sightcolors[] =
       {wxColour(_("AQUAMARINE")), wxColour(_("BLUE")), wxColour(_("BLUE VIOLET")),
        wxColour(_("BROWN")), wxColour(_("CADET BLUE")), wxColour(_("CORAL")),
        wxColour(_("CORNFLOWER BLUE")), wxColour(_("FOREST GREEN")), wxColour(_("GOLD")),
        wxColour(_("GOLDENROD")), wxColour(_("GREY")), wxColour(_("GREEN YELLOW")),
        wxColour(_("INDIAN RED")), wxColour(_("LIGHT BLUE")), wxColour(_("LIGHT STEEL BLUE")),
        wxColour(_("LIME GREEN")), wxColour(_("MAGENTA")), wxColour(_("MAROON")),
        wxColour(_("MEDIUM AQUAMARINE")), wxColour(_("MEDIUM BLUE")), wxColour(_("MEDIUM FOREST GREEN")),
        wxColour(_("MEDIUM GOLDENROD")), wxColour(_("MEDIUM ORCHID")), wxColour(_("MEDIUM SEA GREEN")),
        wxColour(_("MEDIUM SLATE BLUE")), wxColour(_("MEDIUM SPRING GREEN")), wxColour(_("MEDIUM TURQUOISE")),
        wxColour(_("MEDIUM VIOLET RED")), wxColour(_("MIDNIGHT BLUE")), wxColour(_("ORANGE")),
        wxColour(_("ORANGE RED")), wxColour(_("ORCHID")), wxColour(_("PALE GREEN")), wxColour(_("PINK")),
        wxColour(_("PLUM")), wxColour(_("PURPLE")), wxColour(_("RED")), wxColour(_("SALMON")),
        wxColour(_("SEA GREEN")), wxColour(_("SIENNA")), wxColour(_("SKY BLUE")),
        wxColour(_("SLATE BLUE")), wxColour(_("SPRING GREEN")), wxColour(_("STEEL BLUE")),
        wxColour(_("TAN")), wxColour(_("THISTLE")), wxColour(_("TURQUOISE")), wxColour(_("VIOLET")),
        wxColour(_("VIOLET RED")), wxColour(_("WHEAT")), wxColour(_("YELLOW")), wxColour(_("YELLOW GREEN"))};

    m_Color->SetForegroundColour(sightcolors[s_lastsightcolor]);

    if(++s_lastsightcolor == (sizeof sightcolors) / (sizeof *sightcolors))
       s_lastsightcolor=0;

    m_ColorSetButton = new wxButton( itemPanel3, ID_SIGHTDIALOG_COLOR_SET, _("Set Color"),
                                     wxDefaultPosition, wxDefaultSize, 0);
    itemGrid3->Add(m_ColorSetButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxBoxSizer* buttonBox = new wxBoxSizer(wxHORIZONTAL);
    itemBox->Add(buttonBox, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_CancelButton = new wxButton( itemDialog1, ID_SIGHTDIALOG_CANCEL, _("Cancel"),
                                   wxDefaultPosition, wxDefaultSize, 0 );
    buttonBox->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_OKButton = new wxButton( itemDialog1, ID_SIGHTDIALOG_OK, _("OK"),
                               wxDefaultPosition, wxDefaultSize, 0 );
    buttonBox->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_OKButton->SetDefault();

    // note that under MSW for SetColumnWidth() to work we need to create the
    // items with images initially even if we specify dummy image id
    wxListItem itemCol;
    itemCol.SetImage(-1);

//    SetColorScheme((ColorScheme)0);
}

wxTextCtrl *SightDialog::MakeTextLabelPair(wxWindow *dialog, wxSizer *sizer,
                                             const wchar_t *defaulttext,
                                             const wchar_t *labelvalue)
{
    wxBoxSizer* text_sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(text_sizer, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

    wxTextCtrl *ret = new wxTextCtrl( dialog, -1, defaulttext );
    text_sizer->Add(ret, 1, wxALIGN_RIGHT, 5);
   
    wxStaticText* itemStaticText = new wxStaticText( dialog, wxID_STATIC, labelvalue,
                                                    wxDefaultPosition, wxDefaultSize, 0 );
    text_sizer->Add(itemStaticText, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);
    return ret;
}

wxChoice *SightDialog::MakeChoiceLabelPair(wxWindow *dialog, wxSizer *sizer,
                                           const wchar_t *labelvalue)
{
    wxBoxSizer* choice_sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(choice_sizer, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

   wxStaticText* itemStaticText = new wxStaticText( dialog, wxID_STATIC, labelvalue,
                                                    wxDefaultPosition, wxDefaultSize, 0 );
   choice_sizer->Add(itemStaticText, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxADJUST_MINSIZE, 5);

   wxChoice *ret = new wxChoice( dialog, ID_CHOICE_BODY );
   choice_sizer->Add(ret, 1, wxALIGN_RIGHT, 5);
   return ret;
}

#if 0
void SightDialog::SetColorScheme(ColorScheme cs)
{
      SetBackgroundColour(GetGlobalColor(_T("DILG1")));

      wxColour back_color =GetGlobalColor(_T("DILG2"));
      wxColour text_color = GetGlobalColor(_T("DILG3"));


      m_CancelButton->SetBackgroundColour(back_color);
      m_CancelButton->SetForegroundColour(text_color);

      m_OKButton->SetBackgroundColour(back_color);
      m_OKButton->SetForegroundColour(text_color);
}
#endif

/*!
 * Should we show tooltips?
 */

bool SightDialog::ShowToolTips()
{
    return TRUE;
}


void SightDialog::SetDialogTitle(wxString title)
{
      SetTitle(title);
}

void SightDialog::OnSightDialogTypeSet( wxCommandEvent& event )
{   
   m_bodiesChoice2->Enable(m_typeChoice->GetSelection() == LUNAR);
   m_azimuthMagneticNorth->Enable(m_typeChoice->GetSelection() == AZIMUTH);
}

void SightDialog::OnSightDialogFindBodyClick( wxCommandEvent& event )
{
   Sight *sight = MakeNewSight();
   FindBodyDialog *findbody_dialog = new FindBodyDialog(GetParent(), sight);
   findbody_dialog->ShowModal();
   delete sight;
}

void SightDialog::OnSightDialogColorSetClick( wxCommandEvent& event )
{
   // Create the Colour Dialog.
   wxColourDialog *colour_dialog = new wxColourDialog(GetParent());
   colour_dialog->GetColourData().SetColour(m_Color->GetForegroundColour());
   colour_dialog->ShowModal();
   // After closing the Colour dialog, colour_data will contain
   // data about selected colour, and any custom colours.
   // Colour selected for use from the Colour Dialog.
   m_Color->SetForegroundColour ( colour_dialog->GetColourData().GetColour() );

}

void SightDialog::OnSightDialogCancelClick( wxCommandEvent& event )
{
   EndModal(wxID_CANCEL);
}


void SightDialog::OnSightDialogOkClick( wxCommandEvent& event )
{
   EndModal(wxID_OK);
}

void SightDialog::OnClose(wxCloseEvent& event)
{
   EndModal(wxID_CANCEL);
}

wxDateTime   SightDialog::DateTime()
{
   wxDateTime datetime =  m_dateCalendar->GetDate();

   double hours, minutes, seconds;
   m_timeHours->GetValue().ToDouble(&hours);
   m_timeMinutes->GetValue().ToDouble(&minutes);
   m_timeSeconds->GetValue().ToDouble(&seconds);

   double i;
   datetime.SetHour(hours);
   datetime.SetMinute(minutes);
   datetime.SetSecond(seconds);
   datetime.SetMillisecond(1000*modf(seconds, &i));

   return datetime;
}


void SightDialog::ReadSight(Sight *sight)
{
   int index;

   if(dynamic_cast<AltitudeSight*>(sight))
      m_typeChoice->SetSelection(ALTITUDE);
   else if(dynamic_cast<AzimuthSight*>(sight))
      m_typeChoice->SetSelection(AZIMUTH);
   else if(dynamic_cast<LunarSight*>(sight)) {
      m_typeChoice->SetSelection(LUNAR);
      index = m_bodiesChoice->FindString(dynamic_cast<LunarSight*>(sight)->m_Body2);
      if(index != wxNOT_FOUND)
         m_bodiesChoice->SetSelection(index);
   }

   index = m_bodiesChoice->FindString(sight->m_Body);
   if(index != wxNOT_FOUND)
      m_bodiesChoice->SetSelection(index);


   index = m_bodiesChoice2->FindString(sight->m_Body);
   if(index != wxNOT_FOUND)
      m_bodiesChoice2->SetSelection(index);

   m_bodyLimbChoice->SetSelection((int)sight->m_BodyLimb);

   m_dateCalendar->SetDate(sight->m_DateTime);

   m_timeHours->SetValue(sight->m_DateTime.Format(_T("%H")));
   m_timeMinutes->SetValue(sight->m_DateTime.Format(_T("%M")));
   m_timeSeconds->SetValue(sight->m_DateTime.Format(_T("%S")));
   
   m_timeCertaintyText->SetValue(wxString::Format(_T("%f"), sight->m_TimeCertainty));

   m_heightText->SetValue(wxString::Format(_T("%f"), sight->m_Height));
   m_temperatureText->SetValue(wxString::Format(_T("%f"), sight->m_Temperature));
   m_pressureText->SetValue(wxString::Format(_T("%f"), sight->m_Pressure));

   m_measurementText->SetValue(wxString::Format(_T("%f"), sight->m_Measurement));
   m_measurementCertaintyText->SetValue(wxString::Format(_T("%f"), sight->m_MeasurementCertainty));

   AzimuthSight *azimuthsight = dynamic_cast<AzimuthSight*>(sight);
   if(azimuthsight)
      m_azimuthMagneticNorth->SetValue(azimuthsight->m_bMagneticNorth);

   m_Color->SetForegroundColour(sight->m_Colour);
}

Sight *SightDialog::MakeNewSight()
{
   double timecertainty, measurement, measurementminutes, measurementcertainty;
   double height, temperature, pressure;

   m_timeCertaintyText->GetValue().ToDouble(&timecertainty);
   m_heightText->GetValue().ToDouble(&height);
   m_temperatureText->GetValue().ToDouble(&temperature);
   m_pressureText->GetValue().ToDouble(&pressure);
   m_measurementText->GetValue().ToDouble(&measurement);
   m_measurementMinutesText->GetValue().ToDouble(&measurementminutes);
   measurement += measurementminutes / 60;
   m_measurementCertaintyText->GetValue().ToDouble(&measurementcertainty);
   
   switch(m_typeChoice->GetSelection()) {
   case ALTITUDE:
      return new AltitudeSight(m_bodiesChoice->GetStringSelection(),
                               (Sight::BodyLimb)m_bodyLimbChoice->GetSelection(),
                               DateTime(), timecertainty,
                               measurement, measurementcertainty,
                               height, temperature, pressure,
                               m_Color->GetForegroundColour());
   case AZIMUTH:
      return new AzimuthSight(m_bodiesChoice->GetStringSelection(),
                               DateTime(), timecertainty,
                               measurement, measurementcertainty,
                               m_azimuthMagneticNorth->GetValue(),
                              m_Color->GetForegroundColour());
   case LUNAR:
      return new LunarSight(m_bodiesChoice->GetStringSelection(),
                            m_bodiesChoice2->GetStringSelection(),
                               (Sight::BodyLimb)m_bodyLimbChoice->GetSelection(),
                               DateTime(), timecertainty,
                               measurement, measurementcertainty,
                               height, temperature, pressure,
                               m_Color->GetForegroundColour());
   }
   return NULL;
}
