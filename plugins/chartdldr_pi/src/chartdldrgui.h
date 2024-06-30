///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Jun  6 2014)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#ifndef __CHARTDLDRGUI_H__
#define __CHARTDLDRGUI_H__

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/treectrl.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/notebook.h>
#include <wx/statbox.h>
#include <wx/filepicker.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/statline.h>
#include <wx/dcmemory.h>
#include "ocpn_plugin.h"

#if defined(CHART_LIST)
#include <wx/dataview.h>
#endif /* CHART_LIST */
///////////////////////////////////////////////////////////////////////////

#if defined(CHART_LIST)  // Are we building using wxDataViewListCtrl?
// We don't use ArrayOfChartPanels when using wxDataViewListCtrl
#else
class ChartPanel;
class ChartDldrPanelImpl;
#endif /* CHART_LIST */

class DLDR_OCPNChartDirPanel : public wxPanel {
public:
  DLDR_OCPNChartDirPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                         const wxSize& size);
  ~DLDR_OCPNChartDirPanel();

  void OnPaint(wxPaintEvent& event);
  int GetUnselectedHeight() { return m_unselectedHeight; }
  int GetRefHeight() { return m_refHeight; }
  void SetText(wxString text);

private:
  // bool m_bSelected;
  wxColour m_boxColour;
  int m_unselectedHeight;
  wxString m_ChartDir;
  int m_refHeight;

  DECLARE_EVENT_TABLE()
};

///////////////////////////////////////////////////////////////////////////////
/// Class AddSourceDlg
///////////////////////////////////////////////////////////////////////////////
class AddSourceDlg : public wxDialog {
private:
protected:
  wxPanel* m_panelPredefined;
  wxPanel* m_panelCustom;
  wxStaticText* m_stName;
  wxStaticText* m_stUrl;
  wxStdDialogButtonSizer* m_sdbSizerBtns;
  wxButton* m_sdbSizerBtnsOK;
  wxButton* m_sdbSizerBtnsCancel;

  // Virtual event handlers, overide them in your derived class
  virtual void OnSourceSelected(wxTreeEvent& event) { event.Skip(); }
  virtual void OnOkClick(wxCommandEvent& event) { event.Skip(); }
  virtual void OnCancelClick(wxCommandEvent& event) { event.Skip(); }
  void OnDirSelClick(wxCommandEvent& event);
  void applyStyle();

public:
  wxNotebook* m_nbChoice;
  wxTreeCtrl* m_treeCtrlPredefSrcs;
  wxTextCtrl* m_tSourceName;
  wxTextCtrl* m_tChartSourceUrl;
  // wxDirPickerCtrl* m_dpChartDirectory;
  wxTextCtrl* m_tcChartDirectory;
  DLDR_OCPNChartDirPanel* m_panelChartDirectory;

  wxButton* m_buttonChartDirectory;
  wxScrolledWindow* m_sourceswin;
  wxString m_dirExpanded;

  AddSourceDlg(wxWindow* parent, wxWindowID id = wxID_ANY,
               const wxString& title = _("New chart source"),
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize,
               long style = wxDEFAULT_DIALOG_STYLE);
  ~AddSourceDlg();
  void OnNbPage(wxNotebookEvent& event);
};

///////////////////////////////////////////////////////////////////////////////
/// Class ChartDldrPanel
///////////////////////////////////////////////////////////////////////////////
class ChartDldrPanel : public wxPanel {
private:
protected:
  wxListCtrl* m_lbChartSources;
  wxButton* m_bAddSource;
  wxButton* m_bDeleteSource;
  wxButton* m_bEditSource;
  wxButton* m_bUpdateChartList;
  wxButton* m_bUpdateAllCharts;
  wxStaticText* m_stCatalogInfo;
  // wxButton* m_bHelp;
  wxButton* m_bDnldCharts;
  // wxButton* m_bShowLocal;
#if defined(CHART_LIST)
  // Buttons for selecting charts to download
  wxButton* m_bClear;
  wxButton* m_bSelectNew;
  wxButton* m_bSelectUpdated;
  wxButton* m_bSelectAll;
#endif /* CHART_LIST */
  wxNotebook* m_DLoadNB;
  wxString m_csTitle;
  wxStaticText* m_chartsLabel;
#if !defined(CHART_LIST)  // The chart list viewer does not use m_panelArray
  std::vector<std::unique_ptr<ChartPanel>> m_panelArray;
#endif /* CHART_LIST */
  wxBoxSizer* m_boxSizerCharts;

