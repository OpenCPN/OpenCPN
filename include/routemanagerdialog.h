/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2010, Anders Lund <anders@alweb.dk>
 */

#ifndef _RouteManagerDialog_h_
#define _RouteManagerDialog_h_

#include <wx/dialog.h>
#include <wx/timer.h>
#include <wx/listctrl.h>

class wxButton;
class Route;

class RouteManagerDialog : public wxDialog {
      DECLARE_EVENT_TABLE()

      public:
            RouteManagerDialog(wxWindow *parent);
            ~RouteManagerDialog();
            void UpdateRouteListCtrl();     // Rebuild route list

      private:
            void Create();
            void UpdateButtons();           // Correct button state
            void MakeAllRoutesInvisible();  // Mark all routes as invisible. Does not flush settings.
            void ZoomtoRoute(Route *route); // Attempt to zoom route into the view

            // event handlers
            void OnDeleteClick(wxCommandEvent &event);
            void OnPropertiesClick(wxCommandEvent &event);
            void OnZoomtoClick(wxCommandEvent &event);
            void OnActivateClick(wxCommandEvent &event);
            void OnReverseClick(wxCommandEvent &event);
            void OnImportClick(wxCommandEvent &event);
            void OnExportClick(wxCommandEvent &event);
            void OnRouteListClickUp(wxMouseEvent &event);
            void OnRouteListLeftDown(wxMouseEvent &event);
            void OnTimeOut(wxTimerEvent &event);
            void OnBtnLeftDown(wxMouseEvent &event); // record control key state for some action buttons
            void OnDeleteAllRoutesClick(wxCommandEvent &event);
            void OnDeleteAllTracksClick(wxCommandEvent &event);

            void OnRouteSelected(wxListEvent &event);

            // properties
            wxListCtrl *m_pRouteListCtrl;

            wxButton *btnDelete;
            wxButton *btnZoomto;
            wxButton *btnProperties;
            wxButton *btnActivate;
            wxButton *btnImport;
            wxButton *btnExport;
            wxButton *btnReverse;
            wxButton *btnDeleteAllRoutes;
            wxButton *btnDeleteAllTracks;

            wxTimer *m_Timer;         // for mouse click detection
            bool m_bPossibleClick;    // do
            bool m_bCtrlDown;         // record control key state for some action buttons
            bool m_bNeedConfigFlush;  // if true, update config in destructor

};

#endif // _RouteManagerDialog_h_
// kate: indent-width 6; indent-mode cstyle; space-indent on;
