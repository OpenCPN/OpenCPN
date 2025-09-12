/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#include "Layer.h"

#include <algorithm>

extern bool g_bShowLayers;
extern LayerList *pLayerList;

Layer::Layer(void) {
  m_bIsVisibleOnChart = g_bShowLayers;
  m_bIsVisibleOnListing = false;
  m_bHasVisibleNames = wxCHK_UNDETERMINED;
  m_NoOfItems = 0;
  m_LayerType = "";
  m_LayerName = "";
  m_LayerFileName = "";
  m_LayerDescription = "";
  m_CreateTime = wxDateTime::Now();
}

Layer::~Layer(void) {
  //  Remove this layer from the global layer list
  if (pLayerList) {
    auto found = std::find(pLayerList->begin(), pLayerList->end(), this);
    if (found != pLayerList->end()) pLayerList->erase(found);
  }
}
