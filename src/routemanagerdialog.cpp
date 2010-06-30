/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2010, Anders Lund <anders@alweb.dk>
 */

#include "routemanagerdialog.h"

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/listctrl.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

#include <iostream>

#include "navutil.h"
#include "routeprop.h"
#include "routeman.h"
#include "georef.h"
#include "chartbase.h"
//#include "chartimg.h"               // for ChartBaseBSB

#define DIALOG_MARGIN 3

/* XPM */
static const char *eye[]={
"20 20 7 1",
". c none",
"# c #000000",
"a c #333333",
"b c #666666",
"c c #999999",
"d c #cccccc",
"e c #ffffff",
"....................",
"....................",
"....................",
"....................",
".......######.......",
".....#aabccb#a#.....",
"....#deeeddeebcb#...",
"..#aeeeec##aceaec#..",
".#bedaeee####dbcec#.",
"#aeedbdabc###bcceea#",
".#bedad######abcec#.",
"..#be#d######dadb#..",
"...#abac####abba#...",
".....##acbaca##.....",
".......######.......",
"....................",
"....................",
"....................",
"....................",
"...................."};

enum { rmVISIBLE = 0, rmROUTENAME, rmROUTEDESC };// RMColumns;

// GOLBALS :0
extern RouteList *pRouteList;
extern RouteProp *pRoutePropDialog;
extern Routeman  *g_pRouteMan;
extern MyConfig  *pConfig;
extern ChartCanvas *cc1;
extern ChartBase *Current_Ch;
extern Track     *g_pActiveTrack;

// sort callback. Sort by route name.
int wxCALLBACK SortRoutes(long item1, long item2, long list)
{
      wxListCtrl *lc = (wxListCtrl*)list;

      wxListItem it1, it2;
      it1.SetId(lc->FindItem(-1, item1));
      it1.SetColumn(1);
      it1.SetMask(it1.GetMask() | wxLIST_MASK_TEXT);

      it2.SetId(lc->FindItem(-1, item2));
      it2.SetColumn(1);
      it2.SetMask(it2.GetMask() | wxLIST_MASK_TEXT);

      lc->GetItem(it1);
      lc->GetItem(it2);

//      wxString s1 = it1.GetText();
//      wxString s2 = it2.GetText();
//      int res = s1.Cmp(s2);
      return it1.GetText().Cmp(it2.GetText());
}


// event table. Empty, because I find it much easier to see what is connected to what
// using Connect() where possible, so that it is visible in the code.
BEGIN_EVENT_TABLE(RouteManagerDialog, wxDialog)
END_EVENT_TABLE()

// implementation
RouteManagerDialog::RouteManagerDialog(wxWindow *parent)
      : wxDialog(parent, -1, wxString(_("Route Manager")), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
      Create();
}



