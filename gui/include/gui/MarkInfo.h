/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Mark Properties Support
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

#ifndef _MARKINFO_H_
#define _MARKINFO_H_

/*!
 * Includes
 */
#include <wx/listctrl.h>
#include "ocpn_frame.h"    //FIXME (dave ) // for ColorScheme
#include <wx/hyperlink.h>  // toh, 2009.02.08
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
#include "model/hyperlink.h"
#include <wx/htmllbox.h>
#include <wx/datectrl.h>
#include <wx/timectrl.h>
#include <wx/dateevt.h>
#include <wx/list.h>
#include <wx/combobox.h>

#include <wx/dialog.h>
#include "dialog_cntrl.h"

#ifdef __WXGTK__
// wxTimePickerCtrl is completely broken in Gnome based desktop environments as
// of wxGTK 3.0
#include "time_textbox.h"
#endif

#ifdef __WXOSX__
#define DIALOG_PARENT wxFrame
#else
#define DIALOG_PARENT wxDialog
#endif

#define ID_WPT_RANGERINGS_NO 7507
#define ID_RCLK_MENU_COPY_TEXT 7013
#define ID_RCLK_MENU_DELETE 7015
#define ID_RCLK_MENU_COPY 7016
#define ID_RCLK_MENU_COPY_LL 7017
#define ID_RCLK_MENU_PASTE 7018
#define ID_RCLK_MENU_PASTE_LL 7019
#define ID_RCLK_MENU_DELETE_LINK 7023
#define ID_RCLK_MENU_EDIT_LINK 7024
#define ID_RCLK_MENU_ADD_LINK 7025

#include "tcmgr.h"
#include "OCPNPlatform.h"

/*!
 * Forward declarations
 */

class wxListCtrl;
class Route;
class RoutePoint;
class HyperlinkList;
class OCPNIconCombo;

/*!
 * Control identifiers
 */

////@begin control identifiers

#define ID_MARKPROP 8000
#define SYMBOL_MARKPROP_STYLE \
  wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX
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
#define ID_BTN_SHOW_TIDES 8026

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

WX_DECLARE_OBJARRAY(wxBitmap, ArrayOfBitmaps);

class SaveDefaultsDialog;

/**
 * Custom combobox for selecting waypoint icons. Extends wxOwnerDrawnComboBox to
 * provide a combobox with custom-drawn items, specifically for selecting
 * waypoint icons.
 */
class OCPNIconCombo : public wxOwnerDrawnComboBox {
public:
  OCPNIconCombo(wxWindow* parent, wxWindowID id, const wxString& value = _T(""),
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize, int n = 0,
                const wxString choices[] = NULL, long style = 0,
                const wxValidator& validator = wxDefaultValidator,
                const wxString& name = _T("OCPNIconCombo"));

  ~OCPNIconCombo();

  void OnDrawItem(wxDC& dc, const wxRect& rect, int item, int flags) const;
  wxCoord OnMeasureItem(size_t item) const;
  wxCoord OnMeasureItemWidth(size_t item) const;

  int Append(const wxString& item, wxBitmap bmp);
  void Clear(void);

private:
  int itemHeight;
  ArrayOfBitmaps bmpArray;
};

//    LatLonTextCtrl Specification
//    We need a derived wxText control for lat/lon input in the MarkProp dialog
//    Specifically, we need to catch loss-of-focus events and signal the parent
//    dialog to update the mark's lat/lon dynamically.

// DECLARE_EVENT_TYPE(EVT_LLCHANGE, -1)
extern /*expdecl*/ const wxEventType EVT_LLCHANGE;

class LatLonTextCtrl : public wxTextCtrl {
  DECLARE_EVENT_TABLE()

public:
  LatLonTextCtrl(wxWindow* parent, wxWindowID id,
                 const wxString& value = _T(""),
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize, long style = 0,
                 const wxValidator& validator = wxDefaultValidator,
                 const wxString& name = wxTextCtrlNameStr);

  void OnKillFocus(wxFocusEvent& event);

  wxEvtHandler* m_pParentEventHandler;
};

/**
 * Dialog for displaying and editing waypoint properties.
 * Provides a comprehensive interface for viewing and modifying
 * waypoint properties in OpenCPN. Includes functionality for
 * editing basic and extended properties, managing links, and setting defaults.
 */
class MarkInfoDlg : public DIALOG_PARENT {
  DECLARE_EVENT_TABLE()
  friend class SaveDefaultsDialog;

private:
  RoutePoint* m_pRoutePoint;
  static bool instanceFlag;
  int i_htmlList_item;

