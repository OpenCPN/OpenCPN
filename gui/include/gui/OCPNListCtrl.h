/***************************************************************************
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

#ifndef __OCPNLISTCTRL_H__
#define __OCPNLISTCTRL_H__

#include <wx/listctrl.h>

#include "AISTargetListDialog.h"
#include "ais.h"

/**
 * A custom list control for displaying AIS target information. Extends
 * wxListCtrl to provide specialized functionality for displaying and managing
 * AIS target data in OpenCPN.
 */
class OCPNListCtrl : public wxListCtrl {
public:
  OCPNListCtrl(AISTargetListDialog* parent, wxWindowID id, const wxPoint& pos,
               const wxSize& size, long style);
  ~OCPNListCtrl();

  wxString OnGetItemText(long item, long column) const;
  int OnGetItemColumnImage(long item, long column) const;

  wxString GetTargetColumnData(AisTargetData* pAISTarget, long column) const;

  AISTargetListDialog* m_parent;
};

#endif
