/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Framework for Undo features
 * Author:   Jesper Weissglas
 *
 ***************************************************************************
 *   Copyright (C) 2012 by David S. Register                               *
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
 ***************************************************************************
 *
 *
 */

#include "config.h"

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/file.h>
#include <wx/datetime.h>
#include <wx/clipbrd.h>

#include "model/route.h"
#include "model/routeman.h"
#include "model/select.h"

#include "chcanv.h"
#include "MarkInfo.h"
#include "navutil.h"
#include "ocpn_frame.h"
#include "routemanagerdialog.h"
#include "styles.h"
#include "undo.h"

extern Routeman* g_pRouteMan;
extern MyConfig* pConfig;
extern MyFrame* gFrame;
extern RouteManagerDialog* pRouteManagerDialog;
extern MarkInfoDlg* g_pMarkInfoDialog;

Undo::Undo(ChartCanvas* parent)
    : m_parent(parent),
      m_is_inside_undoable_action(false),
      m_stackpointer(0),
      m_depth_setting(10) {}

Undo::~Undo() { undoStack.clear(); }

wxString UndoAction::Description() const {
  wxString descr;
  switch (type) {
    case Undo_CreateWaypoint:
      descr = _("Create Mark");
      break;
    case Undo_DeleteWaypoint:
      descr = _("Delete Mark");
      break;
    case Undo_MoveWaypoint:
      descr = _("Move Waypoint");
      break;
    case Undo_AppendWaypoint:
      descr = _("Append Waypoint");
      break;
    default:
      descr = _T("");
      break;
  }
  return descr;
}

void doUndoMoveWaypoint(UndoAction* action, ChartCanvas*) {
  double lat, lon;
  RoutePoint* currentPoint = (RoutePoint*)action->after[0];
  wxRealPoint* lastPoint = (wxRealPoint*)action->before[0];
  lat = currentPoint->m_lat;
  lon = currentPoint->m_lon;
  currentPoint->m_lat = lastPoint->y;
  currentPoint->m_lon = lastPoint->x;
  lastPoint->y = lat;
  lastPoint->x = lon;
  SelectItem* selectable = (SelectItem*)action->selectable[0];
  selectable->m_slat = currentPoint->m_lat;
  selectable->m_slon = currentPoint->m_lon;

  if ((NULL != g_pMarkInfoDialog) && (g_pMarkInfoDialog->IsShown())) {
    if (currentPoint == g_pMarkInfoDialog->GetRoutePoint())
      g_pMarkInfoDialog->UpdateProperties(true);
  }

  wxArrayPtrVoid* routeArray =
      g_pRouteMan->GetRouteArrayContaining(currentPoint);
  if (routeArray) {
    for (unsigned int ir = 0; ir < routeArray->GetCount(); ir++) {
      Route* pr = (Route*)routeArray->Item(ir);
      pr->FinalizeForRendering();
      pr->UpdateSegmentDistances();
      pConfig->UpdateRoute(pr);
    }
    delete routeArray;
  }
}

void doUndoDeleteWaypoint(UndoAction* action, ChartCanvas*) {
  RoutePoint* point = (RoutePoint*)action->before[0];
  pSelect->AddSelectableRoutePoint(point->m_lat, point->m_lon, point);
  pConfig->AddNewWayPoint(point, -1);
  if (NULL != pWayPointMan) pWayPointMan->AddRoutePoint(point);
  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateWptListCtrl();
}

void doRedoDeleteWaypoint(UndoAction* action, ChartCanvas*) {
  RoutePoint* point = (RoutePoint*)action->before[0];
  pConfig->DeleteWayPoint(point);
  pSelect->DeleteSelectablePoint(point, SELTYPE_ROUTEPOINT);
  if (NULL != pWayPointMan) pWayPointMan->RemoveRoutePoint(point);
  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateWptListCtrl();
}

