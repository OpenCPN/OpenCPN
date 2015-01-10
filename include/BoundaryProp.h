/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Boundary Properties Support
 * Author:   Jon Gough
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

#ifndef BOUNDARYPROP_H
#define BOUNDARYPROP_H

#include <wx/clrpicker.h>

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_BOUNDARYPROP 9000
#define SYMBOL_BOUNDARYPROP_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_BOUNDARYPROP_TITLE _("Boundary Properties")
#define SYMBOL_BOUNDARYPROP_IDNAME ID_BOUNDARYPROP
#define SYMBOL_BOUNDARYPROP_SIZE wxSize(450, 300)
#define SYMBOL_BOUNDARYPROP_POSITION wxDefaultPosition

#define ID_TEXTCTRL            9001
#define ID_TEXTCTRL2           9002
#define ID_TEXTCTRL1           9003
#define ID_TEXTCTRL3           9005
#define ID_LISTCTRL            9004
#define ID_BOUNDARYPROP_CANCEL    9006
#define ID_BOUNDARYPROP_OK        9007
#define ID_BOUNDARYPROP_SPLIT     9107
#define ID_BOUNDARYPROP_EXTEND    9207
#define ID_BOUNDARYPROP_COPYTXT   9307
#define ID_BOUNDARYPROP_PRINT     9407
//#define ID_PLANSPEEDCTL        9008
#define ID_TEXTCTRL4           9009
#define ID_TEXTCTRLDESC        9010
#define ID_STARTTIMECTL        9011
//#define ID_TIMEZONESEL         9012
#define ID_TRACKLISTCTRL       9013
#define ID_RCLK_MENU_COPY_TEXT 9014
#define ID_RCLK_MENU_EDIT_WP   9015
#define ID_RCLK_MENU_DELETE    9016
#define ID_RCLK_MENU_COPY      9017
#define ID_RCLK_MENU_COPY_LL   9018
#define ID_RCLK_MENU_PASTE     9019
#define ID_RCLK_MENU_PASTE_LL  9020
//#define ID_TIMEZONESEL_UTC     9021
//#define ID_TIMEZONESEL_LOCAL   9022
//#define ID_TIMEZONESEL_LMT     9023


class BoundaryProp : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( BoundaryProp )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    BoundaryProp( );
    BoundaryProp(wxWindow* parent, wxWindowID id = SYMBOL_BOUNDARYPROP_IDNAME,
        const wxString& caption = SYMBOL_BOUNDARYPROP_TITLE,
        const wxPoint& pos = SYMBOL_BOUNDARYPROP_POSITION,
        const wxSize& size = SYMBOL_BOUNDARYPROP_SIZE,
        long style = SYMBOL_BOUNDARYPROP_STYLE );
    ~BoundaryProp( );

    void CreateControls();

    void SetColorScheme(ColorScheme cs);
    void SetDialogTitle(const wxString & title);
    void OnBoundarypropCancelClick( wxCommandEvent& event );
    void OnBoundarypropOkClick( wxCommandEvent& event );
    void OnBoundarypropListClick( wxListEvent& event );
//    void OnRoutepropPrintClick( wxCommandEvent& event );
//    void OnRoutepropCopyTxtClick( wxCommandEvent& event );
    void OnBoundaryPropRightClick( wxListEvent &event );
    void OnBoundaryPropMenuSelected( wxCommandEvent &event );
//    bool IsThisRouteExtendable();
    void OnEvtColDragEnd(wxListEvent& event);
    void InitializeList();


    /// Should we show tooltips?
    static bool ShowToolTips();

    void SetBoundaryAndUpdate(Boundary *pB, bool only_points = false);
    Boundary *GetBoundary(void){return m_pBoundary;}

    bool UpdateProperties(void);
    wxString MakeTideInfo(int jx, time_t tm, int tz_selection, long LMT_Offset);
    bool SaveChanges(void);

    wxTextCtrl  *m_TotalDistCtl;

    wxTextCtrl  *m_BoundaryNameCtl;
    wxTextCtrl  *m_textDescription;


    wxListCtrl        *m_wpList;

    wxButton*     m_CancelButton;
    wxButton*     m_OKButton;
    wxButton*     m_CopyTxtButton;

    Boundary     *m_pBoundary;
    RoutePoint *m_pExtendPoint;
    Route *m_pExtendRoute;
    RoutePoint    *m_pEnroutePoint;
    bool          m_bStartNow;

    int         m_nSelected; // index of point selected in Properties dialog row
    int         m_tz_selection;

    wxDateTime	 m_starttime; // kept as UTC
//    wxRadioBox	*pDispTz;
    wxStaticText  *m_staticText1;
    wxStaticText  *m_staticText2;
    wxStaticText  *m_staticText3;
    wxChoice      *m_chColor;
    wxChoice      *m_chStyle;
    wxChoice      *m_chWidth;

    wxStaticBoxSizer* m_pListSizer;
    wxScrolledWindow *itemDialog1;
    
    wxColourPickerCtrl  *m_colourBoundary;
    
private:
    int GetTZSelection(void);
    wxRadioButton  *m_prb_tzUTC;
    wxRadioButton  *m_prb_tzLocal;
    wxRadioButton  *m_prb_tzLMT;
    
};


#endif // BOUNDARYPROP_H
