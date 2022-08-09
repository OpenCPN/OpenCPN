/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OCPN_AUIManager
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
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

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <OCPN_AUIManager.h>
#include "ocpn_plugin.h"
#include "ocpn_frame.h"

#ifdef __WXMSW__
#include "wx/msw/wrapwin.h"
#include "wx/msw/private.h"
#include "wx/msw/dc.h"
#endif

// -- static utility functions --

static wxBitmap wxOPaneCreateStippleBitmap() {
  unsigned char data[] = {0, 0, 0, 192, 192, 192, 192, 192, 192, 0, 0, 0};
  wxImage img(2, 2, data, true);
  return wxBitmap(img);
}

static void ODrawResizeHint(wxDC& dc, const wxRect& rect) {
#if 1
  wxBitmap stipple = wxOPaneCreateStippleBitmap();
  wxBrush brush(stipple);
  dc.SetBrush(brush);
#ifdef __WXMSW__
  wxMSWDCImpl* impl = (wxMSWDCImpl*)dc.GetImpl();
  PatBlt(GetHdcOf(*impl), rect.GetX(), rect.GetY(), rect.GetWidth(),
         rect.GetHeight(), PATINVERT);
#else
  dc.SetPen(*wxTRANSPARENT_PEN);

  dc.SetLogicalFunction(wxXOR);
  dc.DrawRectangle(rect);
#endif
#endif
}

// Convenience function
bool OAuiManager_HasLiveResize(wxAuiManager& manager) {
  // With Core Graphics on Mac, it's not possible to show sash feedback,
  // so we'll always use live update instead.
#if defined(__WXMAC__)
  wxUnusedVar(manager);
  return true;
#else
  return (manager.GetFlags() & wxAUI_MGR_LIVE_RESIZE) == wxAUI_MGR_LIVE_RESIZE;
#endif
}

// OCPN_AUIManager Implementation

BEGIN_EVENT_TABLE(OCPN_AUIManager, wxEvtHandler)
EVT_AUI_PANE_BUTTON(wxAuiManager::OnPaneButton)
EVT_AUI_RENDER(wxAuiManager::OnRender)
EVT_PAINT(OCPN_AUIManager::OnPaint)
EVT_ERASE_BACKGROUND(OCPN_AUIManager::OnEraseBackground)
EVT_SIZE(OCPN_AUIManager::OnSize)
EVT_SET_CURSOR(OCPN_AUIManager::OnSetCursor)
EVT_LEFT_DOWN(OCPN_AUIManager::OnLeftDown)
EVT_LEFT_UP(OCPN_AUIManager::OnLeftUp)
EVT_MOTION(OCPN_AUIManager::OnMotionx)
EVT_LEAVE_WINDOW(OCPN_AUIManager::OnLeaveWindow)
EVT_MOUSE_CAPTURE_LOST(OCPN_AUIManager::OnCaptureLost)
EVT_CHILD_FOCUS(OCPN_AUIManager::OnChildFocus)
EVT_AUI_FIND_MANAGER(OCPN_AUIManager::OnFindManager)
END_EVENT_TABLE()

OCPN_AUIManager::OCPN_AUIManager(wxWindow* managed_wnd, unsigned int flags)
    : wxAuiManager(managed_wnd, flags)

{}

OCPN_AUIManager::~OCPN_AUIManager() {}

