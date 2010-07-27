/******************************************************************************
 * $Id: sightdialog.h,v 1.10 2010/05/15 03:55:35 bdbcat Exp $
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
 */

#ifndef _SIGHTDIALOG_H_
#define _SIGHTDIALOG_H_

/*!
 * Includes
 */
#include "wx/listctrl.h"
#include "wx/hyperlink.h"           // toh, 2009.02.08
#include "scrollingdialog.h"

/*!
 * Forward declarations
 */

class   wxListCtrl;
/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_SIGHTDIALOG 7000
#define SYMBOL_SIGHTDIALOG_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_SIGHTDIALOG_TITLE _("Sight Properties")
#define SYMBOL_SIGHTDIALOG_IDNAME ID_SIGHTDIALOG
#define SYMBOL_SIGHTDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_SIGHTDIALOG_POSITION wxDefaultPosition

#define ID_SIGHTDIALOG_DATETIME_SET 7006
#define ID_SIGHTDIALOG_COLOR_SET 7007
#define ID_SIGHTDIALOG_ELEVATION_CHECK 7008
#define ID_SIGHTDIALOG_AZIMUTH_CHECK 7009
#define ID_SIGHTDIALOG_CANCEL 7010
#define ID_SIGHTDIALOG_OK 7011

#define ID_MARKPROP 8000
#define SYMBOL_MARKPROP_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_MARKPROP_TITLE _("Mark Properties")
#define SYMBOL_MARKPROP_IDNAME ID_MARKPROP
#define SYMBOL_MARKPROP_SIZE wxSize(200, 300)
#define SYMBOL_MARKPROP_POSITION wxDefaultPosition
#define ID_MARKPROP_CANCEL 8001
#define ID_MARKPROP_OK 8002
#define ID_ICONCTRL 8003
#define ID_LATCTRL 8004
#define ID_LONCTRL 8005
#define ID_SHOWNAMECHECKBOX1 8006

// toh, 2009.02.08
#define ID_MARKINFO 8007
#define SYMBOL_MARKINFO_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_MARKINFO_TITLE _("Mark Information")
#define SYMBOL_MARKINFO_IDNAME ID_MARKINFO
#define SYMBOL_MARKINFO_SIZE wxSize(200, 300)
#define SYMBOL_MARKINFO_POSITION wxDefaultPosition
#define ID_MARKINFO_CANCEL 8008
#define ID_MARKINFO_OK 8009


////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif
#ifndef wxFIXED_MINSIZE
#define wxFIXED_MINSIZE 0
#endif

/*!
 * SightDialog class declaration
 */

class Sight;

class SightDialog: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( SightDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    SightDialog();
    SightDialog(wxWindow* parent, wxWindowID id = SYMBOL_SIGHTDIALOG_IDNAME,
        const wxString& caption = SYMBOL_SIGHTDIALOG_TITLE,
        const wxPoint& pos = SYMBOL_SIGHTDIALOG_POSITION,
        const wxSize& size = SYMBOL_SIGHTDIALOG_SIZE,
        long style = SYMBOL_SIGHTDIALOG_STYLE );
    ~SightDialog( );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_SIGHTDIALOG_IDNAME,
                 const wxString& caption = SYMBOL_SIGHTDIALOG_TITLE,
                 const wxPoint& pos = SYMBOL_SIGHTDIALOG_POSITION,
                 const wxSize& size = SYMBOL_SIGHTDIALOG_SIZE,
                 long style = SYMBOL_SIGHTDIALOG_STYLE );

    void CreateControls();

 //   void SetColorScheme(ColorScheme cs);
    void SetDialogTitle(wxString title);

    void OnSightDialogDateTimeSetClick( wxCommandEvent& event );
    void OnSightDialogColorSetClick( wxCommandEvent& event );
    void OnElevationCheck( wxCommandEvent& event );
    void OnAzimuthCheck( wxCommandEvent& event );
    void UpdateElevationAzimuth();
    void OnSightDialogCancelClick( wxCommandEvent& event );
    void OnSightDialogOkClick( wxCommandEvent& event );
    void OnClose(wxCloseEvent& event);

    void ReadSight(Sight *s);
    Sight *MakeNewSight();


    /// Should we show tooltips?
    static bool ShowToolTips();

    wxChoice*   m_bodiesChoice;
    wxChoice*   m_bodyLimbChoice;

    wxStaticText* m_datetimeText;
    wxDateTime    m_datetime;
    wxTextCtrl*   m_timeCertaintyText;

    wxCheckBox*   m_elevationUsed;
    wxTextCtrl*   m_elevationText, *m_elevationCertaintyText;

    wxCheckBox*   m_azimuthUsed, *m_azimuthMagneticNorth;
    wxTextCtrl*   m_azimuthText, *m_azimuthCertaintyText;

    wxTextCtrl*   m_heightText;

    wxStaticText*   m_Color;

    wxButton*     m_ColorSetButton;
    wxButton*     m_CancelButton;
    wxButton*     m_OKButton;
};



#endif
    // _SIGHTDIALOG_H_
