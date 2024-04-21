/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2024 by David S. Register                               *
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

#ifndef _CONNECT_DIALOG_H
#define _CONNECT_DIALOG_H

#include <wx/checkbox.h>
#include <wx/listctrl.h>
#include <wx/radiobut.h>
#include <wx/stattext.h>

#include "model/conn_params.h"
#include "model/comm_util.h"

#include "observable.h"

class options;
class ConnectionParamsPanel;

//----------------------------------------------------------------------------
// ConnectionsDialog
//----------------------------------------------------------------------------
class ConnectionsDialog : public wxEvtHandler {
public:
  ConnectionsDialog();
  ConnectionsDialog(wxScrolledWindow *container, options *parent);

  ~ConnectionsDialog();

  void Init(void);
  void SetInitialSettings(void);
  void ApplySettings(bool bFinal = false);
  void SetSelectedConnectionPanel(ConnectionParamsPanel *panel);
  void OnAddDatasourceClick(wxCommandEvent &event);
  void OnEditDatasourceClick(wxCommandEvent &event);
  void OnRemoveDatasourceClick(wxCommandEvent &event);

  void OnShowGpsWindowCheckboxClick(wxCommandEvent& event);
  void EnableConnection(ConnectionParams *conn, bool value);
  void OnPriorityDialog(wxCommandEvent &event);

  void FillSourceList();
  void UpdateSourceList(bool bResort);
  bool SortSourceList(void);

  void UpdateDatastreams(bool bfinal = false);
  void OnSize(wxSizeEvent &ev);

//private:
  wxScrolledWindow *m_container;
  options *m_parent;
  ConnectionParams *mSelectedConnection;

  wxButton *m_buttonAdd, *m_buttonRemove, *m_buttonEdit;

  wxTextCtrl *m_tFilterSec;
  wxStaticText *m_stFilterSec;
  wxCheckBox *m_cbGarminUploadHost;
  wxCheckBox *m_cbFurunoGP3X, *m_cbNMEADebug, *m_cbFilterSogCog, *m_cbInput;
  wxCheckBox *m_cbAPBMagnetic;
  wxButton  *m_ButtonPriorityDialog;
  wxBoxSizer *boxSizerConnections;
  wxBoxSizer *m_bSizerOuterContainer;
  wxStaticBoxSizer *m_sbSizerLB;
  wxTextCtrl *m_TalkerIdText;

#ifdef __ANDROID__
  wxPanel *m_scrollWinConnections;
#else
  wxScrolledWindow *m_scrollWinConnections;
#endif

};


#endif    //_CONNECT_DIALOG_H
