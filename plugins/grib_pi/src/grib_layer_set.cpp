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

#include "grib_layer_set.h"
#include "GribRecordSet.h"

GRIBLayer* GRIBLayerSet::AddLayer(const wxString& name,
                                  const wxArrayString& files) {
  // Don't allow duplicate names
  if (m_layers.find(name) != m_layers.end()) {
    return nullptr;
  }

  auto layer = std::make_unique<GRIBLayer>(name, files);
  if (!layer->IsValid()) {
    return nullptr;
  }

  auto* layerPtr = layer.get();
  m_layers[name] = std::move(layer);
  return layerPtr;
}

bool GRIBLayerSet::RemoveLayer(const wxString& name) {
  return m_layers.erase(name) > 0;
}

GRIBLayer* GRIBLayerSet::GetLayer(const wxString& name) {
  auto it = m_layers.find(name);
  return it != m_layers.end() ? it->second.get() : nullptr;
}

const GRIBLayer* GRIBLayerSet::GetLayer(const wxString& name) const {
  auto it = m_layers.find(name);
  return it != m_layers.end() ? it->second.get() : nullptr;
}

std::vector<GRIBLayer*> GRIBLayerSet::GetEnabledLayers() {
  std::vector<GRIBLayer*> enabled;
  for (auto& pair : m_layers) {
    if (pair.second->IsEnabled()) {
      enabled.push_back(pair.second.get());
    }
  }
  return enabled;
}

GribTimelineRecordSet* GRIBLayerSet::GetTimeLineRecordSet(wxDateTime time) {
  auto enabled = GetEnabledLayers();
  if (enabled.empty()) {
    return nullptr;
  }

  // TODO: Create a composite timeline record set from enabled layers
  // 1. Create a new GribTimelineRecordSet
  // 2. For each enabled layer:
  //    - Get its records for the target time
  //    - Merge them into the composite set, with newer layers taking precedence
  return nullptr;
}

double GRIBLayerSet::GetTimeInterpolatedValue(int idx, double lon, double lat,
                                              wxDateTime time) const {
  // Get all enabled layers sorted by reference time (newest first)
  std::vector<const GRIBLayer*> enabled;
  for (const auto& pair : m_layers) {
    if (pair.second->IsEnabled()) {
      enabled.push_back(pair.second.get());
    }
  }

  if (enabled.empty()) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  // Sort by reference time, newest first
  std::sort(enabled.begin(), enabled.end(),
            [](const GRIBLayer* a, const GRIBLayer* b) {
              return a->GetRefDateTime() > b->GetRefDateTime();
            });

  // Try each layer in order until we get a valid value
  for (const auto* layer : enabled) {
    double val = layer->GetTimeInterpolatedValue(idx, lon, lat, time);
    if (!std::isnan(val)) {
      return val;
    }
  }

  return std::numeric_limits<double>::quiet_NaN();
}
