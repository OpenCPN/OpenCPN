/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  RouteProerties Support
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

#ifndef _ROUTEPROP_H_
#define _ROUTEPROP_H_

/*!
 * Includes
 */
#include "wx/listctrl.h"
#include "chart1.h"                 // for ColorScheme
#include "wx/hyperlink.h"           // toh, 2009.02.08
#include <wx/choice.h>
#include <wx/tglbtn.h>
#include <wx/bmpcbox.h>
#include <wx/notebook.h>
#include <wx/filesys.h>
#include <wx/clrpicker.h>
#include "LinkPropDlg.h"

#if wxCHECK_VERSION(2, 9, 0)
#include <wx/dialog.h>
#else
#include "scrollingdialog.h"
#endif

/*!
 * Forward declarations
 */

class   wxListCtrl;
class   Route;
class   RoutePoint;
class   HyperlinkList;

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_ROUTEPROP 7000
#define SYMBOL_ROUTEPROP_TITLE _("Route Properties")
#define SYMBOL_ROUTEPROP_IDNAME ID_ROUTEPROP
#define SYMBOL_ROUTEPROP_SIZE wxSize(450, 300)
#define SYMBOL_ROUTEPROP_POSITION wxDefaultPosition

#ifdef __WXOSX__
#define SYMBOL_ROUTEPROP_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX|wxSTAY_ON_TOP
#else
#define SYMBOL_ROUTEPROP_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#endif


#define ID_TEXTCTRL            7001
#define ID_TEXTCTRL2           7002
#define ID_TEXTCTRL1           7003
#define ID_TEXTCTRL3           7005
#define ID_LISTCTRL            7004
#define ID_ROUTEPROP_CANCEL    7006
#define ID_ROUTEPROP_OK        7007
#define ID_ROUTEPROP_SPLIT     7107
#define ID_ROUTEPROP_EXTEND    7207
#define ID_ROUTEPROP_COPYTXT   7307
#define ID_ROUTEPROP_PRINT     7407
#define ID_WAYPOINTRANGERINGS  7507 
#define ID_SHOWWAYPOINTRANGERINGS  7607 
#define ID_PLANSPEEDCTL        7008
#define ID_TEXTCTRL4           7009
#define ID_TEXTCTRLDESC        7010
#define ID_STARTTIMECTL        7011
#define ID_TIMEZONESEL         7012
#define ID_TRACKLISTCTRL       7013
#define ID_RCLK_MENU_COPY_TEXT 7014
#define ID_RCLK_MENU_EDIT_WP   7015
#define ID_RCLK_MENU_DELETE    7016
#define ID_RCLK_MENU_COPY      7017
#define ID_RCLK_MENU_COPY_LL   7018
#define ID_RCLK_MENU_PASTE     7019
#define ID_RCLK_MENU_PASTE_LL  7020
#define ID_TIMEZONESEL_UTC     7021
#define ID_TIMEZONESEL_LOCAL   7022
#define ID_TIMEZONESEL_LMT     7023

#define ID_MARKPROP 8000
#define SYMBOL_MARKPROP_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_MARKPROP_TITLE _("Waypoint Properties")
#define SYMBOL_MARKPROP_IDNAME ID_MARKPROP
#define SYMBOL_MARKPROP_SIZE wxSize(200, 300)
#define SYMBOL_MARKPROP_POSITION wxDefaultPosition
#define ID_MARKPROP_CANCEL 8001
#define ID_MARKPROP_OK 8002
#define ID_ICONCTRL 8003
#define ID_LATCTRL 8004
#define ID_LONCTRL 8005
#define ID_SHOWNAMECHECKBOX1 8006

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
    static RouteProp* getInstance(wxWindow* parent, wxWindowID id = SYMBOL_ROUTEPROP_IDNAME,
                                   const wxString& caption = SYMBOL_ROUTEPROP_TITLE,
                                   const wxPoint& pos = SYMBOL_ROUTEPROP_POSITION,
                                   const wxSize& size = SYMBOL_ROUTEPROP_SIZE,
                                   long style = SYMBOL_ROUTEPROP_STYLE );
    ~RouteProp( );
    static bool getInstanceFlag(){ return instanceFlag; } 
    
    void CreateControls();

    void SetColorScheme(ColorScheme cs);
    void SetDialogTitle(const wxString & title);
    void OnRoutepropCancelClick( wxCommandEvent& event );
    void OnRoutepropOkClick( wxCommandEvent& event );
    void OnPlanSpeedCtlUpdated( wxCommandEvent& event );
    void OnStartTimeCtlUpdated( wxCommandEvent& event );
    void OnTimeZoneSelected( wxCommandEvent& event );
    void OnRoutepropListClick( wxListEvent& event );
    void OnRoutepropSplitClick( wxCommandEvent& event );
    void OnRoutepropExtendClick( wxCommandEvent& event );
    void OnRoutepropPrintClick( wxCommandEvent& event );
    void OnRoutepropCopyTxtClick( wxCommandEvent& event );
    void OnRoutePropRightClick( wxListEvent &event );
    void OnRoutePropMenuSelected( wxCommandEvent &event );
    bool IsThisRouteExtendable();
    void OnEvtColDragEnd(wxListEvent& event);
    void InitializeList();
    void RecalculateSize( void );
    
    /// Should we show tooltips?
    static bool ShowToolTips();

    void SetRouteAndUpdate(Route *pR, bool only_points = false);
    Route *GetRoute(void){return m_pRoute;}

    bool UpdateProperties(void);
    wxString MakeTideInfo(int jx, time_t tm, int tz_selection, long LMT_Offset);
    bool SaveChanges(void);

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

    wxButton*     m_CancelButton;
    wxButton*     m_OKButton;
    wxButton*     m_CopyTxtButton;
    wxButton*     m_PrintButton;
    wxButton*     m_ExtendButton;
    wxButton*     m_SplitButton;

    Route       *m_pRoute;
    Route       *m_pHead; // for route splitting
    Route       *m_pTail;
    RoutePoint *m_pExtendPoint;
    Route *m_pExtendRoute;
    RoutePoint    *m_pEnroutePoint;
    bool          m_bStartNow;

    double      m_planspeed;
    double      m_avgspeed;

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
    