void OCPN_AUIManager::OnMotionx(wxMouseEvent& event) {
  // sometimes when Update() is called from inside this method,
  // a spurious mouse move event is generated; this check will make
  // sure that only real mouse moves will get anywhere in this method;
  // this appears to be a bug somewhere, and I don't know where the
  // mouse move event is being generated.  only verified on MSW

  wxPoint mouse_pos = event.GetPosition();
  if (m_lastMouseMove == mouse_pos) return;
  m_lastMouseMove = mouse_pos;

  if (m_action == actionResize) {
    // It's necessary to reset m_actionPart since it destroyed
    // by the Update within DoEndResizeAction.
    if (m_currentDragItem != -1)
      m_actionPart = &(m_uiParts.Item(m_currentDragItem));
    else
      m_currentDragItem = m_uiParts.Index(*m_actionPart);

    if (m_actionPart) {
      wxPoint pos = m_actionPart->rect.GetPosition();
      if (m_actionPart->orientation == wxHORIZONTAL)
        pos.y = wxMax(0, event.m_y - m_actionOffset.y);
      else
        pos.x = wxMax(0, event.m_x - m_actionOffset.x);

      wxSize client_size = m_frame->GetClientSize();
      int used_width = 0, used_height = 0;

      size_t dock_i, dock_count = m_docks.GetCount();
      for (dock_i = 0; dock_i < dock_count; ++dock_i) {
        wxAuiDockInfo& dock = m_docks.Item(dock_i);
        if (dock.dock_direction == wxAUI_DOCK_TOP ||
            dock.dock_direction == wxAUI_DOCK_BOTTOM) {
          used_height += dock.size;
        }
        if (dock.dock_direction == wxAUI_DOCK_LEFT ||
            dock.dock_direction == wxAUI_DOCK_RIGHT) {
          used_width += dock.size;
        }
        //                     if (dock.resizable)
        //                         used_width += sashSize;
      }

      if (OAuiManager_HasLiveResize(*this)) {
        m_frame->ReleaseMouse();
        if ((used_width < client_size.x * 9 / 10) &&
            (used_width > client_size.x * 1 / 10))
          DoEndResizeAction(event);

        m_frame->CaptureMouse();
      } else {
        bool bhasMouse = m_frame->HasCapture();

        if (bhasMouse) m_frame->ReleaseMouse();

        //  Tell MyFrame that the sash is moving, so that he
        //  may disable any top-level windows and so avoid mouse focus problems.
        MyFrame* pmf = wxDynamicCast(m_frame, MyFrame);
        if (pmf) pmf->NotifyChildrenResize();

        wxRect rect(m_frame->ClientToScreen(pos), m_actionPart->rect.GetSize());
        wxScreenDC dc;

        if (!m_0actionHintRect.IsEmpty()) {
          // remove old resize hint
          ODrawResizeHint(dc, m_0actionHintRect);
          m_0actionHintRect = wxRect();
        }

        wxRect frameScreenRect = m_frame->GetScreenRect();

        rect.x =
            wxMax(rect.x, frameScreenRect.x + frameScreenRect.width * 1 / 10);
        rect.x =
            wxMin(rect.x, frameScreenRect.x + frameScreenRect.width * 9 / 10);

        // draw new resize hint, if it's inside the managed frame
        if (1 /*frameScreenRect.Contains(rect)*/) {
          ODrawResizeHint(dc, rect);
          m_0actionHintRect = rect;
        }

        if (bhasMouse) m_frame->CaptureMouse();
      }
    }
  } else {
    OnMotion(event);
  }
}

