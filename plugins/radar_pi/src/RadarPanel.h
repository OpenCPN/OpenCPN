/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Authors:  David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Douwe Fokkema
 *           Sean D'Epagnier
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register              bdbcat@yahoo.com *
 *   Copyright (C) 2012-2013 by Dave Cowell                                *
 *   Copyright (C) 2012-2016 by Kees Verruijt         canboat@verruijt.net *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#ifndef _RADAR_FRAME_H_
#define _RADAR_FRAME_H_

#include "radar_pi.h"

PLUGIN_BEGIN_NAMESPACE

class RadarPanel : public wxPanel {
 public:
  RadarPanel(radar_pi* pi, RadarInfo* ri, wxWindow* parent);
  bool Create();
  virtual ~RadarPanel();

  void SetCaption(wxString name);  // Set the AUI caption
  void ShowFrame(bool visible);
  bool IsPaneShown();

  void close(wxAuiManagerEvent& event);

  wxPoint GetPos();
  wxAuiManager* m_aui_mgr;
  wxString m_dock;

 private:
  wxWindow* m_parent;
  radar_pi* m_pi;
  RadarInfo* m_ri;
  wxString m_aui_name;
  wxBoxSizer* m_sizer;
  wxStaticText* m_text;
  wxSize m_best_size;
};

PLUGIN_END_NAMESPACE

#endif