private:
    RouteProp( );
    RouteProp(wxWindow* parent, wxWindowID id = SYMBOL_ROUTEPROP_IDNAME,
              const wxString& caption = SYMBOL_ROUTEPROP_TITLE,
              const wxPoint& pos = SYMBOL_ROUTEPROP_POSITION,
              const wxSize& size = SYMBOL_ROUTEPROP_SIZE,
              long style = SYMBOL_ROUTEPROP_STYLE );
    
    static bool instanceFlag;
    static RouteProp *single;
    
    int GetTZSelection(void);
    wxRadioButton  *m_prb_tzUTC;
    wxRadioButton  *m_prb_tzLocal;
    wxRadioButton  *m_prb_tzLMT;
    
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

///////////////////////////////////////////////////////////////////////////////
/// Class MarkInfoDef
///////////////////////////////////////////////////////////////////////////////
class MarkInfoDef : public wxDialog
{
    DECLARE_EVENT_TABLE()
    
	private:

	protected:
        wxBoxSizer*             bSizerLinks;
        wxBitmapComboBox*       m_bcomboBoxIcon;
        wxStaticBitmap*         m_bitmapIcon;
        wxButton*               m_buttonAddLink;
        wxButton*               m_buttonExtDescription;
        wxCheckBox*             m_checkBoxShowName;
        wxCheckBox*             m_checkBoxVisible;
        wxHyperlinkCtrl*        m_hyperlink17;
        wxObject*               m_contextObject;
        wxMenu*                 m_menuLink;
        wxNotebook*             m_notebookProperties;
        wxScrolledWindow*       m_panelBasicProperties;
        wxPanel*                m_panelDescription;
        wxPanel*                m_panelExtendedProperties;
        wxScrolledWindow*       m_scrolledWindowLinks;
        wxStdDialogButtonSizer* m_sdbSizerButtons;
        wxButton*               m_sdbSizerButtonsCancel;
        wxButton*               m_sdbSizerButtonsOK;
        wxStaticText*           m_staticTextDescription;
        wxStaticText*           m_staticTextEditEnabled;
        wxStaticText*           m_staticTextGpx;
        wxStaticText*           m_staticTextGuid;
        wxStaticText*           m_staticTextIcon;
        wxStaticText*           m_staticTextLatitude;
        wxStaticText*           m_staticTextLayer;
        wxStaticText*           m_staticTextLongitude;
        wxStaticText*           m_staticTextName;
        wxTextCtrl*             m_textCtrlExtDescription;
        wxTextCtrl*             m_textCtrlGpx;
        wxTextCtrl*             m_textCtrlGuid;
        wxTextCtrl*             m_textDescription;
        wxTextCtrl*             m_textLatitude;
        wxTextCtrl*             m_textLongitude;
        wxTextCtrl*             m_textName;
        wxToggleButton*         m_toggleBtnEdit;
        wxStaticBoxSizer*       sbSizerLinks;
        wxSize                  m_defaultClientSize;
        wxStaticText*           m_staticTextArrivalRadius;
        wxTextCtrl*             m_textArrivalRadius;
        wxBoxSizer*             bSizerBasicProperties;
        wxCheckBox*             m_checkBoxShowWaypointRangeRings;
    
