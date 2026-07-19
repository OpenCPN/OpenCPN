/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_pi.h"

#include "ocpn_plugin.h"

#include <wx/filename.h>
#include <wx/regex.h>
#include <wx/uri.h>

#include "chartdldr_android_ui.h"

ChartDldrGuiAddSourceDlg::ChartDldrGuiAddSourceDlg(wxWindow *parent)
    : AddSourceDlg(parent) {
  wxFileName fn;
  fn.SetPath(*GetpSharedDataLocation());
  fn.AppendDir(_T("plugins"));
  fn.AppendDir(_T("chartdldr_pi"));
  fn.AppendDir(_T("data"));

  int w = 16;  // default for desktop
  int h = 16;

#ifdef __ANDROID__
  w = 6 * g_androidDPmm;  // mm nominal size
  h = w;

  p_buttonIconList = new wxImageList(w, h);

  fn.SetFullName(_T("button_right.png"));
  wxImage im1(fn.GetFullPath(), wxBITMAP_TYPE_PNG);
  im1.Rescale(w, h, wxIMAGE_QUALITY_HIGH);
  p_buttonIconList->Add(im1);

  fn.SetFullName(_T("button_right.png"));
  wxImage im2(fn.GetFullPath(), wxBITMAP_TYPE_PNG);
  im2.Rescale(w, h, wxIMAGE_QUALITY_HIGH);
  p_buttonIconList->Add(im2);

  fn.SetFullName(_T("button_down.png"));
  wxImage im3(fn.GetFullPath(), wxBITMAP_TYPE_PNG);
  im3.Rescale(w, h, wxIMAGE_QUALITY_HIGH);
  p_buttonIconList->Add(im3);

  fn.SetFullName(_T("button_down.png"));
  wxImage im4(fn.GetFullPath(), wxBITMAP_TYPE_PNG);
  im4.Rescale(w, h, wxIMAGE_QUALITY_HIGH);
  p_buttonIconList->Add(im4);

  m_treeCtrlPredefSrcs->AssignButtonsImageList(p_buttonIconList);
#else
  p_iconList = new wxImageList(w, h);

  fn.SetFullName(_T("folder.png"));
  wxImage ima(fn.GetFullPath(), wxBITMAP_TYPE_PNG);
  ima.Rescale(w, h, wxIMAGE_QUALITY_HIGH);
  p_iconList->Add(ima);

  fn.SetFullName(_T("file.png"));
  wxImage imb(fn.GetFullPath(), wxBITMAP_TYPE_PNG);
  imb.Rescale(w, h, wxIMAGE_QUALITY_HIGH);
  p_iconList->Add(imb);

  m_treeCtrlPredefSrcs->AssignImageList(p_iconList);
#endif /* __ANDROID__ */

  m_treeCtrlPredefSrcs->SetIndent(w);

  m_base_path = wxEmptyString;
  LoadSources();
  m_nbChoice->SetSelection(0);
  // m_treeCtrlPredefSrcs->ExpandAll();

  m_treeCtrlPredefSrcs->Connect(
      wxEVT_COMMAND_TREE_SEL_CHANGED,
      wxTreeEventHandler(ChartDldrGuiAddSourceDlg::OnSourceSelected), NULL,
      this);
  m_sdbSizerBtnsOK->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrGuiAddSourceDlg::OnOkClick), NULL, this);
  m_sdbSizerBtnsCancel->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrGuiAddSourceDlg::OnCancelClick), NULL,
      this);
  m_nbChoice->Connect(
      wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
      wxNotebookEventHandler(ChartDldrGuiAddSourceDlg::OnChangeType), NULL,
      this);

  Fit();

  applyStyle();
}

bool ChartDldrGuiAddSourceDlg::LoadSources() {
  wxTreeItemId tree = m_treeCtrlPredefSrcs->AddRoot(_T("root"));

  wxFileName fn;
  fn.SetPath(*GetpPrivateApplicationDataLocation());
  fn.SetFullName(_T("chartdldr_pi-chart_sources.xml"));
  if (!fn.FileExists()) {
    fn.SetPath(*GetpSharedDataLocation());
    fn.AppendDir(_T("plugins"));
    fn.AppendDir(_T("chartdldr_pi"));
    fn.AppendDir(_T("data"));
    fn.SetFullName(_T("chart_sources.xml"));
    if (!fn.FileExists()) {
      wxLogMessage(wxString::Format(
          _T("Error: chartdldr_pi::LoadSources() %s not found!"),
          fn.GetFullPath().c_str()));
      return false;
    }
  }
  wxString path = fn.GetFullPath();

  pugi::xml_document *doc = new pugi::xml_document;
  bool ret = doc->load_file(path.mb_str());
  if (ret) {
    pugi::xml_node root = doc->first_child();

    for (pugi::xml_node element = root.first_child(); element;
         element = element.next_sibling()) {
      if (!strcmp(element.name(), "sections")) {
        LoadSections(tree, element);
      }
    }
  }
  wxDELETE(doc);
  return ret;
}