bool OCPN_AUIManager::DoEndResizeAction(wxMouseEvent& event) {
  // resize the dock or the pane
  if (m_actionPart && m_actionPart->type == wxAuiDockUIPart::typeDockSizer) {
    // first, we must calculate the maximum size the dock may be
    int sashSize = m_art->GetMetric(wxAUI_DOCKART_SASH_SIZE);

    int used_width = 0, used_height = 0;

    wxSize client_size = m_frame->GetClientSize();

    size_t dock_i, dock_count = m_docks.GetCount();
    for (dock_i = 0; dock_i < dock_count; ++dock_i) {
      wxAuiDockInfo& dock = m_docks.Item(dock_i);
      if (dock.dock_direction == wxAUI_DOCK_TOP ||
          dock.dock_direction == wxAUI_DOCK_BOTTOM) {
        used_height += dock.size;
      }
      if (dock.dock_direction == wxAUI_DOCK_LEFT ||
          dock.dock_direction == wxAUI_DOCK_RIGHT) {
        used_width += dock.size;
      }
      if (dock.resizable) used_width += sashSize;
    }

    int available_width = client_size.GetWidth() - used_width;
    int available_height = client_size.GetHeight() - used_height;

#if wxUSE_STATUSBAR
    // if there's a status control, the available
    // height decreases accordingly
    if (wxDynamicCast(m_frame, wxFrame)) {
      wxFrame* frame = static_cast<wxFrame*>(m_frame);
      wxStatusBar* status = frame->GetStatusBar();
      if (status) {
        wxSize status_client_size = status->GetClientSize();
        available_height -= status_client_size.GetHeight();
      }
    }
#endif

    wxRect& rect = m_actionPart->dock->rect;

    wxPoint new_pos(event.m_x - m_actionOffset.x, event.m_y - m_actionOffset.y);
    int new_size, old_size = m_actionPart->dock->size;

    switch (m_actionPart->dock->dock_direction) {
      case wxAUI_DOCK_LEFT:
        new_size = new_pos.x - rect.x;
        if (new_size - old_size > available_width)
          new_size = old_size + available_width;
        m_actionPart->dock->size = new_size;
        break;
      case wxAUI_DOCK_TOP:
        new_size = new_pos.y - rect.y;
        if (new_size - old_size > available_height)
          new_size = old_size + available_height;
        m_actionPart->dock->size = new_size;
        break;
      case wxAUI_DOCK_RIGHT:
        new_size =
            rect.x + rect.width - new_pos.x - m_actionPart->rect.GetWidth();
        if (new_size - old_size > available_width)
          new_size = old_size + available_width;
        m_actionPart->dock->size = new_size;

        m_actionPart->dock->size =
            wxMax(m_actionPart->dock->size, client_size.x * 1 / 10);
        m_actionPart->dock->size =
            wxMin(m_actionPart->dock->size, client_size.x * 9 / 10);

        break;
      case wxAUI_DOCK_BOTTOM:
        new_size =
            rect.y + rect.height - new_pos.y - m_actionPart->rect.GetHeight();
        if (new_size - old_size > available_height)
          new_size = old_size + available_height;
        m_actionPart->dock->size = new_size;
        break;
    }

    Update();
    Repaint(NULL);
  } else if (m_actionPart &&
             m_actionPart->type == wxAuiDockUIPart::typePaneSizer) {
    wxAuiDockInfo& dock = *m_actionPart->dock;
    wxAuiPaneInfo& pane = *m_actionPart->pane;

    int total_proportion = 0;
    int dock_pixels = 0;
    int new_pixsize = 0;

    int caption_size = m_art->GetMetric(wxAUI_DOCKART_CAPTION_SIZE);
    int pane_borderSize = m_art->GetMetric(wxAUI_DOCKART_PANE_BORDER_SIZE);
    int sashSize = m_art->GetMetric(wxAUI_DOCKART_SASH_SIZE);

    wxPoint new_pos(event.m_x - m_actionOffset.x, event.m_y - m_actionOffset.y);

    // determine the pane rectangle by getting the pane part
    wxAuiDockUIPart* pane_part = GetPanePart(pane.window);
    wxASSERT_MSG(pane_part,
                 wxT("Pane border part not found -- shouldn't happen"));

    // determine the new pixel size that the user wants;
    // this will help us recalculate the pane's proportion
    if (dock.IsHorizontal())
      new_pixsize = new_pos.x - pane_part->rect.x;
    else
      new_pixsize = new_pos.y - pane_part->rect.y;

    // determine the size of the dock, based on orientation
    if (dock.IsHorizontal())
      dock_pixels = dock.rect.GetWidth();
    else
      dock_pixels = dock.rect.GetHeight();

    // determine the total proportion of all resizable panes,
    // and the total size of the dock minus the size of all
    // the fixed panes
    int i, dock_pane_count = dock.panes.GetCount();
    int pane_position = -1;
    for (i = 0; i < dock_pane_count; ++i) {
      wxAuiPaneInfo& p = *dock.panes.Item(i);
      if (p.window == pane.window) pane_position = i;

      // while we're at it, subtract the pane sash
      // width from the dock width, because this would
      // skew our proportion calculations
      if (i > 0) dock_pixels -= sashSize;

      // also, the whole size (including decorations) of
      // all fixed panes must also be subtracted, because they
      // are not part of the proportion calculation
      if (p.IsFixed()) {
        if (dock.IsHorizontal())
          dock_pixels -= p.best_size.x;
        else
          dock_pixels -= p.best_size.y;
      } else {
        total_proportion += p.dock_proportion;
      }
    }

    // new size can never be more than the number of dock pixels
    if (new_pixsize > dock_pixels) new_pixsize = dock_pixels;

    // find a pane in our dock to 'steal' space from or to 'give'
    // space to -- this is essentially what is done when a pane is
    // resized; the pane should usually be the first non-fixed pane
    // to the right of the action pane
    int borrow_pane = -1;
    for (i = pane_position + 1; i < dock_pane_count; ++i) {
      wxAuiPaneInfo& p = *dock.panes.Item(i);
      if (!p.IsFixed()) {
        borrow_pane = i;
        break;
      }
    }

    // demand that the pane being resized is found in this dock
    // (this assert really never should be raised)
    wxASSERT_MSG(pane_position != -1, wxT("Pane not found in dock"));

    // prevent division by zero
    if (dock_pixels == 0 || total_proportion == 0 || borrow_pane == -1) {
      m_action = actionNone;
      return false;
    }

    // calculate the new proportion of the pane
    int new_proportion = (new_pixsize * total_proportion) / dock_pixels;

    // default minimum size
    int min_size = 0;

    // check against the pane's minimum size, if specified. please note
    // that this is not enough to ensure that the minimum size will
    // not be violated, because the whole frame might later be shrunk,
    // causing the size of the pane to violate it's minimum size
    if (pane.min_size.IsFullySpecified()) {
      min_size = 0;

      if (pane.HasBorder()) min_size += (pane_borderSize * 2);

      // calculate minimum size with decorations (border,caption)
      if (pane_part->orientation == wxVERTICAL) {
        min_size += pane.min_size.y;
        if (pane.HasCaption()) min_size += caption_size;
      } else {
        min_size += pane.min_size.x;
      }
    }

    // for some reason, an arithmatic error somewhere is causing
    // the proportion calculations to always be off by 1 pixel;
    // for now we will add the 1 pixel on, but we really should
    // determine what's causing this.
    min_size++;

    int min_proportion = (min_size * total_proportion) / dock_pixels;

    if (new_proportion < min_proportion) new_proportion = min_proportion;

    int prop_diff = new_proportion - pane.dock_proportion;

    // borrow the space from our neighbor pane to the
    // right or bottom (depending on orientation);
    // also make sure we don't make the neighbor too small
    int prop_borrow = dock.panes.Item(borrow_pane)->dock_proportion;

    if (prop_borrow - prop_diff < 0) {
      // borrowing from other pane would make it too small,
      // so cancel the resize operation
      prop_borrow = min_proportion;
    } else {
      prop_borrow -= prop_diff;
    }

    dock.panes.Item(borrow_pane)->dock_proportion = prop_borrow;
    pane.dock_proportion = new_proportion;

    // repaint
    Update();
    Repaint(NULL);
  }

  return true;
}

