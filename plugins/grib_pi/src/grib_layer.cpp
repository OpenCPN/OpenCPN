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

#include "grib_layer.h"
#include "GribRecord.h"
#include "GribRecordSet.h"

GRIBLayer::GRIBLayer(const wxString& name, const wxArrayString& files,
                     bool cumRec, bool waveRec)
    : m_name(name) {
  if (!files.IsEmpty()) {
    m_file = std::make_unique<GRIBFile>(files, cumRec, waveRec);
  }
}

wxString GRIBLayer::GetLastError() const {
  return m_file ? m_file->GetLastMessage() : wxString();
}

time_t GRIBLayer::GetRefDateTime() const {
  return m_file ? m_file->GetRefDateTime() : 0;
}

double GRIBLayer::GetTimeInterpolatedValue(int idx, double lon, double lat,
                                           wxDateTime time) const {
  if (!m_enabled || !m_file || !m_file->IsOK()) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  // TODO: Implement time interpolation using GribRecord functionality
  // This will need to:
  // 1. Find the records before and after the requested time
  // 2. Interpolate between them
  // 3. Handle edge cases (no data, single record, etc.)

  return std::numeric_limits<double>::quiet_NaN();  // Placeholder
}