        wxChoice*               m_choiceWaypointRangeRingsNumber;
        wxFlexGridSizer*        waypointradarGrid;
        wxTextCtrl*             m_textWaypointRangeRingsStep;
        wxChoice*               m_choiceWaypointRangeRingsUnits;
        wxFlexGridSizer*        waypointrrSelect;
        wxColourPickerCtrl*     m_colourWaypointRangeRingsColour;
        
    // Virtual event handlers, overide them in your derived class
        virtual void OnPositionCtlUpdated( wxCommandEvent& event ) { event.Skip(); }
        virtual void OnDescChangedBasic( wxCommandEvent& event ) { event.Skip(); }
        virtual void OnExtDescriptionClick( wxCommandEvent& event ) { event.Skip(); }
        virtual void OnDeleteLink( wxCommandEvent& event ) { event.Skip(); }
        virtual void OnEditLink( wxCommandEvent& event ) { event.Skip(); }
        virtual void OnAddLink( wxCommandEvent& event ) { event.Skip(); }
        virtual void OnEditLinkToggle( wxCommandEvent& event ) { event.Skip(); }
        virtual void OnDescChangedExt( wxCommandEvent& event ) { event.Skip(); }
        virtual void OnMarkInfoCancelClick( wxCommandEvent& event ) { event.Skip(); }
        virtual void OnMarkInfoOKClick( wxCommandEvent& event ) { event.Skip(); }
        virtual void OnArrivalRadiusChange( wxCommandEvent& event ) { event.Skip(); }
        virtual void OnWaypointRangeRingsStepChange( wxCommandEvent& event ) { event.Skip(); }
        void OnCopyPasteLatLon( wxCommandEvent& event );
        void OnWaypointRangeRingSelect( wxCommandEvent& event );
        void OnShowWaypointRangeRingSelect( wxCommandEvent& event );

    public:
        MarkInfoDef( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Waypoint Properties"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1, -1 ), long style = wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxRESIZE_BORDER );
        ~MarkInfoDef();

        void RecalculateSize( void );
        
        void m_hyperlink17OnContextMenu( wxMouseEvent &event )
        {
            m_hyperlink17->PopupMenu( m_menuLink, event.GetPosition() );
        }

};

class MarkInfoImpl : public MarkInfoDef
{
public :
      void SetColorScheme( ColorScheme cs );
      void OnMarkInfoOKClick( wxCommandEvent& event );
      void OnMarkInfoCancelClick( wxCommandEvent& event );
      void SetRoutePoint( RoutePoint *pRP );
      void SetDialogTitle(const wxString & title) { SetTitle(title); }
      RoutePoint *GetRoutePoint(void) { return m_pRoutePoint; }
      bool UpdateProperties( bool positionOnly = false );
      void ValidateMark(void);
      void InitialFocus(void);
      void OnRightClick( wxCommandEvent& event );

      static MarkInfoImpl *getInstance( wxWindow* parent,
                          wxWindowID id = wxID_ANY,
                          const wxString& title = _("Waypoint Information"),
                          const wxPoint& pos = wxDefaultPosition,
                          const wxSize& size = wxSize( -1, -1 ),
                          long style = wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxRESIZE_BORDER );
      static bool getInstanceFlag(){ return instanceFlag; } 
      ~MarkInfoImpl();

      void m_hyperlinkContextMenu( wxMouseEvent &event );

protected :
      virtual void OnPositionCtlUpdated( wxCommandEvent& event );
      void OnDeleteLink( wxCommandEvent& event );
      void OnEditLink( wxCommandEvent& event );
      void OnAddLink( wxCommandEvent& event );
      void OnEditLinkToggle( wxCommandEvent& event );
      void OnDescChangedBasic( wxCommandEvent& event );
      void OnDescChangedExt( wxCommandEvent& event );
      void OnExtDescriptionClick( wxCommandEvent& event );

private :
    MarkInfoImpl( wxWindow* parent,
                  wxWindowID id = wxID_ANY,
                  const wxString& title = _("Waypoint Information"),
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxSize( -1, -1 ),
                  long style = wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxRESIZE_BORDER );
    
    static bool instanceFlag;
    static MarkInfoImpl *single;
    
      RoutePoint  *m_pRoutePoint;
      HyperlinkList *m_pMyLinkList;
      void OnHyperLinkClick(wxHyperlinkEvent &event);
      LinkPropImpl* m_pLinkProp;
      bool SaveChanges();
      wxHyperlinkCtrl* m_pEditedLink;

      int           m_current_icon_Index;
      double        m_lat_save;
      double        m_lon_save;
      wxString      m_IconName_save;
      bool          m_bShowName_save;
      bool          m_bIsVisible_save;
};

#endif // _ROUTEPROP_H_