  bool m_bShowName_save;
  wxString m_Name_save;
  wxString m_IconName_save;
  int m_current_icon_Index;
  double m_lat_save;
  double m_lon_save;
  wxString m_Description_save;
  HyperlinkList* m_pMyLinkList;
  bool m_bIsVisible_save;
  bool m_bUseScaMin_save;
  int m_iScaminVal_save;
  bool m_bShowWaypointRangeRings_save;
  int m_iWaypointRangeRingsNumber_save;
  float m_fWaypointRangeRingsStep_save;
  wxColour m_wxcWaypointRangeRingsColour_save;
  double m_WaypointArrivalRadius_save;
  float m_PlannedSpeed_save;
  wxDateTime m_ArrETA_save;
  std::map<double, const IDX_entry*> m_tss;
  wxString m_lasttspos;

protected:
  OCPNIconCombo* m_bcomboBoxIcon;
  wxBoxSizer* bSizerBasicProperties;
  wxBoxSizer* bSizerLinks;
  wxButton* m_buttonExtDescription;
  wxButton* m_buttonLinksMenu;
  wxBitmapButton* m_buttonShowTides;
  wxButton* DefaultsBtn;
  wxCheckBox* m_checkBoxScaMin;
  wxCheckBox* m_checkBoxShowName;
  wxCheckBox* m_checkBoxShowNameExt;
  wxCheckBox* m_checkBoxVisible;
  wxChoice* m_choiceWaypointRangeRingsUnits;
  wxColourPickerCtrl* m_PickColor;
  wxCheckBox* m_cbEtaPresent;
  wxBoxSizer* bMainSizer;
  wxFlexGridSizer* fSizerBasicProperties;
  wxFlexGridSizer* waypointradarGrid;
  wxFlexGridSizer* waypointrrSelect;
  wxGridBagSizer* bGB_SizerProperties;
  wxFlexGridSizer* gbSizerInnerProperties;
  wxNotebook* m_notebookProperties;
  wxObject* m_contextObject;
  wxScrolledWindow* m_panelBasicProperties;
  wxPanel* m_panelDescription;
  wxScrolledWindow* m_panelExtendedProperties;
  wxSimpleHtmlListBox* m_htmlList;
  wxSize m_defaultClientSize;
  wxChoice* m_ChoiceWaypointRangeRingsNumber;
  wxStaticBitmap* m_bitmapIcon;
  wxStaticBoxSizer* sbS_Description;
  wxStaticBoxSizer* sbSizerExtProperties;
  wxStaticBoxSizer* sbSizerLinks;
  wxStaticBoxSizer* sbSizerBasicProperties;
  wxStaticBoxSizer* sbRangeRingsExtProperties;
  wxStaticBoxSizer* sbSizerDescription;
  wxStaticText* m_staticTextArrivalRadius;
  wxStaticText* m_staticTextDescription;
  wxStaticText* m_staticTextEditEnabled;
  wxStaticText* m_staticTextGpx;
  wxStaticText* m_staticTextGuid;
  wxStaticText* m_staticTextTideStation;
  wxStaticText* m_staticTextIcon;
  wxStaticText* m_staticTextLatitude;
  wxStaticText* m_staticTextLayer;
  wxStaticText* m_staticTextLongitude;
  wxStaticText* m_staticTextName;
  wxStaticText* m_staticTextScaMin;
  wxStaticText* m_staticTextShowNameExt;
  wxStaticText* m_staticTextRR1;
  wxStaticText* m_staticTextRR2;
  wxChoice* m_RangeRingUnits;
  wxStaticText* m_staticTextRR4;
  wxStaticText* m_staticTextArrivalUnits;
  wxStaticText* m_staticTextPlSpeed;
  wxStaticText* m_staticTextEta;
  wxStaticText* m_staticTextPlSpeedUnits;
  wxStdDialogButtonSizer* m_sdbSizerButtons;
  wxTextCtrl* m_textArrivalRadius;
  wxTextCtrl* m_textCtrlExtDescription;
  wxTextCtrl* m_textCtrlGpx;
  wxTextCtrl* m_textCtrlGuid;
  wxScrolledWindow* m_scrolledWindowLinks;
  wxHyperlinkCtrl* m_hyperlink17;
  wxMenu* m_menuLink;
  wxToggleButton* m_toggleBtnEdit;
  wxButton* m_buttonAddLink;

#ifdef __OCPN__ANDROID__
  wxChoice* m_comboBoxTideStation;
#else
  wxComboBox* m_comboBoxTideStation;
#endif
  wxTextCtrl* m_textDescription;
  wxTextCtrl* m_textLatitude;
  wxTextCtrl* m_textLongitude;
  TextField* m_textName;
  wxTextCtrl* m_textScaMin;
  wxTextCtrl* m_textWaypointRangeRingsStep;
  wxTextCtrl* m_textCtrlPlSpeed;
  wxBitmap _img_MUI_settings_svg;
  wxButton* m_sdbSizerButtonsCancel;
  wxButton* m_sdbSizerButtonsOK;

