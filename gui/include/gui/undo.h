/**************************************************************************
 *   Copyright (C) 2012 by David S. Register                               *
 *   Copyright (c) 2012 Jesper Weissglas                                   *
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
 ***************************************************************************/

/** \file undo.h Framework for Undo features. */

#ifndef UNDO_H
#define UNDO_H

#include <vector>
#include <deque>

class ChartCanvas;

enum UndoType {
  Undo_CreateWaypoint,
  Undo_DeleteWaypoint,
  Undo_AppendWaypoint,
  Undo_MoveWaypoint
};

enum UndoBeforePointerType { Undo_IsOrphanded, Undo_NeedsCopy, Undo_HasParent };

typedef void* UndoItemPointer;

class UndoAction {
public:
  ~UndoAction();
  wxString Description() const;

  UndoType type;
  std::vector<UndoItemPointer> before;
  std::vector<UndoBeforePointerType> beforeType;
  std::vector<UndoItemPointer> after;
  std::vector<UndoItemPointer> selectable;
};

class Undo {
public:
  Undo(ChartCanvas* parent);
  ~Undo();
  bool AnythingToUndo() const;
  bool AnythingToRedo() const;
  void InvalidateRedo();
  void InvalidateUndo();
  bool InUndoableAction() const { return m_is_inside_undoable_action; }
  const UndoAction& GetNextUndoableAction() const;
  const UndoAction& GetNextRedoableAction() const;
  bool UndoLastAction();
  bool RedoNextAction();
  bool BeforeUndoableAction(UndoType type, UndoItemPointer before,
                            UndoBeforePointerType beforeType,
                            UndoItemPointer selectable);
  bool AfterUndoableAction(UndoItemPointer after);
  bool CancelUndoableAction(bool noDataDelete = false);

private:
  ChartCanvas* m_parent;
  bool m_is_inside_undoable_action;
  UndoAction m_candidate;
  unsigned int m_stackpointer;
  unsigned int m_depth_setting;
  std::deque<UndoAction> m_undo_stack;
};

#endif
