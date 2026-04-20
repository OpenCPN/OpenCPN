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

#include "undo_defs.h"
#include "chcanv.h"

class Undo {
public:
  Undo(AbstractChartCanvas* parent);
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
  AbstractChartCanvas* GetParent() { return m_parent; }

private:
  AbstractChartCanvas* m_parent;
  bool isInsideUndoableAction;
  UndoAction* candidate;
  unsigned int stackpointer;
  unsigned int depthSetting;
  std::deque<UndoAction*> undoStack;
};

#endif