bool ChartDldrGuiAddSourceDlg::LoadSections(const wxTreeItemId &root,
                                            pugi::xml_node &node) {
  for (pugi::xml_node element = node.first_child(); element;
       element = element.next_sibling()) {
    if (!strcmp(element.name(), "section")) {
      LoadSection(root, element);
    }
  }
  return true;
}

bool ChartDldrGuiAddSourceDlg::LoadSection(const wxTreeItemId &root,
                                           pugi::xml_node &node) {
  wxString name;
  pugi::xml_node nested_sections;
  pugi::xml_node nested_catalogs;
  for (pugi::xml_node element = node.first_child(); element;
       element = element.next_sibling()) {
    if (!strcmp(element.name(), "name")) {
      name = wxString::FromUTF8(element.first_child().value());
    } else if (!strcmp(element.name(), "sections")) {
      nested_sections = element;
    } else if (!strcmp(element.name(), "catalogs")) {
      nested_catalogs = element;
    }
  }

  if (name.IsEmpty()) {
    return true;
  }

  const wxTreeItemId item = m_treeCtrlPredefSrcs->AppendItem(root, name, 0, 0);
  wxFont *pFont = OCPNGetFont(_("Dialog"));
  if (pFont) {
    m_treeCtrlPredefSrcs->SetItemFont(item, *pFont);
  }
  if (nested_sections) {
    LoadSections(item, nested_sections);
  }
  if (nested_catalogs) {
    LoadCatalogs(item, nested_catalogs);
  }

  return true;
}

bool ChartDldrGuiAddSourceDlg::LoadCatalogs(const wxTreeItemId &root,
                                            pugi::xml_node &node) {
  for (pugi::xml_node element = node.first_child(); element;
       element = element.next_sibling()) {
    if (!strcmp(element.name(), "catalog")) LoadCatalog(root, element);
  }

  return true;
}

bool ChartDldrGuiAddSourceDlg::LoadCatalog(const wxTreeItemId &root,
                                           pugi::xml_node &node) {
  wxString name, type, location, dir;
  for (pugi::xml_node element = node.first_child(); element;
       element = element.next_sibling()) {
    if (!strcmp(element.name(), "name"))
      name = wxString::FromUTF8(element.first_child().value());
    else if (!strcmp(element.name(), "type"))
      type = wxString::FromUTF8(element.first_child().value());
    else if (!strcmp(element.name(), "location"))
      location = wxString::FromUTF8(element.first_child().value());
    else if (!strcmp(element.name(), "dir"))
      dir = wxString::FromUTF8(element.first_child().value());
  }
  ChartSource *cs = new ChartSource(name, location, dir);
  wxTreeItemId id = m_treeCtrlPredefSrcs->AppendItem(root, name, 1, 1, cs);

  wxFont *pFont = OCPNGetFont(_("Dialog"));
  if (pFont) m_treeCtrlPredefSrcs->SetItemFont(id, *pFont);

  return true;
}

ChartDldrGuiAddSourceDlg::~ChartDldrGuiAddSourceDlg() {
  m_treeCtrlPredefSrcs->Disconnect(
      wxEVT_COMMAND_TREE_SEL_CHANGED,
      wxTreeEventHandler(ChartDldrGuiAddSourceDlg::OnSourceSelected), NULL,
      this);
  m_sdbSizerBtnsOK->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrGuiAddSourceDlg::OnOkClick), NULL, this);
  m_sdbSizerBtnsCancel->Disconnect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(ChartDldrGuiAddSourceDlg::OnCancelClick), NULL,
      this);
  m_nbChoice->Disconnect(
      wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,
      wxNotebookEventHandler(ChartDldrGuiAddSourceDlg::OnChangeType), NULL,
      this);
}

wxString ChartDldrGuiAddSourceDlg::FixPath(wxString path) {
  wxString sep(wxFileName::GetPathSeparator());
  wxString s = path;
  s.Replace(_T("/"), sep, true);
  s.Replace(_T(USERDATA), m_base_path);
  s.Replace(sep + sep, sep);
  return s;
}

