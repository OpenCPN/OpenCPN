/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  S57 SENC File Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2015 by David S. Register                               *
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

#ifndef OSENC_H
#define OSENC_H

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include "s52s57.h"
#include "mygdal/ogr_s57.h"

//      Various error return enums
#define         SENC_NO_ERROR                           0
#define         ERROR_SENCFILE_NOT_FOUND                1
#define         ERROR_SENC_VERSION_MISMATCH             2
#define         ERROR_CANNOT_CREATE_SENC_DIR            3
#define         ERROR_CANNOT_CREATE_TEMP_SENC_FILE      4
#define         ERROR_INGESTING000                      5
#define         ERROR_REGISTRAR_NOT_SET                 6
#define         ERROR_BASEFILE_ATTRIBUTES               7
#define         ERROR_SENCFILE_ABORT                    8



//      Fwd Definitions
class wxProgressDialog;

//--------------------------------------------------------------------------
//      Osenc definition
//--------------------------------------------------------------------------

class Osenc
{
public:
    Osenc();
    ~Osenc();

    wxString getLastError(){ return errorMessage; }
    
    int ingestHeader(const wxString &senc_file_name);
    int ingest(const wxString &senc_file_name,
               S57ObjVector *pObjectVector,
               VE_ElementVector *pVEArray,
               VC_ElementVector *pVCArray);
    
    //  SENC creation, by Version desired...
    int createSenc124(const wxString& FullPath000, const wxString& SENCFileName, bool b_showProg = true);
    void SetLODMeters(double meters){ m_LOD_meters = meters;}
    void setRegistrar( S57ClassRegistrar *registrar ){ m_poRegistrar = registrar; }
    void setRefLocn( double lat, double lon){ m_ref_lat = lat; m_ref_lon = lon; }
    
    wxString getUpdateDate(){ return m_LastUpdateDate; }
    wxString getBaseDate(){ return m_sdate000; }
    
    wxString getEdition(){ return m_edtn000; }
    int getLastUpdate(){ return m_last_applied_update; }
    wxString getFileModTime(){ return m_FileModeTime; }
    int getSencVersion(){ return m_senc_file_version; }
    wxULongLong getFileSize000(){ return m_FileSize000; }
    wxString getsFileSize000(){ return m_sFileSize000; }
    wxString getName(){ return m_Name; }
    wxString getID(){ return m_ID; }
private:
    void init();
    
    int my_fgets( char *buf, int buf_len_max, wxInputStream& ifs );
    int ingestCell( const wxString &FullPath000, const wxString &working_dir );
    int ValidateAndCountUpdates( const wxFileName file000, const wxString CopyDir,
                                 wxString &LastUpdateDate, bool b_copyfiles);
    int GetUpdateFileArray(const wxFileName file000, wxArrayString *UpFiles);
    bool GetBaseFileAttr( const wxString &FullPath000 );
    

    void CreateSENCRecord124( OGRFeature *pFeature, FILE * fpOut, int mode, S57Reader *poReader );
    void  CreateSENCVectorEdgeTable(FILE * fpOut, S57Reader *poReader);
    void  CreateSENCConnNodeTable(FILE * fpOut, S57Reader *poReader);
    
    wxString            errorMessage;
    
    wxString            m_Name;
    wxString m_ID;
    
    int                 m_Chart_Scale;
    int                 m_senc_file_version;
    
    S57Reader           *poReader;
    OGRS57DataSource    *poS57DS;
    
    wxDateTime          m_date000;
    wxString            m_sdate000;
    
    wxString            m_edtn000;
    int                 m_nGeoRecords;
    int                 m_last_applied_update;
    wxString            m_LastUpdateDate;
    int                 m_native_scale;
    wxString            m_FileModeTime;
    wxULongLong         m_FileSize000;
    wxString            m_sFileSize000;
    
    double              m_ref_lat, m_ref_lon;             // Common reference point, derived from FullExtent
    VectorHelperHash    m_vector_helper_hash;
    double              m_LOD_meters;
    S57ClassRegistrar   *m_poRegistrar;
    wxArrayString       m_tmpup_array;
    
    wxProgressDialog    *s_ProgDialog;
    
    
    
    
    
    
};


#endif          // Guard