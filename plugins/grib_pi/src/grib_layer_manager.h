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

#ifndef _GRIB_LAYER_MANAGER_H_
#define _GRIB_LAYER_MANAGER_H_

#include "GribUIDialog.h"

/** GribLayerManager manages multiple GRIB layers and provides data access. */
class GribLayerManager {
public:
    GribLayerManager();
    ~GribLayerManager();
    
    // Layer management
    bool SetLayerFile(int layerIndex, const wxArrayString& filenames);
    void EnableLayer(int layerIndex, bool enable);
    bool IsLayerEnabled(int layerIndex) const;
    void ClearLayer(int layerIndex);
    
    // Data access methods that consider all enabled layers
    GribTimelineRecordSet* GetTimelineRecordSet(wxDateTime time);
    double GetTimeInterpolatedValue(int recordType, double lat, double lon, wxDateTime time);
    bool GetTimeInterpolatedValues(double& magnitude, double& angle, 
                                 int recordTypeX, int recordTypeY,
                                 double lat, double lon, wxDateTime time);
                                 
    // Get individual layer data if needed.
    GribTimelineRecordSet* GetLayerTimelineRecordSet(int layerIndex, wxDateTime time);
    
    // Methods to maintain compatibility with existing code
    GRIBFile* GetCurrentGribFile() const;  // Returns highest priority enabled layer's file
    
private:
    static const int MAX_LAYERS = 6;  // Maximum number of supported layers
    GribLayer m_Layers[MAX_LAYERS];   // Fixed array of layers
};

#endif // _GRIB_LAYER_MANAGER_H_