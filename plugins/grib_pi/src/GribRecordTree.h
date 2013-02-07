/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Plugin Freinds
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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

#include <wx/treectrl.h>

//----------------------------------------------------------------------------------------------------------
//    Grib File/Record selector Tree Control Specification
//----------------------------------------------------------------------------------------------------------

class GRIBUIDialogBase;
class GRIBUIDialog;
class GRIBFile;
class GribRecordSet;

enum GribTreeItemType { GRIB_FILE_TYPE, GRIB_RECORD_SET_TYPE };

class GribRecordTree: public wxTreeCtrl {
DECLARE_CLASS( GribRecordTree )DECLARE_EVENT_TABLE()
public:
    // Constructors
    GribRecordTree( GRIBUIDialogBase* parent, wxWindowID id = wxID_ANY, const wxPoint& pos =
                    wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTR_HAS_BUTTONS );

    ~GribRecordTree();

    void Init();

    //  Override events
    void OnItemExpanding( wxTreeEvent& event );
    void OnItemSelectChange( wxTreeEvent& event );

    //    Data
    GRIBUIDialogBase *m_parent;
    wxTreeItemId *m_file_id_array;             // an array of wxTreeItemIDs

};

class GribTreeItemData: public wxTreeItemData {
public:
    GribTreeItemData( const GribTreeItemType type );
    ~GribTreeItemData();

    GribTreeItemType m_type;

    //    Data for type GRIB_FILE
    wxString m_file_name;
    GRIBFile *m_pGribFile;
    int m_file_index;

    //    Data for type GRIB_RECORD
    GribRecordSet *m_pGribRecordSet;

};
