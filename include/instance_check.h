/***************************************************************************
 *   Copyright (C) 2023 Alec Leamas                                        *
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

#ifndef INSTANCE_CHECK_H__
#define INSTANCE_CHECK_H__

#include <memory>

/** Common interface for all instance checkers. */
class InstanceCheck {
public:
    /** @return Reference to an InstanceCheck implementation. */
    static InstanceCheck& GetInstance();

    virtual ~InstanceCheck() = default;

    /** Return true if this process is the primary opencpn instance. */
    virtual bool IsMainInstance() = 0;

    /** Wait until this object can be used for example for Dbus connection. */
    virtual void WaitUntilValid() {};

    /**
     * Remove all persistent instance state, including possible lock file
     * and defunct opencpn processes.
     */
    virtual void CleanUp() {};

    /** Do whatever needed before wxWidget's checks triggers. */
    virtual void OnExit () {};
};

#endif   // INSTANCE_CHECK_H__