void doUndoAppendWaypoint(UndoAction* action, ChartCanvas* cc) {
  RoutePoint* point = (RoutePoint*)action->before[0];
  Route* route = (Route*)action->after[0];

  bool noRouteLeftToRedo = false;
  if ((route->GetnPoints() == 2) && (cc->m_routeState == 0))
    noRouteLeftToRedo = true;

  g_pRouteMan->RemovePointFromRoute(point, route, cc->m_routeState);
  gFrame->InvalidateAllGL();

  if (action->beforeType[0] == Undo_IsOrphanded) {
    pConfig->DeleteWayPoint(point);
    pSelect->DeleteSelectablePoint(point, SELTYPE_ROUTEPOINT);
    if (NULL != pWayPointMan) pWayPointMan->RemoveRoutePoint(point);
  }

  if (noRouteLeftToRedo) {
    cc->undo->InvalidateRedo();
  }

  if (RouteManagerDialog::getInstanceFlag()) {
    if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
      pRouteManagerDialog->UpdateWptListCtrl();
  }

  if (cc->m_routeState > 1) {
    cc->m_routeState--;
    cc->m_prev_pMousePoint = route->GetLastPoint();
    cc->m_prev_rlat = cc->m_prev_pMousePoint->m_lat;
    cc->m_prev_rlon = cc->m_prev_pMousePoint->m_lon;
    route->m_lastMousePointIndex = route->GetnPoints();
  }
}

void doRedoAppendWaypoint(UndoAction* action, ChartCanvas* cc) {
  RoutePoint* point = (RoutePoint*)action->before[0];
  Route* route = (Route*)action->after[0];

  if (action->beforeType[0] == Undo_IsOrphanded) {
    pConfig->AddNewWayPoint(point, -1);
    pSelect->AddSelectableRoutePoint(point->m_lat, point->m_lon, point);
  }

  RoutePoint* prevpoint = route->GetLastPoint();

  route->AddPoint(point);
  pSelect->AddSelectableRouteSegment(prevpoint->m_lat, prevpoint->m_lon,
                                     point->m_lat, point->m_lon, prevpoint,
                                     point, route);

  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateWptListCtrl();

  if (cc->m_routeState > 1) {
    cc->m_routeState++;
    cc->m_prev_pMousePoint = route->GetLastPoint();
    cc->m_prev_rlat = cc->m_prev_pMousePoint->m_lat;
    cc->m_prev_rlon = cc->m_prev_pMousePoint->m_lon;
    route->m_lastMousePointIndex = route->GetnPoints();
  }
}

bool Undo::AnythingToUndo() { return undoStack.size() > m_stackpointer; }

bool Undo::AnythingToRedo() { return m_stackpointer > 0; }

const UndoAction& Undo::GetNextUndoableAction() {
  return undoStack[m_stackpointer];
}

const UndoAction& Undo::GetNextRedoableAction() {
  return undoStack[m_stackpointer - 1];
}

void Undo::InvalidateRedo() {
  if (m_stackpointer == 0) return;

  // Make sure we are not deleting any objects pointed to by
  // potential redo actions.

  for (unsigned int i = 0; i < m_stackpointer; i++) {
    switch (undoStack[i].type) {
      case Undo_DeleteWaypoint:
        undoStack.erase(undoStack.begin() + i);
        break;
      case Undo_CreateWaypoint:
      case Undo_MoveWaypoint:
      case Undo_AppendWaypoint:
        break;
    }
  }

  undoStack.erase(undoStack.begin(), undoStack.begin() + m_stackpointer);
  m_stackpointer = 0;
}

void Undo::InvalidateUndo() {
  undoStack.clear();
  m_stackpointer = 0;
}

bool Undo::UndoLastAction() {
  if (!AnythingToUndo()) return false;
  UndoAction action = GetNextUndoableAction();

  switch (action.type) {
    case Undo_CreateWaypoint:
      doRedoDeleteWaypoint(&action,
                           GetParent());  // Same as delete but reversed.
      m_stackpointer++;
      break;

    case Undo_MoveWaypoint:
      doUndoMoveWaypoint(&action, GetParent());
      m_stackpointer++;
      break;

    case Undo_DeleteWaypoint:
      doUndoDeleteWaypoint(&action, GetParent());
      m_stackpointer++;
      break;

    case Undo_AppendWaypoint:
      m_stackpointer++;
      doUndoAppendWaypoint(&action, GetParent());
      break;
  }
  return true;
}

