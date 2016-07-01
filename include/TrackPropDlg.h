/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Track Properties Dialog
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#ifndef __TRACKPROPDLG_H__
#define __TRACKPROPDLG_H__

#include <wx/clipbrd.h>
#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/statbox.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/hyperlink.h>
#include <wx/scrolwin.h>
#include <wx/button.h>
#include <wx/tglbtn.h>
#include <wx/notebook.h>

#if wxCHECK_VERSION(2, 9, 0)
#include <wx/dialog.h>
#else
#include "scrollingdialog.h"
#endif

#include "Track.h"
#include "LinkPropDlg.h"

#define ID_RCLK_MENU_COPY_TEXT 7014

/*!
 * Forward declarations
 */

class   wxListCtrl;
class   OCPNTrackListCtrl;
class   Track;
class   TrackPoint;
class   HyperlinkList;

///////////////////////////////////////////////////////////////////////////////
/// Class TrackPropDlg
///////////////////////////////////////////////////////////////////////////////
class TrackPropDlg : public wxDialog 
{
private:
        static bool instanceFlag;
        static TrackPropDlg *single;
        TrackPropDlg( wxWindow* parent, wxWindowID id, const wxString& title,
                      const wxPoint& pos, const wxSize& size,
                      long style ); 
        
        TrackPoint *m_pExtendPoint;
        Track      *m_pExtendTrack;
        TrackPoint *m_pEntrackPoint;
        bool        m_bStartNow;

        double      m_planspeed;
        double      m_avgspeed;

        int         m_nSelected; // index of point selected in Properties dialog row
        
        bool        IsThisTrackExtendable();
        bool        SaveChanges(void);
        
        HyperlinkList   *m_pMyLinkList;
        LinkPropImpl    *m_pLinkProp;
        void OnHyperLinkClick(wxHyperlinkEvent &event);
        wxHyperlinkCtrl *m_pEditedLink;

    protected:
        wxNotebook* m_notebook1;
        wxScrolledWindow* m_panelBasic;
        wxStaticText* m_stName;
        wxTextCtrl* m_tName;
        wxStaticText* m_stFrom;
        wxTextCtrl* m_tFrom;
        wxStaticText* m_stTo;
        wxTextCtrl* m_tTo;
        wxCheckBox* m_cbShow;
        wxStaticText* m_stColor;
        wxChoice* m_cColor;
        wxStaticText* m_stStyle;
        wxChoice* m_cStyle;
        wxStaticText* m_stWidth;
        wxChoice* m_cWidth;
        wxStaticText* m_stTotDistance;
        wxTextCtrl* m_tTotDistance;
        wxStaticText* m_stAvgSpeed;
        wxTextCtrl* m_tAvgSpeed;
        wxStaticText* m_stTimeEnroute;
        wxTextCtrl* m_tTimeEnroute;
        wxStaticText* m_stShowTime;
        wxRadioButton* m_rbShowTimeUTC;
        wxRadioButton* m_rbShowTimePC;
        wxRadioButton* m_rbShowTimeLocal;
        OCPNTrackListCtrl *m_lcPoints;
        wxScrolledWindow* m_panelAdvanced;
        wxStaticText* m_stDescription;
        wxTextCtrl* m_tDescription;
        wxScrolledWindow* m_scrolledWindowLinks;
        wxHyperlinkCtrl* m_hyperlink1;
        wxMenu* m_menuLink;
        wxButton* m_buttonAddLink;
        wxToggleButton* m_toggleBtnEdit;
        wxStaticText* m_staticTextEditEnabled;
        wxStdDialogButtonSizer* m_sdbBtmBtnsSizer;
        wxButton* m_sdbBtmBtnsSizerOK;
        wxButton* m_sdbBtmBtnsSizerCancel;
        wxStaticBoxSizer* sbSizerLinks;
        wxBoxSizer* bSizerLinks;
    
        wxButton* m_sdbBtmBtnsSizerPrint;
        wxButton* m_sdbBtmBtnsSizerSplit;
        wxButton* m_sdbBtmBtnsSizerExtend;
        wxButton* m_sdbBtmBtnsSizerToRoute;
        wxButton* m_sdbBtmBtnsSizerExport;
        
        wxScrolledWindow *itemDialog1;
        bool m_bcompact;
        

        // Virtual event handlers, overide them in your derived class
        void OnCancelBtnClick( wxCommandEvent& event );
        void OnOKBtnClick( wxCommandEvent& event );
        void OnPrintBtnClick( wxCommandEvent& event );
        void OnSplitBtnClick( wxCommandEvent& event );
        void OnExtendBtnClick( wxCommandEvent& event );
        void OnToRouteBtnClick( wxCommandEvent& event );
        void OnExportBtnClick( wxCommandEvent& event );
        void OnTrackPropCopyTxtClick( wxCommandEvent& event );
        void OnTrackPropListClick( wxListEvent& event );
        void OnTrackPropRightClick( wxListEvent &event );
        void OnTrackPropMenuSelected( wxCommandEvent &event );
        void OnDeleteLink( wxCommandEvent& event );
        void OnEditLink( wxCommandEvent& event );
        void OnAddLink( wxCommandEvent& event );
        void OnEditLinkToggle( wxCommandEvent& event );
        void OnShowTimeTZ( wxCommandEvent& event );
        void CreateControls( void );
        void CreateControlsCompact( void );
        
public:
        static TrackPropDlg *getInstance( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& title = _("Track Properties"),
                                      const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 680,440 ),
                                      long style = wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxMAXIMIZE_BOX|wxMINIMIZE_BOX|wxRESIZE_BORDER ); 
        static bool getInstanceFlag(){ return instanceFlag; } 
        ~TrackPropDlg();

        void m_hyperlink1OnContextMenu( wxMouseEvent &event )
        {
            m_hyperlink1->PopupMenu( m_menuLink, event.GetPosition() );
        }
        
        void SetTrackAndUpdate( Track *pt );
        bool UpdateProperties();
        void InitializeList();
        Track *GetTrack() { return m_pTrack; }
        
        void RecalculateSize( void );
        
        Track      *m_pTrack;
        
        void m_hyperlinkContextMenu( wxMouseEvent &event );
};

class OCPNTrackListCtrl: public wxListCtrl
{
    public:
        OCPNTrackListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style);
        ~OCPNTrackListCtrl();

        wxString OnGetItemText(long item, long column) const;
        int OnGetItemColumnImage(long item, long column) const;

        Track                   *m_pTrack;
        int                     m_tz_selection;
        int                     m_LMT_Offset;
};

#endif //__TRACKPROPDLG_H__
