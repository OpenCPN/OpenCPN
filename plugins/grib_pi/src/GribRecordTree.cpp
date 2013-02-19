/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Object
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
 *
 */

#include "GribUIDialog.h"

//---------------------------------------------------------------------------------------
//          GRIB File/Record selector Tree Control Implementation
//---------------------------------------------------------------------------------------
IMPLEMENT_CLASS ( GribRecordTree, wxTreeCtrl )

// GribRecordTree event table definition

BEGIN_EVENT_TABLE ( GribRecordTree, wxTreeCtrl )
EVT_TREE_SEL_CHANGED ( ID_GRIBRECORDTREE, GribRecordTree::OnItemSelectChange )
END_EVENT_TABLE()

GribRecordTree::GribRecordTree( GRIBUIDialogBase* parent, wxWindowID id, const wxPoint& pos,
                                const wxSize& size, long style )
{
    Init();
    m_parent = parent;
    Create( parent, id, pos, size, style );
}

GribRecordTree::~GribRecordTree()
{
    delete m_file_id_array;
}

void GribRecordTree::Init()
{
    m_file_id_array = NULL;
}

void GribRecordTree::OnItemSelectChange( wxTreeEvent& event )
{
    GRIBUIDialog *parent = dynamic_cast<GRIBUIDialog*>(m_parent);
    if(!parent) return;

    /* set timeline to not be working */
    parent->m_tPlayStop.Stop();
    parent->m_tbPlayStop->SetValue(0);
    parent->m_tbPlayStop->SetLabel(_("Play"));
    parent->m_sTimeline->Disable();

    GribTreeItemData *pdata = (GribTreeItemData *) GetItemData( event.GetItem() );
    if( !pdata ) return;

    switch( pdata->m_type ){
        case GRIB_FILE_TYPE:
        {
            //    Create and ingest the GRIB file object if needed
            if( !pdata->m_pGribFile ) {
                GRIBFile *pgribfile = new GRIBFile( pdata->m_file_name );
                if( pgribfile ) {
                    if( pgribfile->IsOK() ) {
                        pdata->m_pGribFile = pgribfile;
                        parent->PopulateTreeControlGRS( pgribfile, pdata->m_file_index );
                    } else {
                        wxLogMessage( pgribfile->GetLastErrorMessage() );
                    }
                }
            }

            if( pdata->m_pGribFile ) {
                parent->SelectTreeControlGRS( pdata->m_pGribFile );
                ArrayOfGribRecordSets *rsa = pdata->m_pGribFile->GetRecordSetArrayPtr();
                if( rsa->GetCount() > 0)
                    parent->SelectGribRecordSet( &rsa->Item(0) );
            }
            break;
        }

        case GRIB_RECORD_SET_TYPE:
        {
            parent->SelectTreeControlGRS( pdata->m_pGribFile );
            parent->SelectGribRecordSet( pdata->m_pGribRecordSet );
            break;
        }
    }
}

//---------------------------------------------------------------------------------------
//          GRIB Tree Item Data Implementation
//---------------------------------------------------------------------------------------

GribTreeItemData::GribTreeItemData( const GribTreeItemType type )
{
    m_pGribFile = NULL;
    m_pGribRecordSet = NULL;

    m_type = type;
}

GribTreeItemData::~GribTreeItemData()
{
    if(m_type == GRIB_FILE_TYPE)
        delete m_pGribFile;
}

