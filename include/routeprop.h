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
#include <wx/odcombo.h>
#include <wx/gbsizer.h>
#include <wx/spinctrl.h>
#include "LinkPropDlg.h"
#include "Hyperlink.h"
#include <wx/htmllbox.h>
#include <wx/datectrl.h>
#include <wx/timectrl.h>
#include <wx/list.h>

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
class   OCPNIconCombo;

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
#define ID_WPT_RANGERINGS_NO   7507 
#define ID_SHOWWAYPOINTRANGERINGS  7607 
#define ID_PLANSPEEDCTL        7008
#define ID_TEXTCTRL4           7009
#define ID_TEXTCTRLDESC        7010
#define ID_STARTTIMECTL        7011
#define ID_TIMEZONESEL         7012
#define ID_RCLK_MENU_COPY_TEXT 7013
#define ID_RCLK_MENU_EDIT_WP   7014
#define ID_RCLK_MENU_DELETE    7015
#define ID_RCLK_MENU_COPY      7016
#define ID_RCLK_MENU_COPY_LL   7017
#define ID_RCLK_MENU_PASTE     7018
#define ID_RCLK_MENU_PASTE_LL  7019
#define ID_TIMEZONESEL_UTC     7020
#define ID_TIMEZONESEL_LOCAL   7021
#define ID_TIMEZONESEL_LMT     7022
#define ID_RCLK_MENU_DELETE_LINK 7023
#define ID_RCLK_MENU_EDIT_LINK 7024
#define ID_RCLK_MENU_ADD_LINK   7025

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
#define ID_SHOWNAMECHECKBOXBASIC 8006
#define ID_BITMAPCOMBOCTRL 8007
#define ID_NAMECTRL 8008
#define wxID_HTMLLIST 8009
#define ID_DESCR_CTR_DESC 8010
#define ID_DESCR_CTR_BASIC 8011
#define ID_BTN_DESC_BASIC 8012
#define ID_ETA_DATEPICKERCTRL 8013
#define ID_ETA_TIMEPICKERCTRL 8014
#define ID_SHOWNAMECHECKBOX_EXT 8015
#define ID_CHECKBOX_VIS_EXT 8016
#define ID_CHECKBOX_SCAMIN_VIS 8017
#define ID_SET_DEFAULT_ICON 8018
#define ID_SET_DEFAULT_RANGERINGS 8019
#define ID_SET_DEFAULT_ARRIVALRADIUS 8020
#define ID_SET_DEFAULT_SCAMIN 8021
#define ID_SET_DEFAULT_NAMEVIS 8022
#define ID_SET_DEFAULT_ALL 8023
#define ID_BTN_LINK_MENU 8024
#define ID_DEFAULT 8025

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

WX_DECLARE_OBJARRAY(wxBitmap,      ArrayOfBitmaps);

/*!
 * OCPNIconCombo class declaration
 */

class  OCPNIconCombo : public wxOwnerDrawnComboBox
{
public:
    
    OCPNIconCombo(wxWindow* parent, wxWindowID id, const wxString& value = _T(""),
                  const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
                  int n = 0, const wxString choices[] = NULL,
                  long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = _T("OCPNIconCombo") );
    
    
    
    ~OCPNIconCombo ();
    
    void OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const;
    wxCoord OnMeasureItem(size_t item) const;
    wxCoord OnMeasureItemWidth(size_t item) const;
    
    int Append(const wxString& item, wxBitmap bmp);
    void Clear( void );
    
private:
    int         itemHeight;
    ArrayOfBitmaps  bmpArray;
    
};

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
    void CreateControlsCompact();
    
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
    bool m_bcompact;

    enum COLS {LEG, TO_WAYPOINT, DISTANCE, BEARING, BEARING_MAGNETIC,
               LATITUDE, LONGITUDE, ETE_ETD, SPEED, NEXT_TIDE, DESCRIPTION,
               COURSE, COURSE_MAGNETIC, COL_COUNT};
    int cols[COL_COUNT];
    
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
class MarkInfoDlg : public wxDialog
{
    DECLARE_EVENT_TABLE()
    
