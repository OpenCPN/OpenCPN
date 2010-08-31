/******************************************************************************
 * $Id: findbodydialog.cpp,v 1.21 2010/05/20 19:05:13 bdbcat Exp $
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
 * $Log: findbodydialog.cpp,v $
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

#include "findbodydialog.h"

#include "../../../include/ocpn_plugin.h"

#include "sight.h"
#include "celestial_navigation_pi.h"
#include "geodesic.h"

/*!
 * FindBodyDialog type definition
 */

IMPLEMENT_DYNAMIC_CLASS( FindBodyDialog, wxDialog )

/*!
 * FindBodyDialog event table definition
 */

BEGIN_EVENT_TABLE( FindBodyDialog, wxDialog )

    EVT_TEXT( ID_FINDBODYDIALOG_UPDATE_TEXT, FindBodyDialog::OnUpdate )
    EVT_CHECKBOX( ID_FINDBODYDIALOG_UPDATE_CHECK, FindBodyDialog::OnUpdate )
    EVT_BUTTON( ID_FINDBODYDIALOG_DONE, FindBodyDialog::OnDoneClick )
    EVT_CLOSE(FindBodyDialog::OnClose )

END_EVENT_TABLE()

/*!
 * FindBodyDialog constructors
 */
FindBodyDialog::FindBodyDialog() {}

FindBodyDialog::FindBodyDialog( wxWindow* parent, Sight *sight, wxWindowID id,
                     const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
      Create(parent, sight, id, caption, pos, size, style);
      GetSizer()->SetSizeHints(this);
      Centre();
}


FindBodyDialog::~FindBodyDialog( )
{
}


/*!
 * FindBodyDialog creator
 */

bool FindBodyDialog::Create( wxWindow* parent, Sight *sight, wxWindowID id,
                             const wxString& caption, const wxPoint& pos,
                             const wxSize& size, long style )
{
    m_Sight = sight;

    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();

    return TRUE;
}

/*!
 * Control creation for FindBodyDialog
 */

void FindBodyDialog::CreateControls()
{
////@begin FindBodyDialog content construction

    FindBodyDialog* itemDialog1 = this;

    wxGridSizer* itemGrid = new wxGridSizer(5);
    itemDialog1->SetSizer(itemGrid);

    itemGrid->Add(new wxStaticText( itemDialog1, wxID_STATIC, _("Latitude"),
                                    wxDefaultPosition, wxDefaultSize, 0 ), 0,
                  wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_Latitude = new wxTextCtrl( itemDialog1, ID_FINDBODYDIALOG_UPDATE_TEXT,
                                 wxString::Format(_T("%f"), celestial_navigation_pi_CursorLat()));
    itemGrid->Add(m_Latitude, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    itemGrid->Add(new wxStaticText( itemDialog1, wxID_STATIC, _("Longitude"),
                                    wxDefaultPosition, wxDefaultSize, 0 ), 0,
                  wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_Longitude = new wxTextCtrl( itemDialog1, ID_FINDBODYDIALOG_UPDATE_TEXT,
                                  wxString::Format(_T("%f"), celestial_navigation_pi_CursorLon()));
    itemGrid->Add(m_Longitude, 0, wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_azimuthMagneticNorth = new wxCheckBox( itemDialog1, ID_FINDBODYDIALOG_UPDATE_CHECK, _("Magnetic North"),
                                             wxDefaultPosition, wxDefaultSize, 0);
    itemGrid->Add(m_azimuthMagneticNorth, 1, wxALIGN_RIGHT|wxADJUST_MINSIZE, 5);

    itemGrid->Add(new wxStaticText( itemDialog1, wxID_STATIC, _("Altitude:"),
                                    wxDefaultPosition, wxDefaultSize, 0 ), 0,
                  wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_Altitude = new wxStaticText( itemDialog1, wxID_STATIC, _(""),
                                   wxDefaultPosition, wxDefaultSize, 0 );
    itemGrid->Add(m_Altitude, 0,
                  wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    itemGrid->Add(new wxStaticText( itemDialog1, wxID_STATIC, _("Azimuth:"),
                                    wxDefaultPosition, wxDefaultSize, 0 ), 0,
                  wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_Azimuth = new wxStaticText( itemDialog1, wxID_STATIC, _(""),
                                  wxDefaultPosition, wxDefaultSize, 0 );

    itemGrid->Add(m_Azimuth, 0,
                  wxALIGN_LEFT|wxLEFT|wxRIGHT|wxTOP|wxADJUST_MINSIZE, 5);

    m_DoneButton = new wxButton( itemDialog1, ID_FINDBODYDIALOG_DONE, _("Done"),
                               wxDefaultPosition, wxDefaultSize, 0 );
    itemGrid->Add(m_DoneButton, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
    m_DoneButton->SetDefault();

    Update();

    // note that under MSW for SetColumnWidth() to work we need to create the
    // items with images initially even if we specify dummy image id
    wxListItem itemCol;
    itemCol.SetImage(-1);

//    SetColorScheme((ColorScheme)0);
}

/*!
 * Should we show tooltips?
 */

bool FindBodyDialog::ShowToolTips()
{
    return TRUE;
}


void FindBodyDialog::SetDialogTitle(wxString title)
{
      SetTitle(title);
}

extern "C" int geomag_calc(double latitude, double longitude, double alt,
                int day, int month, double year,
                           double results[14]);

void FindBodyDialog::OnUpdate( wxCommandEvent& event )
{
   Update();
}

void FindBodyDialog::Update()
{
   double lat1, lon1, lat2, lon2, bearing, dist;
   m_Latitude->GetValue().ToDouble(&lat1);
   m_Longitude->GetValue().ToDouble(&lon1);

   m_Sight->BodyLocation(m_Sight->m_DateTime, lat2, lon2);

   ll_gc_ll_reverse(lat1, lon1, lat2, lon2, &bearing, &dist);

   dist = 90 - dist/60;

   if(m_azimuthMagneticNorth->GetValue()) {
          double results[14];

          geomag_calc(lat1, lon2, m_Sight->m_Height,
                      m_Sight->m_DateTime.GetDay(), m_Sight->m_DateTime.GetMonth(),
                      m_Sight->m_DateTime.GetYear(), results);
          bearing -= results[0];
   }

   m_Altitude->SetLabel(wxString::Format(_T("%f"), dist));
   m_Azimuth->SetLabel(wxString::Format(_T("%f"), bearing));
}

void FindBodyDialog::OnDoneClick( wxCommandEvent& event )
{
   EndModal(wxID_OK);
}

void FindBodyDialog::OnClose(wxCloseEvent& event)
{
   EndModal(wxID_OK);
}
