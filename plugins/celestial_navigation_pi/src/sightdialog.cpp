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

#include "wx/datetime.h"
#include "wx/calctrl.h"
#include "wx/colordlg.h"

#include "sightdialog.h"

#include "../../../include/ocpn_plugin.h"

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


    EVT_BUTTON( ID_SIGHTDIALOG_DATETIME_SET, SightDialog::OnSightDialogDateTimeSetClick )
    EVT_BUTTON( ID_SIGHTDIALOG_COLOR_SET, SightDialog::OnSightDialogColorSetClick )
    EVT_CHECKBOX( ID_SIGHTDIALOG_ELEVATION_CHECK, SightDialog::OnElevationCheck )
    EVT_CHECKBOX( ID_SIGHTDIALOG_AZIMUTH_CHECK, SightDialog::OnAzimuthCheck )
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

    wxGridSizer* itemGrid = new wxGridSizer(3);
    itemDialog1->SetSizer(itemGrid);

    wxStaticText* itemStaticText00 = new wxStaticText( itemDialog1, wxID_STATIC, _("Name"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGrid->Add(itemStaticText00, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    wxStaticText* itemStaticText01 = new wxStaticText( itemDialog1, wxID_STATIC, _("Value"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGrid->Add(itemStaticText01, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    wxStaticText* itemStaticText02 = new wxStaticText( itemDialog1, wxID_STATIC, _("Certainty"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGrid->Add(itemStaticText02, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    wxStaticText* itemStaticText1 = new wxStaticText( itemDialog1, wxID_STATIC, _("Body"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGrid->Add(itemStaticText1, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_bodiesChoice = new wxChoice( itemDialog1, ID_CHOICE_BODY );
    itemGrid->Add(m_bodiesChoice, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

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

    m_bodyLimbChoice = new wxChoice( itemDialog1, ID_CHOICE_BODY );
    itemGrid->Add(m_bodyLimbChoice, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_bodyLimbChoice->Append(_T("Center"));
    m_bodyLimbChoice->Append(_T("Upper"));
    m_bodyLimbChoice->Append(_T("Lower"));

    m_bodyLimbChoice->SetSelection(0);

    wxStaticText* itemStaticText2 = new wxStaticText( itemDialog1, wxID_STATIC, _("Date Time"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGrid->Add(itemStaticText2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    m_datetime = wxDateTime::Now().ToUTC();
    m_datetimeText = new wxStaticText( itemDialog1, ID_SIGHTDIALOG_DATETIME_SET, m_datetime.Format(wxDefaultDateTimeFormat), wxDefaultPosition, wxDefaultSize, 0 );
    itemGrid->Add(m_datetimeText, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

//   wxCalendarCtrl *calendar = new wxCalendarCtrl(GetParent(), ID_SIGHTDIALOG, m_datetime);
//   itemGrid->Add(calendar, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxBoxSizer* itemBoxSizerTimerUncertenty = new wxBoxSizer(wxHORIZONTAL);
    itemGrid->Add(itemBoxSizerTimerUncertenty, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    m_timeCertaintyText = new wxTextCtrl( itemDialog1, -1, _T("1") );
    itemBoxSizerTimerUncertenty->Add(m_timeCertaintyText, 1, wxALIGN_RIGHT, 5);

    wxStaticText* itemStaticText24 = new wxStaticText( itemDialog1, wxID_STATIC, _("seconds"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizerTimerUncertenty->Add(itemStaticText24, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    m_elevationUsed = new wxCheckBox( itemDialog1, ID_SIGHTDIALOG_ELEVATION_CHECK, _("Elevation"), wxDefaultPosition, wxDefaultSize, 0 );
    m_elevationUsed->SetValue(true);
    itemGrid->Add(m_elevationUsed, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    m_elevationText = new wxTextCtrl( itemDialog1, -1, _T("0") );
    itemGrid->Add(m_elevationText, 1, wxALIGN_RIGHT, 5);

    wxBoxSizer* itemBoxSizerElevationUncertenty = new wxBoxSizer(wxHORIZONTAL);
    itemGrid->Add(itemBoxSizerElevationUncertenty, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    m_elevationCertaintyText = new wxTextCtrl( itemDialog1, -1, _T(".05") );
    itemBoxSizerElevationUncertenty->Add(m_elevationCertaintyText, 1, wxALIGN_RIGHT, 5);

    wxStaticText* itemStaticText34 = new wxStaticText( itemDialog1, wxID_STATIC, _("degrees"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizerElevationUncertenty->Add(itemStaticText34, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    m_azimuthUsed = new wxCheckBox( itemDialog1, ID_SIGHTDIALOG_AZIMUTH_CHECK, _("Azimuth"), wxDefaultPosition, wxDefaultSize, 0);
    itemGrid->Add(m_azimuthUsed, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxBoxSizer* itemBoxSizerAzimuth = new wxBoxSizer(wxHORIZONTAL);
    itemGrid->Add(itemBoxSizerAzimuth, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    m_azimuthMagneticNorth = new wxCheckBox( itemDialog1, -1, _("Magnetic North"), wxDefaultPosition, wxDefaultSize, 0);
    itemBoxSizerAzimuth->Add(m_azimuthMagneticNorth, 1, wxALIGN_RIGHT, 5);
    m_azimuthMagneticNorth->Enable(false);

    m_azimuthText = new wxTextCtrl( itemDialog1, -1, _T("0") );
    itemBoxSizerAzimuth->Add(m_azimuthText, 1, wxALIGN_RIGHT, 5);
    m_azimuthText->Enable(false);

    wxBoxSizer* itemBoxSizerAzimuthUncertenty = new wxBoxSizer(wxHORIZONTAL);
    itemGrid->Add(itemBoxSizerAzimuthUncertenty, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    m_azimuthCertaintyText = new wxTextCtrl( itemDialog1, -1, _T(".05") );
    itemBoxSizerAzimuthUncertenty->Add(m_azimuthCertaintyText, 1, wxALIGN_RIGHT, 5);
    m_azimuthCertaintyText->Enable(false);

    wxStaticText* itemStaticText44 = new wxStaticText( itemDialog1, wxID_STATIC, _("degrees"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizerAzimuthUncertenty->Add(itemStaticText44, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    wxStaticText* itemStaticText5 = new wxStaticText( itemDialog1, wxID_STATIC, _("Height"), wxDefaultPosition, wxDefaultSize, 0);
    itemGrid->Add(itemStaticText5, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    m_heightText = new wxTextCtrl( itemDialog1, -1, _T("2") );
    itemGrid->Add(m_heightText, 1, wxALIGN_RIGHT, 5);

    wxStaticText* itemStaticText54 = new wxStaticText( itemDialog1, wxID_STATIC, _("meters"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGrid->Add(itemStaticText54, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    static unsigned s_lastsightcolor;
    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Color"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGrid->Add(itemStaticText6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);

    m_Color = new wxStaticText( itemDialog1, wxID_STATIC, _(" OOOO "), wxDefaultPosition, wxDefaultSize, 0 );
    itemGrid->Add(m_Color, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

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

    m_ColorSetButton = new wxButton( itemDialog1, ID_SIGHTDIALOG_COLOR_SET, _("Set Color"),
                                     wxDefaultPosition, wxDefaultSize, 0);
    itemGrid->Add(m_ColorSetButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);


    m_CancelButton = new wxButton( itemDialog1, ID_SIGHTDIALOG_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGrid->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_OKButton = new wxButton( itemDialog1, ID_SIGHTDIALOG_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemGrid->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_OKButton->SetDefault();

    // note that under MSW for SetColumnWidth() to work we need to create the
    // items with images initially even if we specify dummy image id
    wxListItem itemCol;
    itemCol.SetImage(-1);


//    SetColorScheme((ColorScheme)0);

}

/*
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
*/
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

void SightDialog::OnSightDialogDateTimeSetClick( wxCommandEvent& event )
{
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

void SightDialog::OnElevationCheck( wxCommandEvent& event )
{
   /* make sure we always have at least elevation or azimuth */
   if (!(m_elevationUsed->GetValue() || m_azimuthUsed->GetValue()))
      m_azimuthUsed->SetValue(true);

   UpdateElevationAzimuth();
}

void SightDialog::OnAzimuthCheck( wxCommandEvent& event )
{
   /* make sure we always have at least elevation or azimuth */
   if (!(m_elevationUsed->GetValue() || m_azimuthUsed->GetValue()))
      m_elevationUsed->SetValue(true);

   UpdateElevationAzimuth();
}

void SightDialog::UpdateElevationAzimuth()
{
   m_elevationText->Enable(m_elevationUsed->GetValue());
   m_elevationCertaintyText->Enable(m_elevationUsed->GetValue());

   m_azimuthText->Enable(m_azimuthUsed->GetValue());
   m_azimuthMagneticNorth->Enable(m_azimuthUsed->GetValue());
   m_azimuthCertaintyText->Enable(m_azimuthUsed->GetValue());
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

void SightDialog::ReadSight(Sight *s)
{
   int index = m_bodiesChoice->FindString(s->m_Body);
   if(index != wxNOT_FOUND)
      m_bodiesChoice->SetSelection(index);

   m_bodyLimbChoice->SetSelection((int)s->m_BodyLimb);

   m_datetime = s->m_DateTime;
   m_datetimeText->SetLabel(m_datetime.Format(wxDefaultDateTimeFormat));

   m_timeCertaintyText->SetValue(wxString::Format(_T("%f"), s->m_TimeCertainty));

   m_elevationUsed->SetValue(!wxIsNaN(s->m_Elevation));
   if(m_elevationUsed->GetValue())
      m_elevationText->SetValue(wxString::Format(_T("%f"), s->m_Elevation));

   m_elevationCertaintyText->SetValue(wxString::Format(_T("%f"), s->m_ElevationCertainty));

   m_azimuthUsed->SetValue(!wxIsNaN(s->m_Azimuth));
   if(m_azimuthUsed->GetValue())
      m_azimuthText->SetValue(wxString::Format(_T("%f"), s->m_Azimuth));

   m_azimuthMagneticNorth->SetValue(s->m_bMagneticNorth);

   UpdateElevationAzimuth();

   m_azimuthCertaintyText->SetValue(wxString::Format(_T("%f"), s->m_AzimuthCertainty));
   m_heightText->SetValue(wxString::Format(_T("%f"), s->m_Height));

   m_Color->SetForegroundColour(s->m_Colour);
}

Sight *SightDialog::MakeNewSight()
{
   double timecertainty, elevation, elevationcertainty, azimuth, azimuthcertainty, height;
   m_timeCertaintyText->GetValue().ToDouble(&timecertainty);
   m_elevationText->GetValue().ToDouble(&elevation);
   m_elevationCertaintyText->GetValue().ToDouble(&elevationcertainty);
   m_azimuthText->GetValue().ToDouble(&azimuth);
   m_azimuthCertaintyText->GetValue().ToDouble(&azimuthcertainty);
   m_heightText->GetValue().ToDouble(&height);
   
   return new Sight(m_bodiesChoice->GetStringSelection(),
                    (Sight::BodyLimb)m_bodyLimbChoice->GetSelection(),
                    m_datetime, timecertainty,
                    m_elevationUsed->GetValue() ? elevation : NAN, elevationcertainty,
                    m_azimuthUsed->GetValue() ? azimuth : NAN,
                    m_azimuthMagneticNorth->GetValue(),
                    azimuthcertainty, height, m_Color->GetForegroundColour());
}