	private:
        RoutePoint  *m_pRoutePoint;
        static bool instanceFlag;
        int           i_htmlList_item;
        LinkPropImpl* m_pLinkProp;
        
        bool            m_bShowName_save;
        wxString        m_Name_save;
        wxString        m_IconName_save;
        int             m_current_icon_Index;
        double          m_lat_save;
        double          m_lon_save;
        wxString        m_Description_save;
        HyperlinkList*  m_pMyLinkList;
        bool            m_bIsVisible_save;
        bool            m_bUseScaMin_save;
        int             m_iScaminVal_save;
        bool            m_bShowWaypointRangeRings_save;
        int             m_iWaypointRangeRingsNumber_save;
        float           m_fWaypointRangeRingsStep_save;
        wxColour        m_wxcWaypointRangeRingsColour_save;
        double          m_WaypointArrivalRadius_save;
        float           m_PlannedSpeed_save;
        wxDateTime      m_ArrETA_save;
        
        
        
	protected:
        OCPNIconCombo*          m_bcomboBoxIcon;
        wxBoxSizer*             bSizerBasicProperties;
        wxBoxSizer*             bSizerLinks;
        wxButton*               m_buttonExtDescription;
        wxButton*               m_buttonLinksMenu;
        wxButton*               DefaultsBtn;
        wxCheckBox*             m_checkBoxScaMin;
        wxCheckBox*             m_checkBoxShowName;
        wxCheckBox*             m_checkBoxShowNameExt;
        wxCheckBox*             m_checkBoxVisible;
        wxChoice*               m_choiceWaypointRangeRingsUnits;
        wxColourPickerCtrl*     m_PickColor;
        wxDatePickerCtrl*       m_EtaDatePickerCtrl;
        wxFlexGridSizer*        fg_MainSizer;
        wxFlexGridSizer*        fSizerBasicProperties;
        wxFlexGridSizer*        waypointradarGrid;
        wxFlexGridSizer*        waypointrrSelect;
        wxGridBagSizer*         bGB_SizerProperties;
        wxGridBagSizer*         gbSizerInnerProperties;
        wxNotebook*             m_notebookProperties;
        wxObject*               m_contextObject;
        wxPanel*                m_PanelBasicProperties;
        wxPanel*                m_PanelDescription;
        wxPanel*                m_PanelExtendedProperties;
        wxSimpleHtmlListBox*    m_htmlList;
        wxSize                  m_defaultClientSize;
        wxSpinCtrl*             m_SpinWaypointRangeRingsNumber;
        wxStaticBitmap*         m_bitmapIcon;
        wxStaticBoxSizer*       sbS_Description;
        wxStaticBoxSizer*       sbSizerExtProperties;
        wxStaticBoxSizer*       sbSizerLinks;
        wxStaticBoxSizer*       sbSizerBasicProperties; 
        wxStaticBoxSizer*       sbRangeRingsExtProperties;
        wxStaticBoxSizer*       sbSizerDescription;
        wxStaticText*           m_staticTextArrivalRadius;                                   
        wxStaticText*           m_staticTextDescription;                                     
        wxStaticText*           m_staticTextEditEnabled;                                     
        wxStaticText*           m_staticTextGpx;                                             
        wxStaticText*           m_staticTextGuid;
        wxStaticText*           m_staticTextIcon;
        wxStaticText*           m_staticTextLatitude;
        wxStaticText*           m_staticTextLayer;
        wxStaticText*           m_staticTextLongitude;
        wxStaticText*           m_staticTextName;
        wxStaticText*           m_staticTextScaMin;
        wxStaticText*           m_staticTextShowNameExt;
        wxStaticText*           m_staticTextRR1;
        wxStaticText*           m_staticTextRR2;
        wxStaticText*           m_staticTextRR3;
        wxStaticText*           m_staticTextRR4;
        wxStaticText*           m_staticTextArrivalUnits;
        wxStaticText*           m_staticTextPlSpeed;
        wxStaticText*           m_staticTextEta;
        wxStaticText*           m_staticTextPlSpeedUnits;
        wxStdDialogButtonSizer* m_sdbSizerButtons;
        wxTextCtrl*             m_textArrivalRadius;
        wxTextCtrl*             m_textCtrlExtDescription;
        wxTextCtrl*             m_textCtrlGpx;
        wxTextCtrl*             m_textCtrlGuid;
        wxTextCtrl*             m_textDescription;
        wxTextCtrl*             m_textLatitude;
        wxTextCtrl*             m_textLongitude;
        wxTextCtrl*             m_textName;
        wxTextCtrl*             m_textScaMin;
        wxTextCtrl*             m_textWaypointRangeRingsStep;
        wxTextCtrl*             m_textCtrlPlSpeed;
        wxTimePickerCtrl*       m_EtaTimePickerCtrl;
        wxBitmap*               _img_MUI_settings_svg;
        
