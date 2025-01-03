/***************************************************************************
 *   Copyright (C) 2024 by OpenCPN Development Team                        *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 **************************************************************************/

#ifndef __GRIB_LAYER_SET_H__
#define __GRIB_LAYER_SET_H__

#include <map>
#include <memory>
#include <vector>
#include <wx/string.h>
#include "grib_layer.h"

class GribTimelineRecordSet;

/**
 * Manages a collection of GRIB layers and their interactions.
 *
 * GRIBLayerSet coordinates multiple GRIB layers, handling:
 * - Layer addition/removal
 * - Layer enable/disable state
 * - Data combination across enabled layers
 * - Time synchronization
 */
class GRIBLayerSet {
public:
  GRIBLayerSet() = default;
  ~GRIBLayerSet() = default;

  /**
   * @name Layer Management
   * Methods to manage the layer collection
   */
  /**
   * Adds a new layer to the set.
   *
   * @param name Unique identifier for the layer
   * @param files GRIB files to include in the layer
   * @return Pointer to the created layer or nullptr if creation failed
   */
  GRIBLayer* AddLayer(const wxString& name, const wxArrayString& files);

  /**
   * Removes a layer from the set.
   *
   * @param name Layer identifier
   * @return true if layer was found and removed
   */
  bool RemoveLayer(const wxString& name);

  /**
   * Finds a layer by name.
   *
   * @param name Layer identifier
   * @return Pointer to the layer or nullptr if not found
   */
  GRIBLayer* GetLayer(const wxString& name);
  const GRIBLayer* GetLayer(const wxString& name) const;

  /**
   * Gets all currently enabled layers.
   *
   * @return Vector of pointers to enabled layers
   */
  std::vector<GRIBLayer*> GetEnabledLayers();

  /**
   * @name Data Access
   * Methods to access combined data across layers
   */
  /**
   * Creates a timeline record set combining data from all enabled layers.
   *
   * This is the main method for accessing interpolated data across all
   * enabled layers. The returned record set contains:
   * - Data from the newest available layer when values overlap
   * - Interpolated values between time points
   * - Combined coverage from all enabled layers
   *
   * @param time Target time for interpolation
   * @return New GribTimelineRecordSet owned by the caller
   */
  GribTimelineRecordSet* GetTimeLineRecordSet(wxDateTime time);

  /**
   * Gets interpolated value from enabled layers at a specific point.
   *
   * @param idx Data type index (wind, pressure, etc)
   * @param lon Longitude in degrees
   * @param lat Latitude in degrees
   * @param time Target time for interpolation
   * @return Interpolated value (from newest layer if multiple layers have data)
   */
  double GetTimeInterpolatedValue(int idx, double lon, double lat,
                                  wxDateTime time) const;

private:
  std::map<wxString, std::unique_ptr<GRIBLayer>>
      m_layers;  ///< All managed layers
};

#endif  // __GRIB_LAYER_SET_H__