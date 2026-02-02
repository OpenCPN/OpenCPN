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
 * Undo features argument  definitions
 */

#ifndef UnDO_DefS_h
#define UnDO_DefS_h

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

#endif  // UnDO_DefS_h
