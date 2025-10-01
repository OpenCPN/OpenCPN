/***************************************************************************
 *   Copyright (C) 2012 Jesper Weissglas                                   *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Framework for Undo features
 */

#ifndef UNDO_H
#define UNDO_H

#include <wx/string.h>

#include <vector>
#include <deque>

#include "chcanv.h"

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
  wxString Description();

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
  bool AnythingToUndo();
  bool AnythingToRedo();
  void InvalidateRedo();
  void InvalidateUndo();
  void Invalidate();
  bool InUndoableAction() { return isInsideUndoableAction; }
  UndoAction* GetNextUndoableAction();
  UndoAction* GetNextRedoableAction();
  bool UndoLastAction();
  bool RedoNextAction();
  bool BeforeUndoableAction(UndoType type, UndoItemPointer before,
                            UndoBeforePointerType beforeType,
                            UndoItemPointer selectable);
  bool AfterUndoableAction(UndoItemPointer after);
  bool CancelUndoableAction(bool noDataDelete = false);
  ChartCanvas* GetParent() { return m_parent; }

private:
  ChartCanvas* m_parent;
  bool isInsideUndoableAction;
  UndoAction* candidate;
  unsigned int stackpointer;
  unsigned int depthSetting;
  std::deque<UndoAction*> undoStack;
};

#endif
