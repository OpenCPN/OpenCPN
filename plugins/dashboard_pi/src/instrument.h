/******************************************************************************
 * $Id: instrument.h, v1.0 2010/08/30 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  DashBoard Plugin
 * Author:   Jean-Eudes Onfray
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   $EMAIL$   *
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

#ifndef _INSTRUMENT_H_
#define _INSTRUMENT_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

// Required GetGlobalColor and OCPNGetFont
#include "../../../include/ocpn_plugin.h"
#include <wx/dcbuffer.h>

class DashboardInstrument;
class DashboardInstrument_Single;
class DashboardInstrument_Double;

class DashboardInstrument : public wxWindow
{
public:
      DashboardInstrument(wxWindow *pparent, wxWindowID id, wxString title);
      ~DashboardInstrument();

      virtual void OnPaint(wxPaintEvent& WXUNUSED(event));

private:

protected:
      int m_TitleHeight;
      wxString          m_title;

      virtual void Draw(wxBufferedDC* dc);

};

class DashboardInstrument_Single : public DashboardInstrument
{
public:
      DashboardInstrument_Single(wxWindow *pparent, wxWindowID id, wxString title);
      ~DashboardInstrument_Single();

      void SetData(wxString data);

protected:
      wxString          m_data;

      void Draw(wxBufferedDC* dc);

};

class DashboardInstrument_Double : public DashboardInstrument
{
public:
      DashboardInstrument_Double(wxWindow *pparent, wxWindowID id, wxString title);
      ~DashboardInstrument_Double();

      void SetData(wxString data1, wxString data2);

protected:
      wxString          m_data1;
      wxString          m_data2;

      void Draw(wxBufferedDC* dc);

};

#endif