void ChartDldrGuiAddSourceDlg::OnChangeType(wxNotebookEvent &event) {
  m_treeCtrlPredefSrcs->Enable(m_nbChoice->GetSelection() == 0);
  m_tSourceName->Enable(m_nbChoice->GetSelection() == 1);
  m_tChartSourceUrl->Enable(m_nbChoice->GetSelection() == 1);
  if (event.GetSelection() == 1) {
    m_buttonChartDirectory->Enable();
  } else {
    wxTreeItemId item = m_treeCtrlPredefSrcs->GetSelection();
    ChartSource *cs = nullptr;
    if (item.IsOk()) {
      cs = (ChartSource *)(m_treeCtrlPredefSrcs->GetItemData(item));
    }
    if (!cs) {
      m_buttonChartDirectory->Disable();
    }
  }
}

void ChartDldrGuiAddSourceDlg::ApplySourceFields(const ChartSource &cs) {
  const wxString dir = FixPath(cs.GetDir());
  m_dirExpanded = dir;
  m_tSourceName->SetValue(cs.GetName());
  m_tChartSourceUrl->SetValue(cs.GetUrl());
  m_tcChartDirectory->SetValue(dir);
  m_panelChartDirectory->SetText(dir);
  m_buttonChartDirectory->Enable();
}

void ChartDldrGuiAddSourceDlg::OnSourceSelected(wxTreeEvent &event) {
  wxTreeItemId item = m_treeCtrlPredefSrcs->GetSelection();
  if (!item.IsOk()) {
    event.Skip();
    return;
  }
  ChartSource *cs = (ChartSource *)(m_treeCtrlPredefSrcs->GetItemData(item));
  if (cs) {
    ApplySourceFields(*cs);
  }
  event.Skip();
}

void ChartDldrGuiAddSourceDlg::SetSourceEdit(std::unique_ptr<ChartSource> &cs) {
  m_nbChoice->SetSelection(1);
  m_tChartSourceUrl->Enable();
  m_treeCtrlPredefSrcs->Disable();
  ApplySourceFields(*cs);
}

void ChartDldrGuiAddSourceDlg::OnOkClick(wxCommandEvent &event) {
  wxString msg = wxEmptyString;

  if (m_nbChoice->GetSelection() == 0) {
    wxTreeItemId item = m_treeCtrlPredefSrcs->GetSelection();
    if (m_treeCtrlPredefSrcs->GetSelection().IsOk()) {
      ChartSource *cs =
          (ChartSource *)(m_treeCtrlPredefSrcs->GetItemData(item));
      if (!cs)
        msg +=
            _("You must select one of the predefined chart sources or create "
              "one of your own.\n");
    } else
      msg +=
          _("You must select one of the predefined chart sources or create one "
            "of your own.\n");
  }
  if (m_nbChoice->GetSelection() == 1 &&
      m_tSourceName->GetValue() == wxEmptyString)
    msg += _("The chart source must have a name.\n");
  wxURI url(m_tChartSourceUrl->GetValue());
  if (m_nbChoice->GetSelection() == 1 &&
      (m_tChartSourceUrl->GetValue() == wxEmptyString ||
       !ValidateUrl(m_tChartSourceUrl->GetValue())))
    msg += _("The chart source must have a valid URL.\n");
  if (m_tcChartDirectory->GetValue() == wxEmptyString)
    msg += _("You must select a local folder to store the charts.\n");
  else if (!wxDirExists(m_tcChartDirectory->GetValue()))
    if (!wxFileName::Mkdir(m_tcChartDirectory->GetValue(), 0755,
                           wxPATH_MKDIR_FULL))
      msg += wxString::Format(_("Directory %s can't be created."),
                              m_tcChartDirectory->GetValue().c_str()) +
             _T("\n");

  if (msg != wxEmptyString)
    OCPNMessageBox_PlugIn(this, msg, _("Chart source definition problem"),
                          wxOK | wxCENTRE | wxICON_ERROR);
  else {
    event.Skip();
    SetReturnCode(wxID_OK);
    EndModal(wxID_OK);
  }
}

void ChartDldrGuiAddSourceDlg::OnCancelClick(wxCommandEvent &event) {
  SetReturnCode(wxID_CANCEL);
  EndModal(wxID_CANCEL);
}

bool ChartDldrGuiAddSourceDlg::ValidateUrl(const wxString Url,
                                           bool catalog_xml) {
  wxRegEx re;
  if (catalog_xml)
    re.Compile(
        _T("^https?\\://[a-zA-Z0-9\\./_-]*\\.[xX][mM][lL]$"));  // TODO: wxRegEx
                                                                // sucks a bit,
                                                                // this RE is
                                                                // way too naive
  else
    re.Compile(
        _T("^https?\\://[a-zA-Z0-9\\./_-]*$"));  // TODO: wxRegEx sucks a bit,
                                                 // this RE is way too naive
  return re.Matches(Url);
}
