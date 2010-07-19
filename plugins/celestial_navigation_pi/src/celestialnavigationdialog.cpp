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

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/listctrl.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/imaglist.h>

#include <iostream>

#include "celestialnavigationdialog.h"
#include "sightdialog.h"
#include "sight.h"
//#include "navutil.h"
//#include "routeprop.h"
#include "georef.h"
//#include "chartimg.h"               // for ChartBaseBSB

#define DIALOG_MARGIN 3

/* XPM */
static const char *eye[]={
"20 20 7 1",
". c none",
"# c #000000",
"a c #333333",
"b c #666666",
"c c #999999",
"d c #cccccc",
"e c #ffffff",
"....................",
"....................",
"....................",
"....................",
".......######.......",
".....#aabccb#a#.....",
"....#deeeddeebcb#...",
"..#aeeeec##aceaec#..",
".#bedaeee####dbcec#.",
"#aeedbdabc###bcceea#",
".#bedad######abcec#.",
"..#be#d######dadb#..",
"...#abac####abba#...",
".....##acbaca##.....",
".......######.......",
"....................",
"....................",
"....................",
"....................",
"...................."};

enum { rmVISIBLE = 0, rmBODY, rmTIME, rmELEVATION, rmAZIMUTH, rmHEIGHT_CORRECTION, rmEDGE_CORRECTION, rmREFRACTION_CORRECTION, rmPARALLAX_CORRECTION, rmCORRECTED_ELEVATION, rmCOLOR };// RMColumns;

// GLOBALS :0
extern SightList *pSightList;
extern ChartCanvas *cc1;
extern ChartBase *Current_Ch;

// sort callback. Sort by body.
int wxCALLBACK SortSights(long item1, long item2, long list)
{
      wxListCtrl *lc = (wxListCtrl*)list;

      wxListItem it1, it2;
      it1.SetId(lc->FindItem(-1, item1));
      it1.SetColumn(1);

      it2.SetId(lc->FindItem(-1, item2));
      it2.SetColumn(1);

      lc->GetItem(it1);
      lc->GetItem(it2);

      return it1.GetText().Cmp(it2.GetText());
}


// event table. Empty, because I find it much easier to see what is connected to what
// using Connect() where possible, so that it is visible in the code.
BEGIN_EVENT_TABLE(CelestialNavigationDialog, wxDialog)
END_EVENT_TABLE()

// implementation
CelestialNavigationDialog::CelestialNavigationDialog(wxWindow *parent)
      : wxDialog(parent, -1, wxString(_("Celestial Navigation Manager")), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
      // Setup GUI
      wxBoxSizer *layout = new wxBoxSizer(wxVERTICAL);
      SetSizer(layout);

      wxBoxSizer *sbsSights = new wxBoxSizer(wxHORIZONTAL);
      layout->Add(sbsSights, 1, wxEXPAND|wxALL, DIALOG_MARGIN);

      m_pSightListCtrl = new wxListCtrl(this, -1, wxDefaultPosition, wxSize(400, -1),
          wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_SORT_ASCENDING|wxLC_HRULES/*|wxLC_VRULES*/|wxBORDER_SUNKEN);
      sbsSights->Add(m_pSightListCtrl, 1, wxEXPAND|wxALL, DIALOG_MARGIN);
      m_pSightListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler(CelestialNavigationDialog::OnSightSelected), NULL, this);
      m_pSightListCtrl->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(CelestialNavigationDialog::OnSightListLeftDown), NULL, this);

      // Columns: visibility ctrl, name
      // note that under MSW for SetColumnWidth() to work we need to create the
      // items with images initially even if we specify dummy image id

      m_pSightListCtrl->InsertColumn(rmVISIBLE, wxT(""));
      m_pSightListCtrl->SetColumnWidth(0, 28);

     m_pSightListCtrl->InsertColumn(rmBODY, _("Body"));
      m_pSightListCtrl->InsertColumn(rmTIME, _("Time"));
      m_pSightListCtrl->InsertColumn(rmELEVATION, _("Elevation"));
      m_pSightListCtrl->InsertColumn(rmAZIMUTH, _("Azimuth"));
      m_pSightListCtrl->InsertColumn(rmHEIGHT_CORRECTION, _("Height Correction"));
      m_pSightListCtrl->InsertColumn(rmEDGE_CORRECTION, _("Limb Correction"));
      m_pSightListCtrl->InsertColumn(rmREFRACTION_CORRECTION, _("Refraction Correction"));
      m_pSightListCtrl->InsertColumn(rmPARALLAX_CORRECTION, _("Parallax Correction"));
      m_pSightListCtrl->InsertColumn(rmCORRECTED_ELEVATION, _("Corrected Elevation"));
      m_pSightListCtrl->InsertColumn(rmCOLOR, _("Color"));

      // Buttons
      wxBoxSizer *bsSightButtons = new wxBoxSizer(wxVERTICAL);
      sbsSights->Add(bsSightButtons, 0, wxALIGN_RIGHT);

      btnNew = new wxButton(this, -1, _("&New"));
      bsSightButtons->Add(btnNew, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnNew->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(CelestialNavigationDialog::OnNewClick), NULL, this);

      btnProperties = new wxButton(this, -1, _("&Properties"));
      bsSightButtons->Add(btnProperties, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnProperties->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(CelestialNavigationDialog::OnPropertiesClick), NULL, this);

      btnDelete = new wxButton(this, -1, _("&Delete"));
      bsSightButtons->Add(btnDelete, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnDelete->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(CelestialNavigationDialog::OnDeleteClick), NULL, this);

      btnDeleteAllSights = new wxButton(this, -1, _("&Delete All Sights..."));
      bsSightButtons->Add(btnDeleteAllSights, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnDeleteAllSights->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(CelestialNavigationDialog::OnDeleteAllSightsClick), NULL, this);


      // Dialog buttons
      wxSizer *szButtons = CreateButtonSizer(wxOK);
      layout->Add(szButtons, 0, wxALL|wxALIGN_RIGHT, DIALOG_MARGIN);

      Fit();

      SetMinSize(GetBestSize());

      // create a image list for the list with just the eye icon
      wxImageList *imglist = new wxImageList(20, 20, true, 1);
      imglist->Add(wxBitmap(eye));
      m_pSightListCtrl->AssignImageList(imglist, wxIMAGE_LIST_SMALL);

      UpdateSightListCtrl();
}

