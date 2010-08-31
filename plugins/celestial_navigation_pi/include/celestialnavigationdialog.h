/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
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
    Copyright (C) 2010, Sean D'Epagnier <geckosenator@gmail.com>
 */

#ifndef _CelestialNavigationDialog_h_
#define _CelestialNavigationDialog_h_

#include <wx/dialog.h>
#include <wx/timer.h>
#include <wx/listctrl.h>
#include <wx/spinctrl.h>

class wxButton;

class CelestialNavigationDialog : public wxDialog {
      DECLARE_EVENT_TABLE()

      public:
            CelestialNavigationDialog(wxWindow *parent);
            ~CelestialNavigationDialog();

            SightList m_SightList;

      private:
            void UpdateSightListCtrl();     // Rebuild sight list
            void MakeAllSightsInvisible();
            void UpdateButtons();           // Correct button state
            void MakeAllGreatCirclesInvisible();  // Mark all sights as invisible. Does not flush settings.

            // event handlers
            void OnNewClick(wxCommandEvent &event);
            void OnPropertiesClick(wxCommandEvent &event);
            void OnDeleteClick(wxCommandEvent &event);
            void OnSightListLeftDown(wxMouseEvent &event);
            void OnBtnLeftDown(wxMouseEvent &event); // record control key state for some action buttons
            void OnDeleteAllSightsClick(wxCommandEvent &event);

            void OnSightSelected(wxListEvent &event);

            void OnUpdateAllSightsTimeOffset(wxSpinEvent &event);

            // properties
            wxListCtrl *m_pSightListCtrl;

            wxButton *btnNew;
            wxButton *btnProperties;
            wxButton *btnDelete;
            wxButton *btnDeleteAllSights;

            wxSpinCtrl *spinSightTimeOffset;

            bool m_bNeedConfigFlush;  // if true, update config in destructor
};

#endif // _CelestialNavigationDialog_h_
// kate: indent-width 6; indent-mode cstyle; space-indent on;
