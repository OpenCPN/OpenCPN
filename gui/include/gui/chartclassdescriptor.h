/**************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *   Copyright (C) 2010 by Mark A Sikes                                    *
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
 * Basic chart info storage
 */

#ifndef __CHARTCLASSDESCRIPTOR_H__
#define __CHARTCLASSDESCRIPTOR_H__

#include <map>
#include <memory>
#include <vector>

#include <wx/progdlg.h>

#include "model/ocpn_types.h"
#include "bbox.h"
#include "LLRegion.h"
#include "chartdb_thread.h"
#include "ocpn_plugin.h"

enum { BUILTIN_DESCRIPTOR = 0, PLUGIN_DESCRIPTOR };

class ChartClassDescriptor {
public:
  ChartClassDescriptor() {};
  virtual ~ChartClassDescriptor() {}

  ChartClassDescriptor(wxString classn, wxString mask, opencpn_plugin *plugin,
                       int type)
      : m_class_name(classn),
        m_search_mask(mask),
        m_descriptor_type(type),
        m_plugin(plugin) {};

  wxString m_class_name;
  wxString m_search_mask;
  int m_descriptor_type;
  opencpn_plugin *m_plugin;
};

#endif
