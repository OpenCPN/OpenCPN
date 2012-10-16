/******************************************************************************
 *
 * Project:  OpenCP
 * Purpose:  OpenCP Main wxWidgets Program
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
 *
 */
#include <iostream>
using namespace std;


#ifndef __ROUTEPRINTOUT_H__
#define __ROUTEPRINTOUT_H__

#include <wx/print.h>
#include <wx/datetime.h>
#include <wx/cmdline.h>

#ifdef __WXMSW__
#include <wx/msw/private.h>
#endif

#include "ocpn_types.h"
#include "navutil.h"
#include "printtable.h"

class MyRoutePrintout: public MyPrintout
{
 public:
  MyRoutePrintout( std::vector<bool> _toPrintOut, Route * route, const wxChar *title = _T("My Route printout"));
  virtual
  bool OnPrintPage(int page);
  void DrawPage(wxDC *dc);
  virtual
  void OnPreparePrinting();
  
  virtual 
  bool HasPage( int num )  {return num > 0 || num <= 1; };
  
  virtual
    void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
  
  protected:
  wxDC * myDC;
  PrintTable table;
  Route * myRoute;
  std::vector<bool> toPrintOut; // list of fields of bool, if certain element should be print out.
  static const int pN = 5; // number of fields sofar
  int	pageToPrint;
  int   numberOfPages;
  int marginX;
  int marginY;
  
};


// route elements selection dialog
////@begin control identifiers
#define ID_ROUTEPRINTSELECTION 9000
#define SYMBOL_ROUTEPRINT_SELECTION_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_ROUTEPRINT_SELECTION_TITLE _("Print Route Selection")
#define SYMBOL_ROUTEPRINT_SELECTION_IDNAME ID_ROUTEPRINTSELECTION
#define SYMBOL_ROUTEPRINT_SELECTION_SIZE wxSize(750, 300)
#define SYMBOL_ROUTEPRINT_SELECTION_POSITION wxDefaultPosition

#define ID_ROUTEPRINT_SELECTION_OK 9001
#define ID_ROUTEPRINT_SELECTION_CANCEL 9002


class RoutePrintSelection: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( RoutePrintSelection )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    RoutePrintSelection( );
    RoutePrintSelection(wxWindow* parent, Route * route, wxWindowID id = SYMBOL_ROUTEPRINT_SELECTION_IDNAME,
        const wxString& caption = SYMBOL_ROUTEPRINT_SELECTION_TITLE,
        const wxPoint& pos = SYMBOL_ROUTEPRINT_SELECTION_POSITION,
        const wxSize& size = SYMBOL_ROUTEPRINT_SELECTION_SIZE,
        long style = SYMBOL_ROUTEPRINT_SELECTION_STYLE );
    ~RoutePrintSelection( );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_ROUTEPRINT_SELECTION_IDNAME,
                 const wxString& caption = SYMBOL_ROUTEPRINT_SELECTION_TITLE,
                 const wxPoint& pos = SYMBOL_ROUTEPRINT_SELECTION_POSITION,
                 const wxSize& size = SYMBOL_ROUTEPRINT_SELECTION_SIZE,
                 long style = SYMBOL_ROUTEPRINT_SELECTION_STYLE );

    void CreateControls();

    void SetColorScheme(ColorScheme cs);
    void SetDialogTitle(wxString title);
    void OnRoutepropCancelClick( wxCommandEvent& event );
    void OnRoutepropOkClick( wxCommandEvent& event );
//     void OnPlanSpeedCtlUpdated( wxCommandEvent& event );
//     void OnStartTimeCtlUpdated( wxCommandEvent& event );
//     void OnTimeZoneSelected( wxCommandEvent& event );
//     void OnRoutepropListClick( wxListEvent& event );
//     void OnRoutepropSplitClick( wxCommandEvent& event );
//     void OnRoutepropExtendClick( wxCommandEvent& event );
//     void OnRoutepropPrintClick( wxCommandEvent& event );
//     void OnRoutepropCopyTxtClick( wxCommandEvent& event );
//     void OnRoutePropRightClick( wxListEvent &event );
//     void OnRoutePropMenuSelected( wxCommandEvent &event );
//     bool IsThisRouteExtendable();
//     bool IsThisTrackExtendable();
//     void OnEvtColDragEnd(wxListEvent& event);
//     void InitializeList();


    /// Should we show tooltips?
    static bool ShowToolTips();

//     void SetRouteAndUpdate(Route *pR);
//     Route *GetRoute(void){return m_pRoute;}

//     bool UpdateProperties(void);
//     wxString MakeTideInfo(int jx, time_t tm, int tz_selection, long LMT_Offset);
//     bool SaveChanges(void);
/*
    wxTextCtrl  *m_TotalDistCtl;
    wxTextCtrl  *m_PlanSpeedCtl;
    wxTextCtrl	*m_StartTimeCtl;
    wxTextCtrl  *m_TimeEnrouteCtl;

    wxStaticText *m_PlanSpeedLabel;
    wxStaticText *m_StartTimeLabel;

    wxTextCtrl  *m_RouteNameCtl;
    wxTextCtrl  *m_RouteStartCtl;
    wxTextCtrl  *m_RouteDestCtl;

    wxListCtrl        *m_wpList;
    OCPNTrackListCtrl *m_wpTrackList;*/

    wxButton*     m_CancelButton;
    wxButton*     m_OKButton;
    
    wxCheckBox * m_checkBoxWPName;
    wxCheckBox * m_checkBoxWPPosition;   
    wxCheckBox * m_checkBoxWPCourse;
    wxCheckBox * m_checkBoxWPDistanceToNext;
    wxCheckBox * m_checkBoxWPDescription;
    
    Route *       route;
    
//     wxButton*     m_CopyTxtButton;
//     wxButton*     m_PrintButton;    
//     wxButton*     m_ExtendButton;
//     wxButton*     m_SplitButton;

//     Route       *m_pRoute;
//     Route       *m_pHead; // for route splitting
//     Route       *m_pTail;
//     RoutePoint *m_pExtendPoint;
//     Route *m_pExtendRoute;
//     RoutePoint    *m_pEnroutePoint;
//     bool          m_bStartNow;
// 
//     double      m_planspeed;
//     double      m_avgspeed;
// 
//     int         m_nSelected; // index of point selected in Properties dialog row
//     int         m_tz_selection;
// 
//     wxDateTime	 m_starttime; // kept as UTC
//     wxRadioBox	*pDispTz;
//     wxStaticText  *m_staticText1;
//     wxStaticText  *m_staticText2;
//     wxStaticText  *m_staticText3;
//     wxChoice      *m_chColor;
//     wxChoice      *m_chStyle;
//     wxChoice      *m_chWidth;
// 
//     wxStaticBoxSizer* m_pListSizer;
};

#endif
