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
 **************************************************************************/

#ifndef __AISTARGETQUERYDIALOG_H__
#define __AISTARGETQUERYDIALOG_H__

#include <wx/dialog.h>
#include <wx/sizer.h>

#include "ocpn_types.h"

class wxHtmlWindow;
class AIS_Target_Data;

class AISTargetQueryDialog: public wxDialog
{
DECLARE_CLASS( AISTargetQueryDialog )
DECLARE_EVENT_TABLE()
public:

      /// Constructors

      AISTargetQueryDialog( );
      AISTargetQueryDialog( wxWindow* parent,
            wxWindowID id = wxID_ANY,
            const wxString& caption = _("Object Query"),
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

      ~AISTargetQueryDialog( );
      /// Initialise our variables
      void Init();

      /// Creation
      bool Create( wxWindow* parent,
            wxWindowID id = wxID_ANY,
            const wxString& caption = _("Object Query"),
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

      void OnClose(wxCloseEvent& event);
      void OnIdOKClick( wxCommandEvent& event );
      void OnIdWptCreateClick( wxCommandEvent& event );
      void OnIdTrkCreateClick( wxCommandEvent& event );
      void OnMove( wxMoveEvent& event );
      void AdjustBestSize(AIS_Target_Data *td);
      void CreateControls();
      void RenderHTMLQuery(AIS_Target_Data *td);
      
      void SetText(const wxString &text_string);
      void SetColorScheme(ColorScheme cs);

      void RecalculateSize( void );
      void SetAutoCentre( bool bval ){ m_bautoCentre = bval;}
      void SetAutoSize( bool bval ){ m_bautosize = bval;}
      
      void UpdateText(void);
      void SetMMSI(int mmsi){ m_MMSI = mmsi; }
      int  GetMMSI(void){ return m_MMSI; }

      //    Data
      int               m_MMSI;
      wxHtmlWindow     *m_pQueryTextCtl;
      ColorScheme       m_colorscheme;
      wxBoxSizer        *m_pboxSizer;
      int               m_nl;
      wxButton          *m_okButton;
      wxButton          *m_createWptBtn;
      wxButton          *m_createTrkBtn;
      bool              m_bsize_set;
      int               m_adjustedFontSize;
      int               m_control_font_size;
      wxFont            *m_basefont;
      wxWindow          *m_parent;
      bool              m_bautoCentre;
      bool              m_bautosize;
};

#endif
