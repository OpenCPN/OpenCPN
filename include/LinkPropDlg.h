/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Hyperlink properties Support
 * Author:   David Register
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

#ifndef _LINKPROPDLG_H_
#define _LINKPROPDLG_H_

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/scrolwin.h>
#include <wx/button.h>
#include <wx/statbox.h>
#include <wx/filesys.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include "chcanv.h"

#if wxCHECK_VERSION(2, 9, 0)
#include <wx/dialog.h>
#else
#include "scrollingdialog.h"
#endif

///////////////////////////////////////////////////////////////////////////////
/// Class LinkPropDlgDef
///////////////////////////////////////////////////////////////////////////////
class LinkPropDlgDef : public wxDialog
{
	private:

	protected:
		wxStaticText* m_staticTextLinkDesc;
		wxStaticText* m_staticTextLinkUrl;
		wxButton* m_buttonBrowseLocal;
		wxStdDialogButtonSizer* m_sdbSizerButtons;
		wxButton* m_sdbSizerButtonsOK;
		wxButton* m_sdbSizerButtonsCancel;

		// Virtual event handlers, overide them in your derived class
		virtual void OnLocalFileClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnCancelClick( wxCommandEvent& event ) { event.Skip(); }
		virtual void OnOkClick( wxCommandEvent& event ) { event.Skip(); }


	public:
		wxTextCtrl* m_textCtrlLinkDescription;
		wxTextCtrl* m_textCtrlLinkUrl;

		LinkPropDlgDef( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Link Properties"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 468,247 ), long style = wxDEFAULT_DIALOG_STYLE );
		~LinkPropDlgDef();

};


///////////////////////////////////////////////////////////////////////////////
/// Class LinkPropImpl
///////////////////////////////////////////////////////////////////////////////
class LinkPropImpl : public LinkPropDlgDef
{
public :
      LinkPropImpl( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Link Properties"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 468,247 ), long style = wxDEFAULT_DIALOG_STYLE );
private :
      void OnLocalFileClick( wxCommandEvent& event );
      void OnOkClick( wxCommandEvent& event );
};

#endif // _LINKPROPDLG_H_