CelestialNavigationDialog::~CelestialNavigationDialog()
{
}

void CelestialNavigationDialog::UpdateSightListCtrl()
{
      // if an item was selected, make it selected again if it still exist
      long selected_index = m_pSightListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      long selected_id = -1;
      if (selected_index > -1)
            selected_id = m_pSightListCtrl->GetItemData(selected_index);

      // Delete existing items
      m_pSightListCtrl->DeleteAllItems();

      // then add sights to the listctrl
      SightList::iterator it;
      wxListItem item;
      int index = 0;

      for (it = (*pSightList).begin(); it != (*pSightList).end(); ++it, ++index)
      {
            item.SetId(index);
            item.SetImage((*it)->IsVisible() ? 0 : -1);
            item.SetData(index);

            long idx = m_pSightListCtrl->InsertItem(item);
            m_pSightListCtrl->SetItemImage(index, (*it)->IsVisible() ? 0 : -1);
            m_pSightListCtrl->SetItem(idx, rmBODY, (*it)->m_Body);
            m_pSightListCtrl->SetItem(idx, rmTIME, (*it)->m_DateTime.Format());
            m_pSightListCtrl->SetItem(idx, rmELEVATION, wxString::Format(_T("%f"), (*it)->m_Elevation));
            m_pSightListCtrl->SetItem(idx, rmAZIMUTH, wxString::Format(_T("%f"), (*it)->m_Azimuth));
            m_pSightListCtrl->SetItem(idx, rmHEIGHT_CORRECTION,
                                      wxString::Format(_T("%f"), (*it)->m_HeightCorrection));
            m_pSightListCtrl->SetItem(idx, rmEDGE_CORRECTION,
                                      wxString::Format(_T("%f"), (*it)->m_LimbCorrection));
            m_pSightListCtrl->SetItem(idx, rmREFRACTION_CORRECTION,
                                      wxString::Format(_T("%f"), (*it)->m_RefractionCorrection));
            m_pSightListCtrl->SetItem(idx, rmPARALLAX_CORRECTION,
                                      wxString::Format(_T("%f"), (*it)->m_ParallaxCorrection));
            m_pSightListCtrl->SetItem(idx, rmCORRECTED_ELEVATION,
                                      wxString::Format(_T("%f"), (*it)->m_CorrectedElevation));
            m_pSightListCtrl->SetItem(idx, rmCOLOR,
                                      wxString::Format(_T("0x%llx%llx%llx"),
                                                       (*it)->m_Colour.Red(),
                                                       (*it)->m_Colour.Green(),
                                                       (*it)->m_Colour.Blue()));
      }

      m_pSightListCtrl->SetColumnWidth(1, wxLIST_AUTOSIZE);
      m_pSightListCtrl->SetColumnWidth(2, wxLIST_AUTOSIZE);

      if(m_pSightListCtrl->GetColumnWidth(1) < 20)
            m_pSightListCtrl->SetColumnWidth(1, 50);

      if(m_pSightListCtrl->GetColumnWidth(2) < 20)
            m_pSightListCtrl->SetColumnWidth(2, 50);

      m_pSightListCtrl->SortItems(SortSights, (long)m_pSightListCtrl);

      // restore selection if possible
      // NOTE this will select a different item, if one is deleted
      // (the next sight will get that index).
      if (selected_id > -1)
      {
            selected_index = m_pSightListCtrl->FindItem(-1, selected_id);
            m_pSightListCtrl->SetItemState(selected_index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
      }

      UpdateButtons();
}

void CelestialNavigationDialog::UpdateButtons()
{
      // enable/disable buttons
      long selected_index_index = m_pSightListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      bool enable = !(selected_index_index < 0);

      btnProperties->Enable(enable);
      btnDelete->Enable(enable);
      btnDeleteAllSights->Enable(pSightList->GetCount());
}

void CelestialNavigationDialog::MakeAllSightsInvisible()
{
      SightList::iterator it;
      long index = 0;
      for (it = pSightList->begin(); it != pSightList->end(); ++it, ++index)
      {
            if ((*it)->IsVisible()) { // avoid config updating as much as possible!
                  (*it)->SetVisible(false);
                  m_pSightListCtrl->SetItemImage(m_pSightListCtrl->FindItem(-1, index), -1); // Likely not same rder :0
            }
      }
}

//BEGIN Event handlers
void CelestialNavigationDialog::OnNewClick(wxCommandEvent &event)
{
      SightDialog *dialog = new SightDialog(GetParent());
      
      if( dialog->ShowModal() == wxID_OK ) {
         Sight *s = dialog->MakeNewSight();
         pSightList->Append(s);

         UpdateSightListCtrl();

         m_bNeedConfigFlush = true;
      }
}

void CelestialNavigationDialog::OnPropertiesClick(wxCommandEvent &event)
{
      // Delete selected_index sight/track
      long selected_index = m_pSightListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (selected_index < 0) return;

      Sight *psight = pSightList->Item(m_pSightListCtrl->GetItemData(selected_index))->GetData();

      SightDialog *dialog = new SightDialog(GetParent());
      dialog->ReadSight(psight);
      
      if( dialog->ShowModal() == wxID_OK ) {
         pSightList->DeleteObject(psight);

         Sight *s = dialog->MakeNewSight();
         pSightList->Insert(selected_index, s);

         UpdateSightListCtrl();

         m_bNeedConfigFlush = true;
      }
}

void CelestialNavigationDialog::OnDeleteClick(wxCommandEvent &event)
{
      // Delete selected_index sight/track
      long selected_index = m_pSightListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (selected_index < 0) return;

      Sight *psight_to_delete = pSightList->Item(m_pSightListCtrl->GetItemData(selected_index))->GetData();
      pSightList->DeleteObject(psight_to_delete);

      UpdateSightListCtrl();
      cc1->Refresh();

      m_bNeedConfigFlush = true;
}
 
void CelestialNavigationDialog::OnDeleteAllSightsClick(wxCommandEvent &event)
{
   wxMessageDialog mdlg(this, _("Are you sure you want to delete <ALL> sights?"), wxString(_("OpenCPN Alert"),wxYES_NO  ));

      pSightList->Clear();

      UpdateSightListCtrl();
      cc1->Refresh();
}

void CelestialNavigationDialog::OnSightListLeftDown(wxMouseEvent &event)
{
      wxPoint pos = event.GetPosition();
      int flags = 0;
      long clicked_index = m_pSightListCtrl->HitTest(pos, flags);

      //    Clicking Visibility column?
      if (clicked_index > -1 && event.GetX() < m_pSightListCtrl->GetColumnWidth(0))
      {
            // Process the clicked item
            Sight *sight = pSightList->Item(m_pSightListCtrl->GetItemData(clicked_index))->GetData();
            sight->SetVisible(!sight->IsVisible());
            m_pSightListCtrl->SetItemImage(clicked_index, sight->IsVisible() ? 0 : -1);

            cc1->Refresh();
      }

      // Allow wx to process...
      event.Skip();
}

void CelestialNavigationDialog::OnSightSelected(wxListEvent &event)
{
    long clicked_index = event.m_itemIndex;
    // Process the clicked item
    Sight *sight = pSightList->Item(m_pSightListCtrl->GetItemData(clicked_index))->GetData();
    m_pSightListCtrl->SetItemImage(clicked_index, sight->IsVisible() ? 0 : -1);
    cc1->Refresh();

    UpdateButtons();

}

//END Event handlers


// kate: indent-width 6; space-indent on; indent-mode cstyle;

