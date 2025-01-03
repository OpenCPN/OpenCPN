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

#ifndef __GRIB_LAYER_H__
#define __GRIB_LAYER_H__

#include <memory>
#include <wx/string.h>
#include "GribReader.h"

class GRIBFile;
class GribTimelineRecordSet;

/**
 * Represents a single GRIB data layer that can be enabled/disabled
 * independently.
 *
 * Each layer manages a GRIBFile instance and provides controlled access to its
 * data. Layers can be individually enabled or disabled, allowing users to
 * combine different GRIB datasets as needed.
 */
class GRIBLayer {
public:
  /**
   * Creates a new GRIB layer.
   *
   * @param name User-friendly name for the layer
   * @param files Array of GRIB file paths to load
   * @param cumRec Whether to copy first cumulative record for
   * precipitation/clouds
   * @param waveRec Whether to copy missing wave records
   */
  GRIBLayer(const wxString& name, const wxArrayString& files,
            bool cumRec = true, bool waveRec = true);
  ~GRIBLayer() = default;

  /**
   * @name Layer State Management
   * Methods to control and query the layer's state
   */
  bool IsEnabled() const { return m_enabled; }
  void SetEnabled(bool enabled) { m_enabled = enabled; }
  bool IsValid() const { return m_file && m_file->IsOK(); }

  /**
   * @name Layer Metadata
   * Methods to access layer information
   */
  const wxString& GetName() const { return m_name; }
  wxString GetLastError() const;
  time_t GetRefDateTime() const;

  /**
   * @name Data Access
   * Methods to access the layer's GRIB data
   */
  GRIBFile* GetFile() { return m_file.get(); }
  const GRIBFile* GetFile() const { return m_file.get(); }

  /**
   * Gets interpolated value for a specific location and time.
   *
   * @param idx Data type index (wind, pressure, etc)
   * @param lon Longitude in degrees
   * @param lat Latitude in degrees
   * @param time Target time for interpolation
   * @return Interpolated value or NaN if not available
   */
  double GetTimeInterpolatedValue(int idx, double lon, double lat,
                                  wxDateTime time) const;

private:
  wxString m_name;                   ///< User-friendly layer name
  bool m_enabled{true};              ///< Layer visibility state
  std::unique_ptr<GRIBFile> m_file;  ///< Underlying GRIB file data
};

#endif  // __GRIB_LAYER_H__