void RouteManagerDialog::Create()
{
      // Setup GUI
      wxBoxSizer *layout = new wxBoxSizer(wxVERTICAL);
      SetSizer(layout);

      wxBoxSizer *sbsRoutes = new wxBoxSizer(wxHORIZONTAL);
      layout->Add(sbsRoutes, 1, wxEXPAND|wxALL, DIALOG_MARGIN);

      m_pRouteListCtrl = new wxListCtrl(this, -1, wxDefaultPosition, wxSize(400, -1),
          wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_SORT_ASCENDING|wxLC_HRULES/*|wxLC_VRULES*/|wxBORDER_SUNKEN);
      m_pRouteListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler(RouteManagerDialog::OnRouteSelected), NULL, this);
      m_pRouteListCtrl->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(RouteManagerDialog::OnRouteListLeftDown), NULL, this);
      m_pRouteListCtrl->Connect(wxEVT_LEFT_UP, wxMouseEventHandler(RouteManagerDialog::OnRouteListClickUp), NULL, this);
      sbsRoutes->Add(m_pRouteListCtrl, 1, wxEXPAND|wxALL, DIALOG_MARGIN);

      // Columns: visibility ctrl, name
      // note that under MSW for SetColumnWidth() to work we need to create the
      // items with images initially even if we specify dummy image id

    wxListItem itemCol;
    itemCol.SetImage(-1);
    m_pRouteListCtrl->InsertColumn(0, itemCol);
 //     m_pRouteListCtrl->InsertColumn(rmVISIBLE, wxT(""));
      m_pRouteListCtrl->SetColumnWidth(0, 28);

      m_pRouteListCtrl->InsertColumn(rmROUTENAME, _("Route Name"));
      m_pRouteListCtrl->InsertColumn(rmROUTEDESC, _("To"));

      // Buttons: Delete, Properties..., Zoom to
      wxBoxSizer *bsRouteButtons = new wxBoxSizer(wxVERTICAL);
      sbsRoutes->Add(bsRouteButtons, 0, wxALIGN_RIGHT);

      btnActivate = new wxButton(this, -1, _("&Activate"));
      bsRouteButtons->Add(btnActivate, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnActivate->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnActivateClick), NULL, this);
      btnActivate->Connect(wxEVT_LEFT_DOWN,
                           wxMouseEventHandler(RouteManagerDialog::OnBtnLeftDown), NULL, this);

      btnZoomto = new wxButton(this, -1, _("&Zoom to"));
      bsRouteButtons->Add(btnZoomto, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnZoomto->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnZoomtoClick), NULL, this);
      btnZoomto->Connect(wxEVT_LEFT_DOWN,
                           wxMouseEventHandler(RouteManagerDialog::OnBtnLeftDown), NULL, this);

      btnReverse = new wxButton(this, -1, _("&Reverse"));
      bsRouteButtons->Add(btnReverse, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnReverse->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                          wxCommandEventHandler(RouteManagerDialog::OnReverseClick), NULL, this);

      btnProperties = new wxButton(this, -1, _("&Properties..."));
      bsRouteButtons->Add(btnProperties, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnProperties->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                             wxCommandEventHandler(RouteManagerDialog::OnPropertiesClick), NULL, this);

      btnDelete = new wxButton(this, -1, _("&Delete"));
      bsRouteButtons->Add(btnDelete, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnDelete->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnDeleteClick), NULL, this);

      btnDeleteAllRoutes = new wxButton(this, -1, _("&Delete All Routes..."));
      bsRouteButtons->Add(btnDeleteAllRoutes, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnDeleteAllRoutes->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnDeleteAllRoutesClick), NULL, this);

      btnDeleteAllTracks = new wxButton(this, -1, _("&Delete All Tracks..."));
      bsRouteButtons->Add(btnDeleteAllTracks, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnDeleteAllTracks->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnDeleteAllTracksClick), NULL, this);

      btnImport = new wxButton(this, -1, _("I&mport..."));
      bsRouteButtons->Add(btnImport, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnImport->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnImportClick), NULL, this);

      btnExport = new wxButton(this, -1, _("&Export..."));
      bsRouteButtons->Add(btnExport, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnExport->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnExportClick), NULL, this);

      // Dialog buttons
      wxSizer *szButtons = CreateButtonSizer(wxOK);
      layout->Add(szButtons, 0, wxALL|wxALIGN_RIGHT, DIALOG_MARGIN);

      Fit();

      SetMinSize(GetBestSize());

      // create a image list for the list with just the eye icon
      wxImageList *imglist = new wxImageList(20, 20, true, 1);
      imglist->Add(wxBitmap(eye));
      m_pRouteListCtrl->AssignImageList(imglist, wxIMAGE_LIST_SMALL);

      UpdateRouteListCtrl();

      // timer for mouseclick detection
      m_Timer = new wxTimer(this);
      Connect(wxEVT_TIMER, wxTimerEventHandler(RouteManagerDialog::OnTimeOut));

      m_bNeedConfigFlush = false;
}

RouteManagerDialog::~RouteManagerDialog()
{
      // Do this just once!!
      if (m_bNeedConfigFlush)
            pConfig->UpdateSettings();
}