        void initialize_images(void);
        void OnBitmapCombClick(wxCommandEvent& event);
        void OnPositionCtlUpdated( wxCommandEvent& event );
        void OnExtDescriptionClick( wxCommandEvent& event );
        void OnDescChangedExt( wxCommandEvent& event );
        void OnDescChangedBasic( wxCommandEvent& event );
        void OnMarkInfoCancelClick( wxCommandEvent& event );
        void OnMarkInfoOKClick( wxCommandEvent& event );
        void OnShowWaypointNameSelectBasic( wxCommandEvent& event );
        void OnShowWaypointNameSelectExt( wxCommandEvent& event );
        void OnSelectScaMinExt( wxCommandEvent& event );
        void OnWptRangeRingsNoChange( wxSpinEvent& event );
        void OnCopyPasteLatLon( wxCommandEvent& event );
        void OnWaypointRangeRingSelect( wxCommandEvent& event );
        void m_htmlListContextMenuBtn( wxCommandEvent &event );
        void m_htmlListContextMenu( wxMouseEvent &event );
        void OnRightClickLatLon( wxCommandEvent& event );
        void OnHtmlLinkClicked(wxHtmlLinkEvent &event);
        void On_html_link_popupmenu_Click( wxCommandEvent& event );
        void DefautlBtnClicked( wxCommandEvent& event );
        
    public:
        MarkInfoDlg( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Waypoint Properties"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( -1, -1 ), long style = wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxRESIZE_BORDER );
        ~MarkInfoDlg();
        void Create();
        void InitialFocus(void);
        void RecalculateSize( void );
        RoutePoint *GetRoutePoint(void) { return m_pRoutePoint; }
        void SetColorScheme( ColorScheme cs );
        void SetRoutePoint( RoutePoint *pRP );
        void UpdateHtmlList();
        void SetDialogTitle(const wxString & title) { SetTitle(title); }
        bool UpdateProperties( bool positionOnly = false );
        void ValidateMark(void);
        bool SaveChanges();
        wxSimpleHtmlListBox *GetSimpleBox()
            { return wxDynamicCast(m_htmlList, wxSimpleHtmlListBox); }
        void OnHtmlCellClicked(wxHtmlCellEvent &event);
};

///////////////////////////////////////////////////////////////////////////////
/// Class SaveDefaultsDialog
///////////////////////////////////////////////////////////////////////////////

class SaveDefaultsDialog: public wxDialog
{
    public:

        SaveDefaultsDialog(wxWindow* parent,wxWindowID id = -1);
        virtual ~SaveDefaultsDialog();

        //(*Handlers(SaveDefaultsDialog)
        void OnQuit(wxCommandEvent& event);

        //(*Declarations(SaveDefaultsDialog)
        wxCheckBox* RangRingsCB;
        wxCheckBox* ArrivalRCB;
        wxCheckBox* IconCB;
        wxCheckBox* NameCB;
        wxStaticText* StaticText1;
        wxCheckBox* ScaleCB;
        wxStaticText* StaticText2;
        //*)

        DECLARE_EVENT_TABLE()
};


#endif // _ROUTEPROP_H_
