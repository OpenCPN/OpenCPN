/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_panel_impl.h"
#include "chartdldr_pi.h"

#include "ocpn_plugin.h"

#ifdef __ANDROID__
#include "androidSupport.h"
#endif

#define ID_MNU_SELALL 2001
#define ID_MNU_DELALL 2002
#define ID_MNU_INVSEL 2003
#define ID_MNU_SELUPD 2004
#define ID_MNU_SELNEW 2005

BEGIN_EVENT_TABLE(ChartDldrPanelImpl, ChartDldrPanel)
EVT_PAINT(ChartDldrPanelImpl::OnPaint)
END_EVENT_TABLE()

void ChartDldrPanelImpl::BindPanelEventHandlers() {
  Bind(wxEVT_SIZE, &ChartDldrPanelImpl::OnSize, this);

  m_lbChartSources->Bind(wxEVT_LIST_ITEM_SELECTED,
                         &ChartDldrPanelImpl::SelectSource, this);
  m_lbChartSources->Bind(wxEVT_LEFT_DCLICK, &ChartDldrPanelImpl::OnLeftDClick,
                         this);

  m_bAddSource->Bind(wxEVT_BUTTON, &ChartDldrPanelImpl::AddSource, this);
  m_bDeleteSource->Bind(wxEVT_BUTTON, &ChartDldrPanelImpl::DeleteSource, this);
  m_bEditSource->Bind(wxEVT_BUTTON, &ChartDldrPanelImpl::EditSource, this);
  m_bUpdateChartList->Bind(wxEVT_BUTTON, &ChartDldrPanelImpl::UpdateChartList,
                           this);
  m_bUpdateAllCharts->Bind(wxEVT_BUTTON, &ChartDldrPanelImpl::UpdateAllCharts,
                           this);
  m_bDnldCharts->Bind(wxEVT_BUTTON, &ChartDldrPanelImpl::OnDownloadCharts,
                      this);

#if defined(CHART_LIST)
  m_bSelectNew->Bind(wxEVT_BUTTON, &ChartDldrPanelImpl::OnSelectNewCharts,
                     this);
  m_bSelectUpdated->Bind(wxEVT_BUTTON,
                         &ChartDldrPanelImpl::OnSelectUpdatedCharts, this);
  m_bSelectAll->Bind(wxEVT_BUTTON, &ChartDldrPanelImpl::OnSelectAllCharts,
                     this);
  m_scrollWinChartList->Bind(wxEVT_DATAVIEW_ITEM_VALUE_CHANGED,
                             &ChartDldrPanelImpl::OnSelectChartItem, this);
  m_scrollWinChartList->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU,
                             &ChartDldrPanelImpl::OnChartListContextMenu, this);
#else
  m_scrollWinChartList->Bind(wxEVT_RIGHT_DOWN,
                             &ChartDldrPanelImpl::OnContextMenu, this);
#endif /* CHART_LIST */
}

void ChartDldrPanelImpl::OnPopupClick(wxCommandEvent& evt) {
  switch (evt.GetId()) {
    case ID_MNU_SELALL:
      chart_list_view_.SetAllChecked(true);
      break;
    case ID_MNU_DELALL:
      chart_list_view_.SetAllChecked(false);
      break;
    case ID_MNU_INVSEL:
      chart_list_view_.InvertAllChecked();
      break;
    case ID_MNU_SELUPD:
      chart_list_view_.SetCheckedForKind(ChartDldrChartUpdateKind::Updated,
                                         true);
      break;
    case ID_MNU_SELNEW:
      chart_list_view_.SetCheckedForKind(ChartDldrChartUpdateKind::New, true);
      break;
  }
}

void ChartDldrPanelImpl::OnContextMenu(wxMouseEvent& event) {
  wxMenu menu;

  wxPoint mouseScreen = wxGetMousePosition();
  wxPoint mouseClient = ScreenToClient(mouseScreen);

#ifdef __ANDROID__
  wxFont* pf = OCPNGetFont(_("Menu"));

  wxMenuItem* item1 = new wxMenuItem(&menu, ID_MNU_SELALL, _("Select all"));
  item1->SetFont(*pf);
  menu.Append(item1);

  wxMenuItem* item2 = new wxMenuItem(&menu, ID_MNU_DELALL, _("Deselect all"));
  item2->SetFont(*pf);
  menu.Append(item2);

  wxMenuItem* item3 =
      new wxMenuItem(&menu, ID_MNU_INVSEL, _("Invert selection"));
  item3->SetFont(*pf);
  menu.Append(item3);

  wxMenuItem* item4 = new wxMenuItem(&menu, ID_MNU_SELUPD, _("Select updated"));
  item4->SetFont(*pf);
  menu.Append(item4);

  wxMenuItem* item5 = new wxMenuItem(&menu, ID_MNU_SELNEW, _("Select new"));
  item5->SetFont(*pf);
  menu.Append(item5);

#else

  menu.Append(ID_MNU_SELALL, _("Select all"), wxT(""));
  menu.Append(ID_MNU_DELALL, _("Deselect all"), wxT(""));
  menu.Append(ID_MNU_INVSEL, _("Invert selection"), wxT(""));
  menu.Append(ID_MNU_SELUPD, _("Select updated"), wxT(""));
  menu.Append(ID_MNU_SELNEW, _("Select new"), wxT(""));

#endif

  menu.Connect(wxEVT_COMMAND_MENU_SELECTED,
               (wxObjectEventFunction)&ChartDldrPanelImpl::OnPopupClick, NULL,
               this);
  PopupMenu(&menu, mouseClient.x, mouseClient.y);
}

void ChartDldrPanelImpl::OnPaint(wxPaintEvent& event) {
  if (!m_populated) {
    m_populated = true;
    for (size_t i = 0; i < pPlugIn->m_ChartSources.size(); i++) {
      AppendCatalog(pPlugIn->m_ChartSources.at(i));
    }
  }
#ifdef __WXMAC__
  if (m_macCatalogListNeedsRefresh) {
    m_lbChartSources->Refresh(true);
    m_macCatalogListNeedsRefresh = false;
  }
#endif
  event.Skip();
}
