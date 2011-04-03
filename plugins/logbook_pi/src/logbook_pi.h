/******************************************************************************
 * $Id: logbook_pi.h, v0.1 2011/03/18 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Logbook Plugin
 * Author:   Jean-Eudes Onfray
 *
 ***************************************************************************
 *   Copyright (C) 2011 by Jean-Eudes Onfray   *
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

#ifndef _LogbookPI_H_
#define _LogbookPI_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#define     PLUGIN_VERSION_MAJOR    0
#define     PLUGIN_VERSION_MINOR    1

#define     MY_API_VERSION_MAJOR    1
#define     MY_API_VERSION_MINOR    4

#include <wx/fileconf.h>
#include <wx/filepicker.h>
#include <wx/spinctrl.h>
#include "../../../include/ocpn_plugin.h"

#include "nmea0183/nmea0183.h"

// Data must be fresher thant this delay to be saved
#define     DATA_VALIDITY    60
#define     LOGBOOK_EMPTY_VALUE    999.

//----------------------------------------------------------------------------------------------------------
//    The PlugIn Class Definition
//----------------------------------------------------------------------------------------------------------

enum
{
      OCPN_LBI_MAIN,
      OCPN_LBI_MIN,
      OCPN_LBI_MAX,
      OCPN_LBI_AVG
};

class LogbookItem
{
      public:
            LogbookItem();

            void SetValue(double);
            void Reset();
            bool IsValid();
            double GetValue(short which, bool reset);
            wxString GetFormattedValue(short which, wxString format, bool reset);

      private:
            double      m_value, m_min, m_max, m_avg;
            int         m_count;
            wxDateTime  m_lastReceived;

};

class logbook_pi : public opencpn_plugin, wxTimer
{
public:
      logbook_pi(void *ppimgr);

      int Init(void);
      bool DeInit(void);

      int GetAPIVersionMajor();
      int GetAPIVersionMinor();
      int GetPlugInVersionMajor();
      int GetPlugInVersionMinor();
      wxBitmap *GetPlugInBitmap();
      wxString GetCommonName();
      wxString GetShortDescription();
      wxString GetLongDescription();

      //void OnTimer(wxTimerEvent& event);
      void Notify();

      void SetNMEASentence(wxString &sentence);
      void ShowPreferencesDialog( wxWindow* parent );

private:
      bool LoadConfig(void);
      bool SaveConfig(void);
      void ApplyConfig(void);

      void WriteLogEntry( wxString entry );

      wxTimer           m_timer;
      wxFileConfig     *m_pconfig;
      wxString          m_filename;
      int               m_interval;

      NMEA0183          m_NMEA0183;                 // Used to parse NMEA Sentences
      short             mPriPosition, mPriCOGSOG, mPriHeadingM, mPriHeadingT, mPriVar, mPriDateTime, mPriWindR, mPriWindT, mPriDepth;
      LogbookItem       mLat, mLon, mCOG, mSOG, mHeadingM, mHeadingT, mSTW, mAWA, mAWS, mTWA, mTWS, mDepth, mTemp, mVar;

//DECLARE_EVENT_TABLE();
};

class LogbookPreferencesDialog : public wxDialog
{
public:
      LogbookPreferencesDialog( wxWindow *pparent, wxWindowID id, wxString filename, int interval );
      ~LogbookPreferencesDialog() {}

      void OnCloseDialog(wxCloseEvent& event);
      void SaveLogbookConfig();

      wxString m_filename;
      int m_interval;

private:
      wxFilePickerCtrl *m_pFilename;
      wxSpinCtrl       *m_pInterval;
};

#endif