void RouteManagerDialog::UpdateRouteListCtrl()
{
      // if an item was selected, make it selected again if it still exist
      long selected_index = m_pRouteListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      long selected_id = -1;
      if (selected_index > -1)
            selected_id = m_pRouteListCtrl->GetItemData(selected_index);

      // Delete existing items
      m_pRouteListCtrl->DeleteAllItems();

      // then add routes to the listctrl
      RouteList::iterator it;
      wxListItem item;
      int index = 0;
      for (it = (*pRouteList).begin(); it != (*pRouteList).end(); ++it, ++index)
      {
            item.SetId(index);
            item.SetImage((*it)->IsVisible() ? 0 : -1);
            item.SetData(index);
            wxString name = (*it)->m_RouteNameString;
            if (name.IsEmpty())
            {
                  if((*it)->m_bIsTrack)
                  {
                        if(g_pActiveTrack == (Track *)(*it))
                              name = _("(Active Track)");
                        else
                              name = _("(Unnamed Track)");
                  }
                  else
                        name = _("(Unnamed Route)");
            }
//            item.SetImage(-1);
//            item.SetColumn(0);
            //item.SetText(name);
            long idx = m_pRouteListCtrl->InsertItem(item);
            // m_pRouteListCtrl->SetItemData(index,index);
            // m_pRouteListCtrl->SetItemImage(index, (*it)->IsVisible() ? 0 : -1);
            m_pRouteListCtrl->SetItem(idx, rmROUTENAME, name);

            wxString startend = (*it)->m_RouteStartString;
           if (!(*it)->m_RouteEndString.IsEmpty())
                  startend.append(_(" - ") + (*it)->m_RouteEndString);

//           item.SetImage(-1);
//            item.SetColumn(2);
//            item.SetText(startend);

           m_pRouteListCtrl->SetItem(idx, rmROUTEDESC,  startend);
      }

      m_pRouteListCtrl->SetColumnWidth(1, wxLIST_AUTOSIZE);
      m_pRouteListCtrl->SetColumnWidth(2, wxLIST_AUTOSIZE);

      if(m_pRouteListCtrl->GetColumnWidth(1) < 20)
            m_pRouteListCtrl->SetColumnWidth(1, 50);

      if(m_pRouteListCtrl->GetColumnWidth(2) < 20)
            m_pRouteListCtrl->SetColumnWidth(2, 50);

      m_pRouteListCtrl->SortItems(SortRoutes, (long)m_pRouteListCtrl);

      // restore selection if possible
      // NOTE this will select a different item, if one is deleted
      // (the next route will get that index).
      if (selected_id > -1)
      {
            selected_index = m_pRouteListCtrl->FindItem(-1, selected_id);
            m_pRouteListCtrl->SetItemState(selected_index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
      }

      UpdateButtons();
}

void RouteManagerDialog::UpdateButtons()
{
      // enable/disable buttons
      long selected_index_index = m_pRouteListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      bool enable = !(selected_index_index < 0);

      btnDelete->Enable(enable);
      btnZoomto->Enable(enable); // && !cc1->m_bFollow);
      btnProperties->Enable(enable);
      btnReverse->Enable(enable);
      btnExport->Enable(enable);

      // set activate button text
      Route *route = NULL;
      if(enable)
            route = pRouteList->Item(m_pRouteListCtrl->GetItemData(selected_index_index))->GetData();

      if(!g_pRouteMan->IsAnyRouteActive())
      {
            btnActivate->Enable(enable);
            if (enable)
                   btnActivate->SetLabel(_("Activate"));

      }
      else
      {
            if(enable)
            {
                 if(route && route->m_bRtIsActive)
                  {
                        btnActivate->Enable(enable);
                        btnActivate->SetLabel(_("Deactivate"));
                  }
                  else
                        btnActivate->Enable(false);
            }
            else
                  btnActivate->Enable(false);
      }

      //    Final test, disallow activate for tracks....
      if(enable)
      {
            if(route)
            {
                  if(route->m_bIsTrack)
                        btnActivate->Enable(false);
            }
      }

}

void RouteManagerDialog::MakeAllRoutesInvisible()
{
      RouteList::iterator it;
      long index = 0;
      for (it = (*pRouteList).begin(); it != (*pRouteList).end(); ++it, ++index)
      {
            if ((*it)->IsVisible()) { // avoid config updating as much as possible!
                  (*it)->SetVisible(false);
                  m_pRouteListCtrl->SetItemImage(m_pRouteListCtrl->FindItem(-1, index), -1); // Likely not same order :0
                  pConfig->UpdateRoute(*it); // auch, flushes config to disk. FIXME
            }
      }
}

void RouteManagerDialog::ZoomtoRoute(Route *route)
{

      // Calculate bbox center
      double clat = route->RBBox.GetMinY() + (route->RBBox.GetHeight()/2);
      double clon = route->RBBox.GetMinX() + (route->RBBox.GetWidth()/2);

      // Calculate ppm
      double rw, rh, ppm; // route width, height, final ppm scale to use
      int ww, wh; // chart window width, height
      // route bbox width in nm
      DistanceBearingMercator(route->RBBox.GetMinY(), route->RBBox.GetMinX(),
                              route->RBBox.GetMinY(), route->RBBox.GetMaxX(),
                              NULL, &rw);
      // route bbox height in nm
      DistanceBearingMercator(route->RBBox.GetMinY(), route->RBBox.GetMinX(),
                              route->RBBox.GetMaxY(), route->RBBox.GetMinX(),
                              NULL, &rh);

      cc1->GetSize(&ww, &wh);

      ppm = wxMin(ww/(rw*1852), wh/(rh*1852))*(100-abs(clat))/90;

      ppm = wxMin(ppm, 1.0);

      cc1->ClearbFollow();
      cc1->SetViewPoint(clat, clon, ppm, 0, cc1->GetVPRotation(), CURRENT_RENDER);
      cc1->Refresh();

      m_bNeedConfigFlush = true;
}

//BEGIN Event handlers
void RouteManagerDialog::OnDeleteClick(wxCommandEvent &event)
{
      // Delete selected_index route/track
      long selected_index = m_pRouteListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (selected_index < 0) return;

      Route *proute_to_delete = pRouteList->Item(m_pRouteListCtrl->GetItemData(selected_index))->GetData();
      if(proute_to_delete->m_bIsTrack)
            g_pRouteMan->DeleteTrack((Track *)proute_to_delete);
      else
            g_pRouteMan->DeleteRoute(proute_to_delete);

      pConfig->DeleteConfigRoute ( proute_to_delete );

      UpdateRouteListCtrl();
      cc1->Refresh();

      m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnDeleteAllRoutesClick(wxCommandEvent &event)
{
      wxMessageDialog mdlg(this, _("Are you sure you want to delete <ALL> routes?"), wxString(_("OpenCPN Alert")),wxYES_NO  );
      if(mdlg.ShowModal() == wxID_YES)
      {
            if ( g_pRouteMan->GetpActiveRoute() )
                  g_pRouteMan->DeactivateRoute();

            g_pRouteMan->DeleteAllRoutes();
      }

      UpdateRouteListCtrl();

      if(pRoutePropDialog)
            pRoutePropDialog->Hide();

      cc1->Refresh();

      m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnDeleteAllTracksClick(wxCommandEvent &event)
{
      wxMessageDialog mdlg(this, _("Are you sure you want to delete <ALL> tracks?"), wxString(_("OpenCPN Alert")),wxYES_NO  );
      if(mdlg.ShowModal() == wxID_YES)
      {
            g_pRouteMan->DeleteAllTracks();
      }

      UpdateRouteListCtrl();

      if(pRoutePropDialog)
            pRoutePropDialog->Hide();

      cc1->Refresh();

      m_bNeedConfigFlush = true;
}


void RouteManagerDialog::OnPropertiesClick(wxCommandEvent &event)
{
      // Show routeproperties dialog for selected route
      long selected_index = m_pRouteListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (selected_index < 0) return;


      Route *route = pRouteList->Item(m_pRouteListCtrl->GetItemData(selected_index))->GetData();
      if (route) {
            if ( NULL == pRoutePropDialog )          // There is one global instance of the RouteProp Dialog
                  pRoutePropDialog  = new RouteProp(GetParent());

            pRoutePropDialog->SetRouteAndUpdate(route);
            pRoutePropDialog->UpdateProperties();

            if(!pRoutePropDialog->IsShown())
                  pRoutePropDialog->ShowModal();
            // route might have changed
            UpdateRouteListCtrl();

            m_bNeedConfigFlush = true;
      }
}

void RouteManagerDialog::OnZoomtoClick(wxCommandEvent &event)
{
//      if (cc1->m_bFollow)
//            return;

      // Zoom into the bounding box of the selected route
      long selected_index = m_pRouteListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (selected_index < 0) return;

      // optionally make this route exclusively visible
      if (m_bCtrlDown)
            MakeAllRoutesInvisible();

      Route *route = pRouteList->Item(m_pRouteListCtrl->GetItemData(selected_index))->GetData();

      // Ensure route is visible
      if (!route->IsVisible())
      {
            route->SetVisible(true);
            m_pRouteListCtrl->SetItemImage(selected_index, route->IsVisible() ? 0 : -1);
            pConfig->UpdateRoute(route);
      }

      ZoomtoRoute(route);
}

void RouteManagerDialog::OnReverseClick(wxCommandEvent &event)
{
      // Reverse selected route
      long selected_index = m_pRouteListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (selected_index < 0) return;

      wxMessageDialog ask(this, _("Waypoints can be renamed to reflect the new order, the names will be '001', '002' etc.\n\nDo you want to rename the waypoints?"),
                          _("Rename Waypoints?"), wxYES_NO);
      bool rename = (ask.ShowModal() == wxID_YES);

      Route *route = pRouteList->Item(m_pRouteListCtrl->GetItemData(selected_index))->GetData();
      route->Reverse(rename);

      // update column 2 - create a UpdateRouteItem(index) instead?
      wxString startend = route->m_RouteStartString;
      if (!route->m_RouteEndString.IsEmpty())
            startend.append(_(" - ") + route->m_RouteEndString);
      m_pRouteListCtrl->SetItem(selected_index, 2,  startend);

      pConfig->UpdateRoute(route);
//       pConfig->UpdateSettings(); // NOTE done once in destructor
      cc1->Refresh();

      m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnImportClick(wxCommandEvent &event)
{
      // Import routes
      // FIXME there is no way to instruct this function about what to import.
      // Suggest to add that!
      pConfig->ImportGPX(this);

      UpdateRouteListCtrl();
}

void RouteManagerDialog::OnExportClick(wxCommandEvent &event)
{
      // Export selected route
      long selected_index = m_pRouteListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (selected_index < 0) return;

      pConfig->ExportGPXRoute(this,
                              pRouteList->Item(m_pRouteListCtrl->GetItemData(selected_index))->GetData());
}

void RouteManagerDialog::OnActivateClick(wxCommandEvent &event)
{
      // Activate the selected route, unless it already is
      long selected_index = m_pRouteListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (selected_index < 0) return;

      if (m_bCtrlDown)
            MakeAllRoutesInvisible();

      Route *route = pRouteList->Item(m_pRouteListCtrl->GetItemData(selected_index))->GetData();

      if (!route->m_bRtIsActive)
      {
            if (!route->IsVisible())
            {
                  route->SetVisible(true);
                  m_pRouteListCtrl->SetItemImage(selected_index, 0, 0);
            }

            ZoomtoRoute(route);
            g_pRouteMan->ActivateRoute(route);
      }
      else
            g_pRouteMan->DeactivateRoute();

      pConfig->UpdateRoute(route);
//       pConfig->UpdateSettings(); // NOTE is this really nessecary??
      cc1->Refresh();

      btnActivate->SetLabel(route->m_bRtIsActive ? _("Deactivate") : _("Activate"));

      m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnRouteListLeftDown(wxMouseEvent &event)
{
      m_bPossibleClick = true;
      m_Timer->Start(250, true);

      wxPoint pos = event.GetPosition();
      int flags = 0;
      long clicked_index = m_pRouteListCtrl->HitTest(pos, flags);

      //    Clicking Visibility column?
      if (clicked_index > -1 && event.GetX() < m_pRouteListCtrl->GetColumnWidth(0))
      {
            // Process the clicked item
            Route *route = pRouteList->Item(m_pRouteListCtrl->GetItemData(clicked_index))->GetData();
            route->SetVisible(!route->IsVisible());
            m_pRouteListCtrl->SetItemImage(clicked_index, route->IsVisible() ? 0 : -1);

            pConfig->UpdateRoute(route);
            cc1->Refresh();
      }

      // Allow wx to process...
      event.Skip();
}

void RouteManagerDialog::OnTimeOut(wxTimerEvent &event)
{
      m_bPossibleClick = false;
}

void RouteManagerDialog::OnRouteListClickUp(wxMouseEvent &event)
{
      return;
      if (!m_bPossibleClick)
           return;

      m_Timer->Stop();

      wxPoint pos = event.GetPosition();
      int flags = 0;
      long clicked_index = m_pRouteListCtrl->HitTest(pos, flags);

      if (clicked_index > -1 && event.GetX() < m_pRouteListCtrl->GetColumnWidth(0))
      {
            // If CTRL is pressed, hide all other routes and make this visible.
            // I do that by just hiding all routes, before processing the one clicked.
            if (event.ControlDown())
                  MakeAllRoutesInvisible();

            // Process the clicked item
            Route *route = pRouteList->Item(m_pRouteListCtrl->GetItemData(clicked_index))->GetData();
            route->SetVisible(!route->IsVisible());
            m_pRouteListCtrl->SetItemImage(clicked_index, route->IsVisible() ? 0 : -1);

            // FIXME add/remove route segments/waypoints from selectable items, so there are no
            // hidden selectables! This should probably be done outside this class!
            // The problem is that the current waypoint class does not provide good support
            // for this, there is a "visible" property, but no means for proper management.
            // Jan. 28 2010: Ideas:
            // - Calculate on the fly how many visible routes use a waypoint.
            //   This requires a semidouble loop (routes, waypoints in visible routes). It could
            //   be done by the function getting the selection. Potentially somewhat slow?
            // - OR keep a property in waypoints telling that
            //   (A number, increased/decreased for each waypoint by Route::SetVisible()).
            //   Immediate result when detecting the selectable object, small overhead in
            //   Route::SetVisible(). I prefer this.
            // - We also need to know if the waypoint should otherwise be visible,
            //   ie it is a "normal" waypoint used in the route (then it should be visible
            //   in all cases). Is this possible with current code?
            // - Get rid of the Select objects, they do no good! They should be replaced with a function
            //   in the application, the search would reqire equal amount of looping, but less
            //   dereferencing pointers, and it would remove the overhead of keeping and maintaining
            //   the extra pointer lists.

            pConfig->UpdateRoute(route);
//             pConfig->UpdateSettings(); // NOTE nessecary?? - no, just once in destructor!
            cc1->Refresh();
      }


      UpdateButtons();
}

void RouteManagerDialog::OnBtnLeftDown(wxMouseEvent &event)
{
      m_bCtrlDown = event.ControlDown();
      event.Skip();
}

void RouteManagerDialog::OnRouteSelected(wxListEvent &event)
{
    long clicked_index = event.m_itemIndex;
    // Process the clicked item
    Route *route = pRouteList->Item(m_pRouteListCtrl->GetItemData(clicked_index))->GetData();
//    route->SetVisible(!route->IsVisible());
    m_pRouteListCtrl->SetItemImage(clicked_index, route->IsVisible() ? 0 : -1);
    pConfig->UpdateRoute(route);
    cc1->Refresh();

    UpdateButtons();

}

//END Event handlers


// kate: indent-width 6; space-indent on; indent-mode cstyle;
