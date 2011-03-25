/******************************************************************************
 * $Id: routeprop.h,v 1.10 2010/05/15 03:55:35 bdbcat Exp $
 *
 * Project:  OpenCPN
 * Purpose:  RouteProerties Support
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   $EMAIL$   *
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
 * $Log: routeprop.h,v $
 * Revision 1.10  2010/05/15 03:55:35  bdbcat
 * Build 514
 *
 * Revision 1.9  2010/04/27 01:45:21  bdbcat
 * Build 426
 *
 *
 */

#ifndef _ROUTEPROP_H_
#define _ROUTEPROP_H_

/*!
 * Includes
 */
#include "wx/listctrl.h"
#include "chart1.h"                 // for ColorScheme
#include "wx/hyperlink.h"           // toh, 2009.02.08

#if wxCHECK_VERSION(2, 9, 0)
#include <wx/dialog.h>
#else
#include "scrollingdialog.h"
#endif


WX_DECLARE_LIST(wxHyperlinkCtrl, HyperlinkCtrlList);// establish class as list member

/*!
 * Forward declarations
 */

class   wxListCtrl;
class   Route;
class   RoutePoint;
/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_ROUTEPROP 7000
#define SYMBOL_ROUTEPROP_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_ROUTEPROP_TITLE _("Route Properties")
#define SYMBOL_ROUTEPROP_IDNAME ID_ROUTEPROP
#define SYMBOL_ROUTEPROP_SIZE wxSize(450, 300)
#define SYMBOL_ROUTEPROP_POSITION wxDefaultPosition

#define ID_TEXTCTRL 7001
#define ID_TEXTCTRL2 7002
#define ID_TEXTCTRL1 7003
#define ID_TEXTCTRL3 7005
#define ID_LISTCTRL 7004
#define ID_ROUTEPROP_CANCEL 7006
#define ID_ROUTEPROP_OK 7007
#define ID_ROUTEPROP_SPLIT 7107
#define ID_ROUTEPROP_EXTEND 7207
#define ID_PLANSPEEDCTL 7008
#define ID_TEXTCTRL4 7009
#define ID_TEXTCTRLDESC 7010

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
 * RouteProp class declaration
 */


class RouteProp: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( RouteProp )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    RouteProp( );
    RouteProp(wxWindow* parent, wxWindowID id = SYMBOL_ROUTEPROP_IDNAME,
        const wxString& caption = SYMBOL_ROUTEPROP_TITLE,
        const wxPoint& pos = SYMBOL_ROUTEPROP_POSITION,
        const wxSize& size = SYMBOL_ROUTEPROP_SIZE,
        long style = SYMBOL_ROUTEPROP_STYLE );
    ~RouteProp( );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_ROUTEPROP_IDNAME,
                 const wxString& caption = SYMBOL_ROUTEPROP_TITLE,
                 const wxPoint& pos = SYMBOL_ROUTEPROP_POSITION,
                 const wxSize& size = SYMBOL_ROUTEPROP_SIZE,
                 long style = SYMBOL_ROUTEPROP_STYLE );

    void CreateControls();

    void SetColorScheme(ColorScheme cs);
    void SetDialogTitle(wxString title);

    void OnRoutepropCancelClick( wxCommandEvent& event );
    void OnRoutepropOkClick( wxCommandEvent& event );
    void OnPlanSpeedCtlUpdated( wxCommandEvent& event );
    void OnRoutepropListClick( wxListEvent& event );
    void OnRoutepropSplitClick( wxCommandEvent& event );
    void OnRoutepropExtendClick( wxCommandEvent& event );
    bool IsThisRouteExtendable();
    bool IsThisTrackExtendable();
    void OnEvtColDragEnd(wxListEvent& event);


    /// Should we show tooltips?
    static bool ShowToolTips();

    void SetRouteAndUpdate(Route *pR);
    Route *GetRoute(void){return m_pRoute;}

    bool UpdateProperties(void);
    bool SaveChanges(void);

    wxTextCtrl  *m_TotalDistCtl;
    wxTextCtrl  *m_PlanSpeedCtl;
    wxTextCtrl  *m_TimeEnrouteCtl;

	wxStaticText *m_PlanSpeedLabel;

    wxTextCtrl  *m_RouteNameCtl;
    wxTextCtrl  *m_RouteStartCtl;
    wxTextCtrl  *m_RouteDestCtl;

    wxListCtrl  *m_wpList;

    wxButton*     m_CancelButton;
    wxButton*     m_OKButton;
    wxButton*     m_ExtendButton;
    wxButton*     m_SplitButton;

    Route       *m_pRoute;
    Route       *m_pHead; // for route splitting
    Route       *m_pTail;
      RoutePoint *m_pExtendPoint;
      Route *m_pExtendRoute;

    double      m_planspeed;
    double      m_avgspeed;

    int         m_nSelected; // index of point selected in Properties dialog row
};


/*!
 * MarkProp class declaration
 */