  wxDatePickerCtrl* m_EtaDatePickerCtrl;
#ifdef __WXGTK__
  TimeCtrl* m_EtaTimePickerCtrl;
#else
  wxTimePickerCtrl* m_EtaTimePickerCtrl;
#endif
  wxArrayString m_choiceTideChoices;
  wxBitmap m_bmTide;
  int m_sizeMetric;
  wxHyperlinkCtrl* m_pEditedLink;

  void initialize_images(void);
  void OnBitmapCombClick(wxCommandEvent& event);
  void OnPositionCtlUpdated(wxCommandEvent& event);
  void OnFocusEvent(wxFocusEvent& event);
  void OnExtDescriptionClick(wxCommandEvent& event);
  void OnDescChangedExt(wxCommandEvent& event);
  void OnDescChangedBasic(wxCommandEvent& event);
  void OnMarkInfoCancelClick(wxCommandEvent& event);
  void OnMarkInfoOKClick(wxCommandEvent& event);
  void OnShowWaypointNameSelectBasic(wxCommandEvent& event);
  void OnShowWaypointNameSelectExt(wxCommandEvent& event);
  void OnSelectScaMinExt(wxCommandEvent& event);
  void OnWptRangeRingsNoChange(wxCommandEvent& event);
  void OnCopyPasteLatLon(wxCommandEvent& event);
  void OnWaypointRangeRingSelect(wxCommandEvent& event);
  void m_htmlListContextMenuBtn(wxCommandEvent& event);
  void m_htmlListContextMenu(wxMouseEvent& event);
  void OnRightClickLatLon(wxCommandEvent& event);
  void OnHtmlLinkClicked(wxHtmlLinkEvent& event);
  void OnHyperLinkClick(wxHyperlinkEvent& event);

  void On_html_link_popupmenu_Click(wxCommandEvent& event);
  void DefautlBtnClicked(wxCommandEvent& event);
  void OnNotebookPageChanged(wxNotebookEvent& event);
  void OnTimeChanged(wxDateEvent& event) { m_cbEtaPresent->SetValue(true); }
  void OnTideStationCombobox(wxCommandEvent& event);
  void OnClose(wxCloseEvent& event);
  void ShowTidesBtnClicked(wxCommandEvent& event);
  void OnAddLink(wxCommandEvent& event);

public:
  MarkInfoDlg(wxWindow* parent, wxWindowID id = wxID_ANY,
              const wxString& title = _("Waypoint Properties"),
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxSize(-1, -1),
              long style = FRAME_WITH_LINKS_STYLE);
  ~MarkInfoDlg();
  void Create();
  void InitialFocus(void);
  void RecalculateSize(void);
  RoutePoint* GetRoutePoint(void) { return m_pRoutePoint; }
  void SetColorScheme(ColorScheme cs);
  void SetRoutePoint(RoutePoint* pRP);
  void ClearData();
  void SetBulkEdit(bool bBulkEdit);
  void UpdateHtmlList();
  void SetDialogTitle(const wxString& title) { SetTitle(title); }
  bool UpdateProperties(bool positionOnly = false);
  void ValidateMark(void);
  bool SaveChanges();
  void OnActivate(wxActivateEvent& event);

  void SetNameValidator(wxTextValidator* pValidator);

  wxSimpleHtmlListBox* GetSimpleBox() {
    return dynamic_cast<wxSimpleHtmlListBox*>(m_htmlList);
  }
  void OnHtmlCellClicked(wxHtmlCellEvent& event);

  SaveDefaultsDialog* m_SaveDefaultDlg;
};

/**
 * Dialog for saving default waypoint properties. Allows users to select
 * which properties of the current waypoint should be saved as defaults for
 * future waypoints.
 */
class SaveDefaultsDialog : public wxDialog {
  friend class MarkInfoDlg;

protected:
  //(*Declarations(SaveDefaultsDialog)
  wxCheckBox* RangRingsCB;
  wxCheckBox* ArrivalRCB;
  wxCheckBox* IconCB;
  wxCheckBox* NameCB;
  wxCheckBox* ScaleCB;
  wxStaticText* stRR;
  wxStaticText* stArrivalR;
  wxStaticText* stIcon;
  wxStaticText* stName;
  wxStaticText* stScale;
  wxStaticText* StaticText1;
  //*)

public:
  SaveDefaultsDialog(MarkInfoDlg* parent);
};

#endif  // _MARKINFO_H_
