/**************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
 ***************************************************************************/

/**
 * \file
 *
 * Chart object layer
 */

#ifndef LAYER_H_
#define LAYER_H_

#include <list>

#include <wx/checkbox.h>
#include <wx/datetime.h>
#include <wx/string.h>

class Layer;  // Forward

using LayerList = std::list<Layer *>;
extern LayerList *pLayerList;  ///< Global instance

/**
 * Represents a layer of chart objects in OpenCPN. A Layer in OpenCPN is a
 * collection of chart objects (such as waypoints, routes, or tracks) that can
 * be managed and displayed as a group. Layers provide a way to organize and
 * control the visibility of related chart objects.
 */
class Layer {
public:
  Layer();
  ~Layer();
  wxString CreatePropString() { return m_LayerFileName; }
  bool IsVisibleOnChart() { return m_bIsVisibleOnChart; }
  void SetVisibleOnChart(bool viz = true) { m_bIsVisibleOnChart = viz; }
  bool IsVisibleOnListing() { return m_bIsVisibleOnListing; }
  void SetVisibleOnListing(bool viz = true) { m_bIsVisibleOnListing = viz; }
  wxCheckBoxState HasVisibleNames() { return m_bHasVisibleNames; }
  void SetVisibleNames(wxCheckBoxState viz = wxCHK_UNDETERMINED) {
    m_bHasVisibleNames = viz;
  }

  bool m_bIsVisibleOnChart;
  bool m_bIsVisibleOnListing;
  wxCheckBoxState m_bHasVisibleNames;
  long m_NoOfItems;
  int m_LayerID;

  wxString m_LayerName;
  wxString m_LayerFileName;
  wxString m_LayerDescription;
  wxString m_LayerType;
  wxDateTime m_CreateTime;
};

#endif