#if wxCHECK_VERSION(2, 9, 0)
class MarkProp: public wxDialog
#else
class MarkProp: public wxScrollingDialog
#endif
{
    DECLARE_DYNAMIC_CLASS( MarkProp )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    MarkProp( );
    MarkProp( wxWindow* parent, wxWindowID id = SYMBOL_MARKPROP_IDNAME,
        const wxString& caption = SYMBOL_MARKPROP_TITLE,
        const wxPoint& pos = SYMBOL_MARKPROP_POSITION,
        const wxSize& size = SYMBOL_MARKPROP_SIZE,
        long style = SYMBOL_MARKPROP_STYLE );

    ~MarkProp();

    /// Creation
      bool Create( wxWindow* parent, wxWindowID id = SYMBOL_MARKPROP_IDNAME,
          const wxString& caption = SYMBOL_MARKPROP_TITLE,
          const wxPoint& pos = SYMBOL_MARKPROP_POSITION,
          const wxSize& size = SYMBOL_MARKPROP_SIZE, long style = SYMBOL_MARKPROP_STYLE );

      void SetRoutePoint(RoutePoint *pRP);
      RoutePoint *GetRoutePoint(void){return m_pRoutePoint;}

      void ValidateMark(void);
      bool UpdateProperties(void);

      void SetColorScheme(ColorScheme cs);

private:
      void CreateControls();

      void OnMarkpropCancelClick( wxCommandEvent& event );
      void OnMarkpropOkClick( wxCommandEvent& event );
      void OnIconListSelected( wxListEvent& event );
      void OnPositionCtlUpdated( wxCommandEvent& event );
      void OnShowNamecheckboxClick( wxCommandEvent& event );

    /// Should we show tooltips?
      static bool ShowToolTips();

      bool SaveChanges(void);

      wxTextCtrl    *m_MarkNameCtl;
      wxTextCtrl    *m_MarkLatCtl;
      wxTextCtrl    *m_MarkLonCtl;
      wxListCtrl    *m_IconList;
      wxTextCtrl    *m_pDescTextCtl;

      wxCheckBox*   m_ShowNameCheckbox;
      wxButton*     m_CancelButton;
      wxButton*     m_OKButton;

      int           m_current_icon_Index;
      double        m_lat_save;
      double        m_lon_save;
      wxString      m_IconName_save;
      bool          m_bShowName_save;

      RoutePoint  *m_pRoutePoint;
};


//    LatLonTextCtrl Specification
//    We need a derived wxText control for lat/lon input in the MarkProp dialog
//    Specifically, we need to catch loss-of-focus events and signal the parent dialog
//    to update the mark's lat/lon dynamically.

//DECLARE_EVENT_TYPE(EVT_LLCHANGE, -1)
extern /*expdecl*/ const wxEventType EVT_LLCHANGE;

class LatLonTextCtrl: public wxTextCtrl
{
//    DECLARE_DYNAMIC_CLASS( LatLonTextCtrl )
    DECLARE_EVENT_TABLE()

public:
      LatLonTextCtrl(wxWindow* parent, wxWindowID id, const wxString& value = _T(""),
            const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
            long style = 0, const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxTextCtrlNameStr);

      void OnKillFocus(wxFocusEvent &event);


      wxEvtHandler *m_pParentEventHandler;

};

// toh, 2009.02.08
/*!
 * MarkInfo class declaration
 */

class MarkInfo: public wxDialog
{
      DECLARE_DYNAMIC_CLASS( MarkInfo )
                  DECLARE_EVENT_TABLE()

      public:
    /// Constructors
            MarkInfo( );
            MarkInfo( wxWindow* parent, wxWindowID id = SYMBOL_MARKINFO_IDNAME,
                      const wxString& caption = SYMBOL_MARKINFO_TITLE,
                      const wxPoint& pos = SYMBOL_MARKINFO_POSITION,
                      const wxSize& size = SYMBOL_MARKINFO_SIZE,
                      long style = SYMBOL_MARKINFO_STYLE );

            ~MarkInfo();

    /// Creation
            bool Create( wxWindow* parent, wxWindowID id = SYMBOL_MARKINFO_IDNAME,
                         const wxString& caption = SYMBOL_MARKINFO_TITLE,
                         const wxPoint& pos = SYMBOL_MARKINFO_POSITION,
                         const wxSize& size = SYMBOL_MARKINFO_SIZE, long style = SYMBOL_MARKINFO_STYLE );

            void SetRoutePoint(RoutePoint *pRP);
            RoutePoint *GetRoutePoint(void){return m_pRoutePoint;}

            void SetColorScheme(ColorScheme cs);

            void CreateControls();

            void OnMarkinfoCancelClick( wxCommandEvent& event );
            void OnMarkinfoOkClick( wxCommandEvent& event );

    /// Should we show tooltips?
            static bool ShowToolTips();

            bool UpdateProperties(void);
            bool SaveChanges(void);

            wxStaticText*   m_MarkNameCtl;
            wxStaticText*   m_MarkLatCtl;
            wxStaticText*   m_MarkLonCtl;
            wxButton*     m_CancelButton;
            wxButton*     m_OKButton;

            wxHyperlinkCtrl *m_HyperlinkCtrl;
            wxHyperlinkCtrl *m_HyperlinkCtrl2;

            double        m_lat_save;
            double        m_lon_save;

            RoutePoint  *m_pRoutePoint;

            HyperlinkCtrlList *m_HyperlinkCtrlList;   // toh, 2009.02.11
};


#endif
    // _ROUTEPROP_H_
