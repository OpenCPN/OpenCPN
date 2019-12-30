/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2019 Alec Leamas                                        *
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


#ifndef DOWNLOAD_MGR_H__
#define DOWNLOAD_MGR_H__

#include <wx/dialog.h>
#include <wx/window.h>

// Accepted by PluginDownloadDialog, reloads plugin list.
wxDECLARE_EVENT(EVT_PLUGINS_RELOAD, wxCommandEvent);

class PluginDownloadDialog: public wxDialog
{
    public:
        PluginDownloadDialog(wxWindow* parent);

        wxWindow* GetRealParent() { return  m_parent; }

};

#endif // DOWNLOAD_MGR_H__
