/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  About Dialog
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
 *
 * $Log: about.h,v $
 * Revision 1.5  2010/04/27 01:44:36  bdbcat
 * Build 426
 *
 * Revision 1.4  2009/06/28 02:02:56  bdbcat
 * Implement "Tips" tab.
 *
 * Revision 1.3  2007/05/03 13:31:19  dsr
 * Major refactor for 1.2.0
 *
 * Revision 1.2  2006/10/07 03:50:54  dsr
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2006/08/21 05:52:11  dsr
 * Initial import as opencpn, GNU Automake compliant.
 *
 * Revision 1.2  2006/04/23 04:04:28  dsr
 * Fix internal names
 *
 * Revision 1.1.1.1  2006/04/19 03:23:27  dsr
 * Rename/Import to OpenCPN
 *
 * Revision 1.2  2006/03/16 03:28:12  dsr
 * Cleanup tabs
 *
 * Revision 1.1  2006/02/23 03:39:19  dsr
 * Initial
 *
 *
 *
 */


#ifndef _ABOUT_H_
#define _ABOUT_H_

#include "wx/notebook.h"

//      Constants

#define ID_DIALOG 10001

#define SYMBOL_ABOUT_TITLE _("About OpenCPN")

#define xID_OK          10009
#define ID_DONATE       10010

#define ID_NOTEBOOK_HELP 10002

class wxHtmlWindow;

class about: public wxDialog
{
  DECLARE_DYNAMIC_CLASS( about )
      DECLARE_EVENT_TABLE()

  public:
    about( );
    about( wxWindow* parent, wxString *pLicense_Data_Locn,
           wxWindowID id = ID_DIALOG,
           const wxString& caption = SYMBOL_ABOUT_TITLE,
           const wxPoint& pos = wxDefaultPosition,
           const wxSize& size = wxSize(500, 500),
           long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX);

   bool Create( wxWindow* parent,
                wxWindowID id = ID_DIALOG,
                 const wxString& caption = SYMBOL_ABOUT_TITLE,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxSize(500, 500),
                 long style = wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX );

    void CreateControls();
    void OnXidOkClick( wxCommandEvent& event );
    void OnPageChange(wxNotebookEvent& event);
    void OnDonateClick( wxCommandEvent& event );
    void Update();

    wxString          *m_pDataLocn;
    wxWindow          *m_parent;
    wxHtmlWindow      *m_ptips_window;
    bool              m_btips_loaded;

    wxPanel* itemPanelAbout;
    wxPanel* itemPanelAuthors;
    wxPanel* itemPanelLicense;
    wxPanel* itemPanelTips;

    wxTextCtrl *pAboutTextCtl;
    wxTextCtrl *pAuthorTextCtl;
    wxTextCtrl *pLicenseTextCtl;

};

#endif    // _ABOUT_H_