  // Virtual event handlers, overide them in your derived class
  virtual void OnPaint(wxPaintEvent& event) { event.Skip(); }
  virtual void OnLeftDClick(wxMouseEvent& event) { event.Skip(); }
  virtual void SelectSource(wxListEvent& event) { event.Skip(); }
  virtual void AddSource(wxCommandEvent& event) { event.Skip(); }
  virtual void DeleteSource(wxCommandEvent& event) { event.Skip(); }
  virtual void EditSource(wxCommandEvent& event) { event.Skip(); }
  virtual void UpdateChartList(wxCommandEvent& event) { event.Skip(); }
  virtual void UpdateAllCharts(wxCommandEvent& event) { event.Skip(); }
  virtual void DoHelp(wxCommandEvent& event) { event.Skip(); }
  virtual void OnDownloadCharts(wxCommandEvent& event) { event.Skip(); }
  virtual void OnShowLocalDir(wxCommandEvent& event) { event.Skip(); }
  virtual void OnSize(wxSizeEvent& event);
#ifdef HAVE_WX_GESTURE_EVENTS
  void OnLongPress(wxLongPressEvent &event);
#endif

#if defined(CHART_LIST)
  virtual void OnSelectChartItem(wxCommandEvent& event) { event.Skip(); }
  virtual void OnSelectNewCharts(wxCommandEvent& event) { event.Skip(); }
  virtual void OnSelectUpdatedCharts(wxCommandEvent& event) { event.Skip(); }
  virtual void OnSelectAllCharts(wxCommandEvent& event) { event.Skip(); }
#endif /* CHART_LIST */
public:
#if defined(CHART_LIST)
  wxDataViewListCtrl* m_scrollWinChartList;
  virtual wxDataViewListCtrl* getChartList() { return m_scrollWinChartList; }
  virtual bool isNew(int item) {
    return (m_scrollWinChartList->GetTextValue(item, 1) == _("New"));
  }
  virtual bool isUpdated(int item) {
    return (m_scrollWinChartList->GetTextValue(item, 1) == _("Out of date"));
  }
  virtual void clearChartList() { m_scrollWinChartList->DeleteAllItems(); }
#else
  wxScrolledWindow* m_scrollWinChartList;
#endif /* CHART_LIST */

  virtual void SetChartInfo(const wxString& info) {
    m_stCatalogInfo->SetLabel(info);
    m_stCatalogInfo->Show(true);
  }
  ChartDldrPanel(wxWindow* parent, wxWindowID id = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxSize(-1, -1),
                 long style = wxTAB_TRAVERSAL);
  ~ChartDldrPanel();
  // ChartDldrPanel() { }

  virtual void OnContextMenu(wxMouseEvent& event) { event.Skip(); }
};

///////////////////////////////////////////////////////////////////////////////
/// Class ChartDldrPrefsDlg
///////////////////////////////////////////////////////////////////////////////
class ChartDldrPrefsDlg : public wxDialog {
private:
protected:
  // wxDirPickerCtrl* m_dpDefaultDir;
  wxStaticText* m_stPreselect;
  wxCheckBox* m_cbSelectUpdated;
  wxCheckBox* m_cbSelectNew;
  wxStaticLine* m_staticline1;
  wxCheckBox* m_cbBulkUpdate;
  wxStdDialogButtonSizer* m_sdbSizerBtns;
  wxButton* m_sdbSizerBtnsOK;
  wxButton* m_sdbSizerBtnsCancel;
  wxButton* m_buttonChartDirectory;
  wxButton* m_buttonDownloadMasterCatalog;
  wxTextCtrl* m_tcDefaultDir;
  DLDR_OCPNChartDirPanel* m_panelChartDirectory;

  void OnDirSelClick(wxCommandEvent& event);
  void OnDownloadMasterCatalog(wxCommandEvent& event);

  // Virtual event handlers, overide them in your derived class
  virtual void OnCancelClick(wxCommandEvent& event);
  virtual void OnOkClick(wxCommandEvent& event);

public:
  ChartDldrPrefsDlg(wxWindow* parent, wxWindowID id = wxID_ANY,
                    const wxString& title = _("Chart Downloader Preferences"),
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxSize(462, 331),
                    long style = wxDEFAULT_DIALOG_STYLE);
  ~ChartDldrPrefsDlg();
};

// We only use this object type in "old-style" chart listing
#if !defined(CHART_LIST)
class ChartPanel : public wxPanel {
public:
  ChartPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos,
             const wxSize& size, wxString Name, wxString stat, wxString latest,
             ChartDldrPanel* DldrPanel, bool bcheck);
  ~ChartPanel();

  void OnContextMenu(wxMouseEvent& event);
  wxCheckBox* GetCB() { return m_cb; }
  bool isNew() { return (m_stat == _("New")); }
  bool isUpdated() { return (m_stat == _("Out of date")); }
  void OnLeftUp(wxMouseEvent &event);
#ifdef HAVE_WX_GESTURE_EVENTS
    void OnLongPress(wxLongPressEvent &event);
#endif

private:
  wxCheckBox* m_cb;
  //    wxStaticText *m_chartInfo;
  //    wxStaticText *m_chartInfo2;
  wxString m_stat;
  wxString m_latest;
  ChartDldrPanel* m_dldrPanel;
  bool m_popupWanted;
};
#endif /* CHART_PANEL */

#endif  //__CHARTDLDRGUI_H__
