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

#ifndef __AISTARGETALERTDIALOG_H__
#define __AISTARGETALERTDIALOG_H__

#include <wx/dialog.h>

#define ID_ACKNOWLEDGE        10001
#define ID_SILENCE            10002
#define ID_JUMPTO             10004
#define ID_WPT_CREATE         10005


class AIS_Decoder;
class wxHtmlWindow;

class OCPN_AlertDialog: public wxDialog
{
    DECLARE_CLASS( OCPN_AlertDialog )
    DECLARE_EVENT_TABLE()
public:
    
    OCPN_AlertDialog( );
    
    virtual ~OCPN_AlertDialog( );
    virtual void Init();
    
    virtual bool Create( wxWindow *parent,
                 wxWindowID id = wxID_ANY,
                 const wxString& caption = _("OpenCPN Alert"),
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );

                 
private:
    wxWindow          *m_pparent;
    
};


class AISTargetAlertDialog: public OCPN_AlertDialog
{
      DECLARE_CLASS( AISTargetAlertDialog )
                  DECLARE_EVENT_TABLE()
      public:

           AISTargetAlertDialog( );

           ~AISTargetAlertDialog( );
           bool Create( int target_mmsi, wxWindow *parent, AIS_Decoder *pdecoder,
                                bool b_jumpto, bool b_createWP, bool b_ack,
                                wxWindowID id = wxID_ANY,
                                const wxString& caption = _("OpenCPN AIS Alert"),
                                const wxPoint& pos = wxDefaultPosition,
                                const wxSize& size = wxDefaultSize,
                                long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU );
           void Init();

           int Get_Dialog_MMSI(void){ return m_target_mmsi; }
           void UpdateText();
           void RecalculateSize( void );
           
      private:
          void CreateControls();
          bool GetAlertText(void);
          void SetColorScheme( void );
            void OnClose(wxCloseEvent& event);
            void OnIdAckClick( wxCommandEvent& event );
            void OnMove( wxMoveEvent& event );
            void OnSize( wxSizeEvent& event );
            void OnIdSilenceClick( wxCommandEvent& event );
            void OnIdJumptoClick( wxCommandEvent& event );
            void OnIdCreateWPClick( wxCommandEvent& event );
            

            wxHtmlWindow      *m_pAlertTextCtl;
            int               m_target_mmsi;
            AIS_Decoder       *m_pdecoder;
            wxFont            *m_pFont;
            wxString          m_alert_text;
            bool              m_bjumpto;
            bool              m_back;
            bool              m_bcreateWP;
            int               m_max_nline;
            int               m_adj_height;
            

};


#endif