bool Undo::RedoNextAction() {
  if (!AnythingToRedo()) return false;
  UndoAction action = GetNextRedoableAction();

  switch (action.type) {
    case Undo_CreateWaypoint:
      // Same as delete but reversed.
      doUndoDeleteWaypoint(&action, GetParent());
      m_stackpointer--;
      break;

    case Undo_MoveWaypoint:
      // For Wpt move, redo is same as undo (swap lat/long);
      doUndoMoveWaypoint(&action, GetParent());
      m_stackpointer--;
      break;

    case Undo_DeleteWaypoint:
      doRedoDeleteWaypoint(&action, GetParent());
      m_stackpointer--;
      break;

    case Undo_AppendWaypoint:
      doRedoAppendWaypoint(&action, GetParent());
      m_stackpointer--;
      break;
  }
  return true;
}

bool Undo::BeforeUndoableAction(UndoType type, UndoItemPointer before,
                                UndoBeforePointerType beforeType,
                                UndoItemPointer selectable) {
  if (CancelUndoableAction()) return false;
  ;
  InvalidateRedo();

  m_candidate = UndoAction();
  m_candidate.before.clear();
  m_candidate.beforeType.clear();
  m_candidate.selectable.clear();
  m_candidate.after.clear();

  m_candidate.type = type;
  UndoItemPointer subject = before;

  switch (beforeType) {
    case Undo_NeedsCopy: {
      switch (m_candidate.type) {
        case Undo_MoveWaypoint: {
          wxRealPoint* point = new wxRealPoint;
          RoutePoint* rp = (RoutePoint*)before;
          point->x = rp->m_lon;
          point->y = rp->m_lat;
          subject = point;
          break;
        }
        case Undo_CreateWaypoint:
          break;
        case Undo_DeleteWaypoint:
          break;
        case Undo_AppendWaypoint:
          break;
      }
      break;
    }
    case Undo_IsOrphanded:
      break;
    case Undo_HasParent:
      break;
  }

  m_candidate.before.push_back(subject);
  m_candidate.beforeType.push_back(beforeType);
  m_candidate.selectable.push_back(selectable);

  m_is_inside_undoable_action = true;
  return true;
}

bool Undo::AfterUndoableAction(UndoItemPointer after) {
  if (!m_is_inside_undoable_action) return false;

  m_candidate.after.push_back(after);
  undoStack.push_front(m_candidate);

  if (undoStack.size() > m_depth_setting) {
    undoStack.pop_back();
  }

  m_is_inside_undoable_action = false;
  return true;
}

bool Undo::CancelUndoableAction(bool noDataDelete) {
  if (m_is_inside_undoable_action) {
    if (noDataDelete) {
      for (unsigned int i = 0; i < m_candidate.beforeType.size(); i++) {
        if (m_candidate.beforeType[i] == Undo_IsOrphanded) {
          m_candidate.beforeType[i] = Undo_HasParent;
        }
      }
    }
    m_is_inside_undoable_action = false;
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------------

UndoAction::~UndoAction() {
  assert(before.size() == beforeType.size());

  for (unsigned int i = 0; i < before.size(); i++) {
    switch (beforeType[i]) {
      case Undo_NeedsCopy: {
        switch (type) {
          case Undo_MoveWaypoint:
            if (before[i]) {
              delete (wxRealPoint*)before[i];
              before[i] = NULL;
            }
            break;
          case Undo_DeleteWaypoint:
            break;
          case Undo_CreateWaypoint:
            break;
          case Undo_AppendWaypoint:
            break;
        }
        break;
      }
      case Undo_IsOrphanded: {
        switch (type) {
          case Undo_DeleteWaypoint:
            if (before[i]) {
              delete (RoutePoint*)before[i];
            }
            break;
          case Undo_CreateWaypoint:
            break;
          case Undo_MoveWaypoint:
            break;
          case Undo_AppendWaypoint:
            if (before[i]) {
              delete (RoutePoint*)before[i];
              before[i] = NULL;
            }
            break;
        }
        break;
      }
      case Undo_HasParent:
        break;
    }
  }
  before.clear();
}
