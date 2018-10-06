/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Plugin Friends
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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

#ifndef __GRIBREQUESTDIALOG_H__
#define __GRIBREQUESTDIALOG_H__

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include "GribUIDialogBase.h"
#include "GribUIDialog.h"

#include "ocpn_plugin.h"


//----------------------------------------------------------------------------------------------------------
//    Request setting Specification
//----------------------------------------------------------------------------------------------------------
class GribRequestSetting : public GribRequestSettingBase
{
public:
      GribRequestSetting( GRIBUICtrlBar &parent );

      ~GribRequestSetting();

      void OnClose( wxCloseEvent& event );
      void SetVpSize(PlugIn_ViewPort *vp);
      void OnVpChange(PlugIn_ViewPort *vp);
      bool MouseEventHook( wxMouseEvent &event );
      bool RenderZoneOverlay( wxDC &dc );
      bool RenderGlZoneOverlay();
      bool DoRenderZoneOverlay();
      void SetRequestDialogSize();
      void StopGraphicalZoneSelection();
      void Save() { wxCommandEvent evt; OnSaveMail( evt); }

      wxString m_RequestConfigBase;
      wxString m_MailToAddresses;
      int m_RenderZoneOverlay;

      wxPoint         m_StartPoint;
      PlugIn_ViewPort *m_Vp;
      double          m_Lat;
      double          m_Lon;

private:

      void ApplyRequestConfig( unsigned rs, unsigned it, unsigned tr );
      wxString WriteMail();
      int EstimateFileSize( double *size );

      void InitRequestConfig();
      void OnExit(wxCommandEvent &event) { wxCloseEvent evt; OnClose ( evt ); }
      void OnTopChange(wxCommandEvent &event);
      void OnMovingClick( wxCommandEvent& event );
      void OnAnyChange( wxCommandEvent& event );
      void OnAnySpinChange( wxSpinEvent& event ) { wxCommandEvent evt; OnAnyChange( evt); }
      void OnTimeRangeChange( wxCommandEvent& event );
      void OnSendMaiL( wxCommandEvent& event );
      void OnSaveMail( wxCommandEvent& event );
      void OnZoneSelectionModeChange( wxCommandEvent& event  );
      void OnCancel( wxCommandEvent& event ) { wxCloseEvent evt; OnClose( evt); }
      void OnCoordinatesChange( wxSpinEvent& event );
      void OnMouseEventTimer( wxTimerEvent & event);
      void SetCoordinatesText();

      GRIBUICtrlBar &m_parent;

      wxDC           *m_pdc;
      wxTimer        m_tMouseEventTimer;
      wxTimer        m_tMouseClickTimer;
      wxMouseEvent   m_SingleClickEvent;

      bool IsZYGRIB;
      bool IsGFS;
      int  m_MailError_Nb;
      int  m_SendMethod;
      bool m_AllowSend;
	  bool  m_IsMaxLong;
};

#endif