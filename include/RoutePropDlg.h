///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include <wx/timectrl.h>
#include <wx/choice.h>
#include <wx/wrapsizer.h>
#include <wx/dataview.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/hyperlink.h>
#include <wx/button.h>
#include <wx/tglbtn.h>
#include <wx/scrolwin.h>
#include <wx/statbox.h>
#include <wx/notebook.h>
#include <wx/frame.h>

#ifdef __WXGTK__
// wxTimePickerCtrl is completely broken in Gnome based desktop environments as
// of wxGTK 3.0
#include "time_textbox.h"
#endif

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class RoutePropDlg
///////////////////////////////////////////////////////////////////////////////
class RoutePropDlg : public wxFrame {
private:
protected:
  wxNotebook* m_ntbRteProp;
  wxPanel* m_pnlBasic;
  wxStaticText* m_stName;
  wxTextCtrl* m_tcName;
  wxStaticText* m_stFrom;
  wxTextCtrl* m_tcFrom;
  wxStaticText* m_stTo;
  wxTextCtrl* m_tcTo;
  wxStaticText* m_stDistTotal;
  wxTextCtrl* m_tcDistance;
  wxStaticText* m_stPlanSpeed;
  wxTextCtrl* m_tcPlanSpeed;
  wxStaticText* m_stEnroute;
  wxTextCtrl* m_tcEnroute;
  wxStaticText* m_stDeparture;
  wxDatePickerCtrl* m_dpDepartureDate;
#ifdef __WXGTK__
  TimeCtrl* m_tpDepartureTime;
#else
  wxTimePickerCtrl* m_tpDepartureTime;
#endif
  wxStaticText* m_stTimeZone;
  wxChoice* m_choiceTimezone;
  wxStaticText* m_staticTextColor;
  wxChoice* m_choiceColor;
  wxStaticText* m_stStyle;
  wxChoice* m_choiceStyle;
  wxStaticText* m_stWidth;
  wxChoice* m_choiceWidth;
  wxDataViewListCtrl* m_dvlcWaypoints;
  wxDataViewColumn* m_dataViewListColumnLeg;
  wxDataViewColumn* m_dataViewListColumnToWpt;
  wxDataViewColumn* m_dataViewListColumnDistance;
  wxDataViewColumn* m_dataViewListColumnBearing;
  wxDataViewColumn* m_dataViewListColumnTotalDistance;
  wxDataViewColumn* m_dataViewListColumnLat;
  wxDataViewColumn* m_dataViewListColumnLon;
  wxDataViewColumn* m_dataViewListColumnETE;
  wxDataViewColumn* m_dataViewListColumnETA;
  wxDataViewColumn* m_dataViewListColumnSpeed;
  wxDataViewColumn* m_dataViewListColumnNTE;
  wxDataViewColumn* m_dataViewListColumnDesc;
  wxDataViewColumn* m_dataViewListColumnCourse;
  wxDataViewColumn* m_dataViewListColumnETD;
  wxDataViewColumn* m_dataViewListColumnEmpty;
  wxPanel* m_pnlAdvanced;
  wxStaticText* m_stDescription;
  wxTextCtrl* m_tcDescription;
  wxScrolledWindow* m_scrolledWindowLinks;
  wxBoxSizer* bSizerLinks;
  wxHyperlinkCtrl* m_hyperlink1;
  wxMenu* m_menuLink;
  wxButton* m_btnAddLink;
  wxToggleButton* m_toggleBtnEdit;
  wxStaticText* m_stEditEnabled;
  wxMenu* m_menuLinks;
  wxButton* m_btnPrint;
  wxButton* m_btnExtend;
  wxButton* m_btnSplit;
  wxStdDialogButtonSizer* m_sdbSizerBtns;
  wxButton* m_sdbSizerBtnsOK;
  wxButton* m_sdbSizerBtnsCancel;

  // Virtual event handlers, overide them in your derived class
  virtual void RoutePropDlgOnClose(wxCloseEvent& event) { event.Skip(); }
  virtual void RoutePropDlgOnSize(wxSizeEvent& event) { event.Skip(); }
  virtual void RoutePropDlgOnNotebookPageChanged(wxNotebookEvent& event) {
    event.Skip();
  }
  virtual void PlanSpeedOnKillFocus(wxFocusEvent& event) { event.Skip(); }
  virtual void PlanSpeedOnTextEnter(wxCommandEvent& event) { event.Skip(); }
  virtual void DepartureDateOnDateChanged(wxDateEvent& event) { event.Skip(); }
  virtual void DepartureTimeOnTimeChanged(wxDateEvent& event) { event.Skip(); }
  virtual void TimezoneOnChoice(wxCommandEvent& event) { event.Skip(); }
  virtual void WaypointsOnDataViewListCtrlItemContextMenu(
      wxDataViewEvent& event) {
    event.Skip();
  }
  virtual void WaypointsOnDataViewListCtrlItemEditingDone(
      wxDataViewEvent& event) {
    event.Skip();
  }
  virtual void WaypointsOnDataViewListCtrlItemValueChanged(
      wxDataViewEvent& event) {
    event.Skip();
  }
  virtual void WaypointsOnDataViewListCtrlSelectionChanged(
      wxDataViewEvent& event) {
    event.Skip();
  }
  virtual void ItemEditOnMenuSelection(wxCommandEvent& event) { event.Skip(); }
  virtual void ItemAddOnMenuSelection(wxCommandEvent& event) { event.Skip(); }
  virtual void ItemDeleteOnMenuSelection(wxCommandEvent& event) {
    event.Skip();
  }
  virtual void AddLinkOnButtonClick(wxCommandEvent& event) { event.Skip(); }
  virtual void BtnEditOnToggleButton(wxCommandEvent& event) { event.Skip(); }
  virtual void PrintOnButtonClick(wxCommandEvent& event) { event.Skip(); }
  virtual void ExtendOnButtonClick(wxCommandEvent& event) { event.Skip(); }
  virtual void SplitOnButtonClick(wxCommandEvent& event) { event.Skip(); }
  virtual void BtnsOnCancelButtonClick(wxCommandEvent& event) { event.Skip(); }
  virtual void BtnsOnOKButtonClick(wxCommandEvent& event) { event.Skip(); }

public:
  RoutePropDlg(wxWindow* parent, wxWindowID id = wxID_ANY,
               const wxString& title = _("Route Properties"),
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxSize(550, 450),
               long style = wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT |
                            wxTAB_TRAVERSAL);

  ~RoutePropDlg();

  void m_hyperlink1OnContextMenu(wxMouseEvent& event) {
    m_hyperlink1->PopupMenu(m_menuLink, event.GetPosition());
  }

  void m_scrolledWindowLinksOnContextMenu(wxMouseEvent& event) {
    m_scrolledWindowLinks->PopupMenu(m_menuLinks, event.GetPosition());
  }

  void OnKeyChar(wxKeyEvent& event);

  DECLARE_EVENT_TABLE()
};