void OCPN_AUIManager::OnLeftUp(wxMouseEvent& event) {
  if (m_action == actionResize) {
    m_frame->ReleaseMouse();

    if (!OAuiManager_HasLiveResize(*this)) {
      // get rid of the hint rectangle
      wxScreenDC dc;
      ODrawResizeHint(dc, m_0actionHintRect);
    }
    if (m_currentDragItem != -1 && OAuiManager_HasLiveResize(*this))
      m_actionPart = &(m_uiParts.Item(m_currentDragItem));

    DoEndResizeAction(event);

    m_currentDragItem = -1;

  } else if (m_action == actionClickButton) {
    m_hoverButton = NULL;
    m_frame->ReleaseMouse();

    if (m_actionPart) {
      UpdateButtonOnScreen(m_actionPart, event);

      // make sure we're still over the item that was originally clicked
      if (m_actionPart == HitTest(event.GetX(), event.GetY())) {
        // fire button-click event
        wxAuiManagerEvent e(wxEVT_AUI_PANE_BUTTON);
        e.SetManager(this);
        e.SetPane(m_actionPart->pane);

#if wxCHECK_VERSION(3, 1, 4)
        e.SetButton(m_actionPart->button);
#else
        e.SetButton(m_actionPart->button->button_id);
#endif
        ProcessMgrEvent(e);
      }
    }
  } else if (m_action == actionClickCaption) {
    m_frame->ReleaseMouse();
  } else if (m_action == actionDragFloatingPane) {
    m_frame->ReleaseMouse();
  }
#if 0
    else if (m_action == actionDragToolbarPane)
    {
        m_frame->ReleaseMouse();

        wxAuiPaneInfo& pane = GetPane(m_actionWindow);
        wxASSERT_MSG(pane.IsOk(), wxT("Pane window not found"));

        // save the new positions
        wxAuiDockInfoPtrArray docks;
        FindDocks(m_docks, pane.dock_direction,
                  pane.dock_layer, pane.dock_row, docks);
        if (docks.GetCount() == 1)
        {
            wxAuiDockInfo& dock = *docks.Item(0);

            wxArrayInt pane_positions, pane_sizes;
            GetPanePositionsAndSizes(dock, pane_positions, pane_sizes);

            int i, dock_pane_count = dock.panes.GetCount();
            for (i = 0; i < dock_pane_count; ++i)
                dock.panes.Item(i)->dock_pos = pane_positions[i];
        }

        pane.state &= ~wxAuiPaneInfo::actionPane;
        Update();
    }
#endif
  else {
    event.Skip();
  }

  m_action = actionNone;
  m_lastMouseMove = wxPoint();  // see comment in OnMotion()
}

