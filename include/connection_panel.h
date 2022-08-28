/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David S. Register                               *
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
 **************************************************************************/

#ifndef __DSPORTTYPE_H__
#include <dsPortType.h>
#endif

#ifndef _CONNECTIONPANEL_H
#define _CONNECTIONPANEL_H

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

class ConnectionParams;
class ConnectionsDialog;


class ConnectionParamsPanel : public wxPanel {
public:
  ConnectionParamsPanel(wxWindow *parent, wxWindowID id, const wxPoint &pos,
                        const wxSize &size,
                        ConnectionParams *p_itemConnectionParams,
                        ConnectionsDialog *pContainer);
  ~ConnectionParamsPanel();

  void OnSelected(wxMouseEvent &event);
  void SetSelected(bool selected);
  void OnPaint(wxPaintEvent &event);
  void OnEraseBackground(wxEraseEvent &event);
  void CreateControls(void);
  void OnEnableCBClick(wxCommandEvent &event);
  void Update(ConnectionParams *ConnectionParams);

  bool GetSelected() { return m_bSelected; }
  int GetUnselectedHeight() { return m_unselectedHeight; }
  ConnectionParams *m_pConnectionParams;

private:
  ConnectionsDialog *m_pContainer;
  bool m_bSelected;
  wxStaticText *m_pName;
  wxColour m_boxColour;
  int m_unselectedHeight;
  wxCheckBox *m_cbEnable;
  wxStaticText *t2;
  wxStaticText *t4;
  wxStaticText *t6;
  wxStaticText *t12;
  wxStaticText *t14;
  wxStaticText *t16;
  wxStaticText *t18;

  wxStaticText *t21;

  DECLARE_EVENT_TABLE()
};


#endif
