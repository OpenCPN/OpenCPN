/******************************************************************************
 * $Id: demo_pi.h,v 1.8 2010/06/21 01:54:37 bdbcat Exp $
 *
 * Project:  OpenCPN
 * Purpose:  DEMO Plugin
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   bdbcat@yahoo.com   *
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

#ifndef _DEMOPI_H_
#define _DEMOPI_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#define     PLUGIN_VERSION_MAJOR    1
#define     PLUGIN_VERSION_MINOR    0

#define     MY_API_VERSION_MAJOR    1
#define     MY_API_VERSION_MINOR    1

#include "../../../include/ocpn_plugin.h"

#include "nmea0183/nmea0183.h"


class DemoWindow;

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------


class demo_pi : public opencpn_plugin
{
public:
      demo_pi(void *ppimgr):opencpn_plugin(ppimgr){}

//    The required PlugIn Methods
      int Init(void);
      bool DeInit(void);

      int GetAPIVersionMajor();
      int GetAPIVersionMinor();
      int GetPlugInVersionMajor();
      int GetPlugInVersionMinor();

      wxString GetCommonName();
      wxString GetShortDescription();
      wxString GetLongDescription();

//    The optional method overrides

      void SetNMEASentence(wxString &sentence);
      void OnContextMenuItemCallback(int id);
      void UpdateAuiStatus(void);
      


private:
      wxWindow         *m_parent_window;

      DemoWindow       *m_pdemo_window;
      wxAuiManager     *m_AUImgr;
      int               m_show_id;
      int               m_hide_id;

};



class DemoWindow : public wxWindow
{
public:
      DemoWindow(wxWindow *pparent, wxWindowID id);
      ~DemoWindow();

      void OnPaint(wxPaintEvent& event);
      void SetSentence(wxString &sentence);
      void OnSize(wxSizeEvent& event);

      NMEA0183        m_NMEA0183;                 // Used to parse NMEA Sentences

      wxString          m_NMEASentence;
      double            mLat, mLon, mSog, mCog, mVar;


DECLARE_EVENT_TABLE()
};


#endif



