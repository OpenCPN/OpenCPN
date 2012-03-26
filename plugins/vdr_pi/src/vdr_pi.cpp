/******************************************************************************
 * $Id: vdr_pi.cpp, v0.2 2011/05/23 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  VDR Plugin
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


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <typeinfo>
#include "vdr_pi.h"
#include "icons.h"

// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin* create_pi(void *ppimgr)
{
    return new vdr_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin* p)
{
    delete p;
}




//---------------------------------------------------------------------------------------------------------
//
//    VDR PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------

vdr_pi::vdr_pi(void *ppimgr)
      :opencpn_plugin_16(ppimgr), wxTimer(this)
{
      // Create the PlugIn icons
      initialize_images();
}

int vdr_pi::Init(void)
{
      AddLocaleCatalog( _T("opencpn-vdr_pi") );

      //    Get a pointer to the opencpn configuration object
      m_pconfig = GetOCPNConfigObject();
      m_pauimgr = GetFrameAuiManager();
      m_pvdrcontrol = NULL;

      //    And load the configuration items
      LoadConfig();

      //    This PlugIn needs two toolbar icons
      m_tb_item_id_record = InsertPlugInTool(_T(""), _img_vdr_record, _img_vdr_record, wxITEM_CHECK,
            _("Record"), _T(""), NULL, VDR_TOOL_POSITION, 0, this);
      m_tb_item_id_play = InsertPlugInTool(_T(""), _img_vdr_play, _img_vdr_play, wxITEM_CHECK,
            _("Play"), _T(""), NULL, VDR_TOOL_POSITION, 0, this);
      m_recording = false;

      return (
           WANTS_TOOLBAR_CALLBACK    |
           INSTALLS_TOOLBAR_TOOL     |
           WANTS_CONFIG              |
           WANTS_NMEA_SENTENCES      |
           WANTS_AIS_SENTENCES
            );
}

bool vdr_pi::DeInit(void)
{
      SaveConfig();
      if ( IsRunning() ) // Timer started?
      {
            Stop(); // Stop timer
            m_istream.Close();
      }

      if ( m_pvdrcontrol )
      {
            m_pauimgr->DetachPane( m_pvdrcontrol );
            m_pvdrcontrol->Close();
            m_pvdrcontrol->Destroy();
            m_pvdrcontrol = NULL;
      }

      if ( m_recording )
      {
            m_ostream.Close();
            m_recording = false;
      }

      RemovePlugInTool( m_tb_item_id_record );
      RemovePlugInTool( m_tb_item_id_play );
      return true;
}

int vdr_pi::GetAPIVersionMajor()
{
      return MY_API_VERSION_MAJOR;
}

int vdr_pi::GetAPIVersionMinor()
{
      return MY_API_VERSION_MINOR;
}

int vdr_pi::GetPlugInVersionMajor()
{
      return PLUGIN_VERSION_MAJOR;
}

int vdr_pi::GetPlugInVersionMinor()
{
      return PLUGIN_VERSION_MINOR;
}

wxBitmap *vdr_pi::GetPlugInBitmap()
{
      return _img_vdr_pi;
}

wxString vdr_pi::GetCommonName()
{
      return _("VDR");
}


wxString vdr_pi::GetShortDescription()
{
      return _("Voyage Data Recorder plugin for OpenCPN");
}

wxString vdr_pi::GetLongDescription()
{
      return _("Voyage Data Recorder plugin for OpenCPN\n\
Provides NMEA stream save and replay.");

}

void vdr_pi::SetNMEASentence(wxString &sentence)
{
      if (m_recording)
            m_ostream.Write(sentence);
}

void vdr_pi::SetAISSentence(wxString &sentence)
{
      if (m_recording)
            m_ostream.Write(sentence);
}

void vdr_pi::Notify()
{
      wxString str;
      int pos = m_istream.GetCurrentLine();

      if (m_istream.Eof() || pos==-1)
            str = m_istream.GetFirstLine();
      else
            str = m_istream.GetNextLine();

      PushNMEABuffer(str+_T("\r\n"));

      if ( m_pvdrcontrol )
            m_pvdrcontrol->SetProgress(pos);
}

void vdr_pi::SetInterval( int interval )
{
      m_interval = interval;
      if ( IsRunning() ) // Timer started?
            Start( m_interval, wxTIMER_CONTINUOUS ); // restart timer with new interval
}

int vdr_pi::GetToolbarToolCount(void)
{
      return 2;
}

void vdr_pi::OnToolbarToolCallback(int id)
{
      if ( id == m_tb_item_id_play )
      {
            if ( IsRunning() ) // Timer started?
            {
                  Stop(); // Stop timer
                  m_istream.Close();

                  if ( m_pvdrcontrol )
                  {
                        m_pauimgr->DetachPane( m_pvdrcontrol );
                        m_pvdrcontrol->Close();
                        m_pvdrcontrol->Destroy();
                        m_pvdrcontrol = NULL;
                  }
                  SetToolbarItemState( id, false );
            }
            else
            {
                  wxFileDialog fdlg( GetOCPNCanvasWindow(), _("Choose a file"), wxT(""), m_ifilename, wxT("*.*"), wxFD_OPEN|wxFD_FILE_MUST_EXIST );
                  if ( fdlg.ShowModal() != wxID_OK)
                  {
                        SetToolbarItemState( id, false );
                        return;
                  }
                  m_ifilename.Clear();
                  m_ifilename = fdlg.GetPath();

                  m_istream.Open( m_ifilename );
                  Start( m_interval, wxTIMER_CONTINUOUS ); // start timer
      
                  if (! m_pvdrcontrol )
                  {
                        m_pvdrcontrol = new VDRControl( GetOCPNCanvasWindow(), wxID_ANY, this, 1000/m_interval, m_istream.GetLineCount() );
                        wxAuiPaneInfo pane = wxAuiPaneInfo().Name(_T("VDR")).Caption(_("VDR replay")).CaptionVisible(true).Float().FloatingPosition(50,100).Dockable(false).Fixed().CloseButton(false).Show(true);
                        m_pauimgr->AddPane( m_pvdrcontrol, pane );
                        m_pauimgr->Update();
                  }

                  SetToolbarItemState( id, true );
            }
      }
      else if ( id == m_tb_item_id_record )
      {
            if ( m_recording )
            {
                  m_ostream.Close();
                  m_recording = false;

                  SetToolbarItemState( id, false );
            }
            else
            {
                  wxFileDialog fdlg( GetOCPNCanvasWindow(), _("Choose a file"), wxT(""), m_ofilename, wxT("*.*"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT );
                  if ( fdlg.ShowModal() != wxID_OK)
                  {
                        SetToolbarItemState( id, false );
                        return;
                  }
                  m_ofilename.Clear();
                  m_ofilename = fdlg.GetPath();

                  //m_ostream.Open( m_ofilename, wxFile::write_append );
                  m_ostream.Open( m_ofilename, wxFile::write );
                  m_recording = true;

                  SetToolbarItemState( id, true );
            }
      }
}

void vdr_pi::SetColorScheme(PI_ColorScheme cs)
{
      if ( m_pvdrcontrol )
      {
            m_pvdrcontrol->SetColorScheme( cs );
      }
}

bool vdr_pi::LoadConfig(void)
{
      wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

      if(pConf)
      {
            pConf->SetPath( _T("/PlugIns/VDR") );

            pConf->Read( _T("InputFilename"), &m_ifilename, wxEmptyString );
            pConf->Read( _T("OutputFilename"), &m_ofilename, wxEmptyString );
            pConf->Read( _T("Interval"), &m_interval, 1 );

            return true;
      }
      else
            return false;
}

bool vdr_pi::SaveConfig(void)
{
      wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

      if(pConf)
      {
            pConf->SetPath( _T("/PlugIns/VDR") );

            pConf->Write( _T("InputFilename"), m_ifilename );
            pConf->Write( _T("OutputFilename"), m_ofilename );
            pConf->Write( _T("Interval"), m_interval );

            return true;
      }
      else
            return false;
}

//----------------------------------------------------------------
//
//    VDR replay control Implementation
//
//----------------------------------------------------------------

VDRControl::VDRControl( wxWindow *pparent, wxWindowID id, vdr_pi *vdr, int speed, int range )
      :wxWindow( pparent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE, _T("Dashboard") ), m_pvdr(vdr)
{
      wxColour cl;
      GetGlobalColor(_T("DILG1"), &cl);
      SetBackgroundColour(cl);

      wxFlexGridSizer *topsizer = new wxFlexGridSizer(2);
      topsizer->AddGrowableCol(1);

      wxStaticText *itemStaticText01 = new wxStaticText( this, wxID_ANY, _("Speed:") );
      topsizer->Add( itemStaticText01, 0, wxEXPAND|wxALL, 2 );

      m_pslider = new wxSlider( this, wxID_ANY, speed, 1, 100, wxDefaultPosition, wxSize( 200, 20) );
      topsizer->Add( m_pslider, 1, wxALL|wxEXPAND, 2 );
      m_pslider->Connect( wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(VDRControl::OnSliderUpdated), NULL, this);

      wxStaticText *itemStaticText02 = new wxStaticText( this, wxID_ANY, _("Progress:") );
      topsizer->Add( itemStaticText02, 0, wxEXPAND|wxALL, 2 );

      m_pgauge = new wxGauge( this, wxID_ANY, range, wxDefaultPosition, wxDefaultSize, wxGA_HORIZONTAL );
      topsizer->Add( m_pgauge, 1, wxALL|wxEXPAND, 2 );

      SetSizer( topsizer );
      topsizer->Fit( this );
      Layout();
}

void VDRControl::SetColorScheme( PI_ColorScheme cs )
{
      wxColour cl;
      GetGlobalColor( _T("DILG1"), &cl );
      SetBackgroundColour( cl );

      Refresh(false);
}

void VDRControl::SetProgress( int progress )
{
      m_pgauge->SetValue( progress );
}

void VDRControl::OnSliderUpdated( wxCommandEvent& event )
{
      m_pvdr->SetInterval( 1000/m_pslider->GetValue() );
}

