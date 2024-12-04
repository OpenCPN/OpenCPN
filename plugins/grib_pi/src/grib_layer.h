/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Plugin Freinds
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2014 by David S. Register                               *
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
 */

#ifndef _GRIB_LAYER_H_
#define _GRIB_LAYER_H_

#include "GribUIDialog.h"

/** GribLayer encapsulates a single GRIB file with associated state. */
class GribLayer {
public:
    GribLayer() : m_bEnabled(false), m_pGribFile(nullptr) {}
    ~GribLayer() { delete m_pGribFile; }
    
    /** Enable/disable this layer. */
    void SetEnabled(bool enabled) { m_bEnabled = enabled; }
    bool IsEnabled() const { return m_bEnabled; }
    
    /** Set/get the GRIB file for this layer */
    void SetGribFile(GRIBFile* file) { 
        delete m_pGribFile;
        m_pGribFile = file; 
    }
    GRIBFile* GetGribFile() const { return m_pGribFile; }
    bool IsOK() const { return m_pGribFile && m_pGribFile->IsOK(); }
    
    // Take ownership of GRIBFile
    void Assign(GRIBFile* file) {
        delete m_pGribFile;
        m_pGribFile = file;
        m_enabled = file && file->IsOK();
    }

    void Clear() {
        delete m_pGribFile;
        m_pGribFile = nullptr;
        m_enabled = false;
    }

private:
    bool m_bEnabled;           /// Whether this layer is currently enabled.
    GRIBFile* m_pGribFile;     /// The GRIB file associated with this layer.
};

#endif // _GRIB_LAYER_H_