// FindDocks() is an internal function that returns a list of docks which meet
// the specified conditions in the parameters and returns a sorted array
// (sorted by layer and then row)
static void OCPNFindDocks(wxAuiDockInfoArray& docks, int dock_direction,
                          int dock_layer, int dock_row,
                          wxAuiDockInfoPtrArray& arr) {
  int begin_layer = dock_layer;
  int end_layer = dock_layer;
  int begin_row = dock_row;
  int end_row = dock_row;
  int dock_count = docks.GetCount();
  int layer, row, i, max_row = 0, max_layer = 0;

  // discover the maximum dock layer and the max row
  for (i = 0; i < dock_count; ++i) {
    max_row = wxMax(max_row, docks.Item(i).dock_row);
    max_layer = wxMax(max_layer, docks.Item(i).dock_layer);
  }

  // if no dock layer was specified, search all dock layers
  if (dock_layer == -1) {
    begin_layer = 0;
    end_layer = max_layer;
  }

  // if no dock row was specified, search all dock row
  if (dock_row == -1) {
    begin_row = 0;
    end_row = max_row;
  }

  arr.Clear();

  for (layer = begin_layer; layer <= end_layer; ++layer)
    for (row = begin_row; row <= end_row; ++row)
      for (i = 0; i < dock_count; ++i) {
        wxAuiDockInfo& d = docks.Item(i);
        if (dock_direction == -1 || dock_direction == d.dock_direction) {
          if (d.dock_layer == layer && d.dock_row == row) arr.Add(&d);
        }
      }
}

wxAuiDockInfo* OCPN_AUIManager::FindDock(wxAuiPaneInfo& pane) {
  wxAuiDockInfoPtrArray arr;
  OCPNFindDocks(m_docks, pane.dock_direction, pane.dock_layer, pane.dock_row,
                arr);
  if (arr.GetCount())
    return arr.Item(0);
  else
    return NULL;
}

void OCPN_AUIManager::SetDockSize(wxAuiDockInfo* dock, int size) {
  dock->size = size;

  Update();
  Repaint(NULL);
}

bool OCPN_AUIManager::ProcessDockResult(wxAuiPaneInfo& target,
                                        const wxAuiPaneInfo& new_pos) {
  // printf("DockResult direction: %d   layer: %d   position: %d %d\n" ,
  // new_pos.dock_direction, new_pos.dock_layer, new_pos.dock_pos,
  // GetCanvasIndexUnderMouse());

  // If we are docking a Dashboard window, we restrict the spots that can accept
  // the docking action
  if (new_pos.window->GetName().IsSameAs(_T("panel"))) {
    // Dashboards can not go on the left( interferes with global toolbar )
    if (/*(new_pos.dock_layer != 1)  ||*/ (new_pos.dock_direction ==
                                           wxAUI_DOCK_LEFT))
      return false;

    // Also, in multi-canvas mode, the dashboard  is restricted to layer 1 in
    // right hand canvas. This forces it to dock at the far right only.
    if (GetCanvasCount() > 1) {
      if (GetCanvasIndexUnderMouse() > 0) {
        if (new_pos.dock_layer == 0) return false;
      }
    }
  }

  return wxAuiManager::ProcessDockResult(target, new_pos);
}
