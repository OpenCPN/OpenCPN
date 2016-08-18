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

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <wx/wfstream.h>
#include <wx/filename.h>

#include "Osenc.h"
#include "s52s57.h"
#include "s57chart.h"
#include "cutil.h"

#include "mygdal/ogr_s57.h"
#include "mygdal/cpl_string.h"

#include "mygeom.h"
#include "georef.h"

//      External definitions
void OpenCPN_OGRErrorHandler( CPLErr eErrClass, int nError,
                              const char * pszErrorMsg );               // installed GDAL OGR library error handler



//--------------------------------------------------------------------------
//      Osenc implementation
//--------------------------------------------------------------------------

Osenc::Osenc()
{
    init();
}

Osenc::~Osenc()
{
}

void Osenc::init( void )
{
    m_LOD_meters = 0;
    m_poRegistrar = NULL;
    m_senc_file_version = 0;
    s_ProgDialog = NULL;
    
    m_ref_lat = 0;
    m_ref_lon = 0;
    
}

int Osenc::ingestHeader(const wxString &senc_file_name)
{
    
    int ret_val = SENC_NO_ERROR;                    // default is OK
    
    //    Sanity check for existence of file
    
    int nProg = 0;
    
    wxString ifs( senc_file_name );
    
    wxFFileInputStream fpx_u( ifs );
    if (!fpx_u.IsOk()) {
        return ERROR_SENCFILE_NOT_FOUND;
    }
    wxBufferedInputStream fpx( fpx_u );
    
    int MAX_LINE = 499999;
    char *buf = (char *) malloc( MAX_LINE + 1 );
    
    int dun = 0;
    
    while( !dun ) {
        
        if( my_fgets( buf, MAX_LINE, fpx ) == 0 ) {
            dun = 1;
            break;
        }
        
        if( !strncmp( buf, "OGRF", 4 ) ) {
            dun = 1;
        }               //OGRF
        
        else if( !strncmp( buf, "SENC", 4 ) ) {
            sscanf( buf, "SENC Version=%i", &m_senc_file_version );
        }
        
        else if( !strncmp( buf, "DATEUPD", 7 ) ) {
            m_LastUpdateDate = wxString( &buf[8], wxConvUTF8 ).BeforeFirst( '\n' );
        }
        
        else if( !strncmp( buf, "UPDT", 4 ) ) {
            sscanf( buf, "UPDT=%i", &m_last_applied_update );
        }
        
        else if( !strncmp( buf, "EDTN000", 7 ) ) {
            m_edtn000 = wxString( &buf[8], wxConvUTF8 ).BeforeFirst( '\n' );
        }
        
        else if( !strncmp( buf, "DATE000", 7 ) ) {
            m_sdate000 = wxString( &buf[8], wxConvUTF8 ).BeforeFirst( '\n' );
        }
        
        else if( !strncmp( buf, "FILESIZE000", 11 ) ) {
            m_sFileSize000 =  wxString( &buf[12], wxConvUTF8 ).BeforeFirst( '\n' );
        }
        
        else if( !strncmp( buf, "SCALE", 5 ) ) {
            int ins;
            sscanf( buf, "SCALE=%d", &ins );
            m_Chart_Scale = ins;
        }
        
        else if( !strncmp( buf, "NAME", 4 ) ) {
            m_Name = wxString( &buf[5], wxConvUTF8 ).BeforeFirst( '\n' );
        }
        
    }                       //while(!dun)
    
    free( buf );
    
    return ret_val;
}




int Osenc::ingest(const wxString &senc_file_name,
           S57ObjVector *pObjectVector,
           VE_ElementVector *pVEArray,
           VC_ElementVector *pVCArray)
{
 
    int ret_val = SENC_NO_ERROR;                    // default is OK
    int senc_file_version = 0;
    
    wxFileName fn(senc_file_name);
    m_ID = fn.GetName();                          // This will be the NOAA File name, usually
    
    //    Sanity check for existence of file
    
    int nProg = 0;
    
    wxString ifs( senc_file_name );
    
    wxFFileInputStream fpx_u( ifs );
    if (!fpx_u.IsOk()) {
        return ERROR_SENCFILE_NOT_FOUND;
    }
    wxBufferedInputStream fpx( fpx_u );
    
    int MAX_LINE = 499999;
    char *buf = (char *) malloc( MAX_LINE + 1 );
    
    int dun = 0;
    
    while( !dun ) {
        
        if( my_fgets( buf, MAX_LINE, fpx ) == 0 ) {
            dun = 1;
            break;
        }
        
        if( !strncmp( buf, "OGRF", 4 ) ) {
            
            S57Obj *obj = new S57Obj( buf, MAX_LINE, &fpx, 0, 0, senc_file_version );
            if( obj ) {
                //      Ensure that Area objects actually describe a valid object
                if( GEO_AREA == obj->Primitive_type ) {
                    if( !obj->BBObj.GetValid() ) {
                        delete obj;
                        continue;
                    }
                }
                pObjectVector->push_back(obj); 
             }
        }               //OGRF
        
        else if( !strncmp( buf, "VETableStart", 12 ) ) {
            int index = -1;
            int count;
            
            fpx.Read( &index, sizeof(int) );
            
            while( -1 != index ) {
                fpx.Read( &count, sizeof(int) );
                
                double *pPoints = NULL;
                if( count ) {
                    pPoints = (double *) malloc( count * 2 * sizeof(double) );
                    fpx.Read( pPoints, count * 2 * sizeof(double) );
                }
                
                VE_Element *pvee = new VE_Element;
                pvee->index = index;
                pvee->nCount = count;
                pvee->pPoints = pPoints;
                pvee->max_priority = 0;//-99;            // Default
                
                pVEArray->push_back(pvee);
                
                //    Next element
                fpx.Read( &index, sizeof(int) );
            }
            
#if 0   // clients job            
            //    Create a hash map of VE_Element pointers as a chart class member
            int n_ve_elements = ve_array.GetCount();
            
            for( int i = 0; i < n_ve_elements; i++ ) {
                VE_Element ve_from_array = ve_array.Item( i );
                VE_Element *vep = new VE_Element;
                vep->index = ve_from_array.index;
                vep->nCount = ve_from_array.nCount;
                vep->pPoints = ve_from_array.pPoints;
                vep->max_priority = 0;            // Default
                
                if(vep->nCount){
                    //  Get a bounding box for the edge
                    double east_max = -1e7; double east_min = 1e7;
                    double north_max = -1e7; double north_min = 1e7;
                    
                    double *vrun = vep->pPoints;
                    for(size_t i=0 ; i < vep->nCount; i++){
                        east_max = wxMax(east_max, *vrun);
                        east_min = wxMin(east_min, *vrun);
                        vrun++;
                        
                        north_max = wxMax(north_max, *vrun);
                        north_min = wxMin(north_min, *vrun);
                        vrun++;
                    }
                    
                    double lat, lon;
                    fromSM( east_min, north_min, m_ref_lat, m_ref_lon, &lat, &lon );
                    vep->BBox.SetMin( lon, lat);
                    fromSM( east_max, north_max, m_ref_lat, m_ref_lon, &lat, &lon );
                    vep->BBox.SetMax( lon, lat);
                }
                
                m_ve_hash[vep->index] = vep;
                
            }
#endif            
        }
        
        else if( !strncmp( buf, "VCTableStart", 12 ) ) {
            int index = -1;
            int index_max = -1;
            
            fpx.Read( &index, sizeof(int) );
            
            while( -1 != index ) {
                
                double *pPoint = NULL;
                pPoint = (double *) malloc( 2 * sizeof(double) );
                fpx.Read( pPoint, 2 * sizeof(double) );
                
                VC_Element *pvce = new VC_Element;
                pvce->index = index;
                pvce->pPoint = pPoint;
                
                pVCArray->push_back(pvce);
                
                //    Next element
                fpx.Read( &index, sizeof(int) );
            }

#if 0 // clients job            
            //    Create a hash map VC_Element pointers as a chart class member
            int n_vc_elements = vc_array.GetCount();
            
            for( int i = 0; i < n_vc_elements; i++ ) {
                VC_Element vc_from_array = vc_array.Item( i );
                VC_Element *vcp = new VC_Element;
                vcp->index = vc_from_array.index;
                vcp->pPoint = vc_from_array.pPoint;
                
                m_vc_hash[vcp->index] = vcp;
            }
#endif            
        }
        
        else if( !strncmp( buf, "SENC", 4 ) ) {
            sscanf( buf, "SENC Version=%i", &senc_file_version );
            if( senc_file_version != CURRENT_SENC_FORMAT_VERSION ) {
                errorMessage = wxString( _T("   Wrong version on SENC file ") );
                errorMessage.Append( senc_file_name );
                dun = 1;
                ret_val = ERROR_SENC_VERSION_MISMATCH;                   // error
            }
        }
        
        else if( !strncmp( buf, "DATEUPD", 7 ) ) {
            m_LastUpdateDate = wxString( &buf[8], wxConvUTF8 ).BeforeFirst( '\n' ) ;
        }
        
        else if( !strncmp( buf, "DATE000", 7 ) ) {
            m_sdate000 =  wxString( &buf[8], wxConvUTF8 ).BeforeFirst( '\n' );
        }
        
        else if( !strncmp( buf, "SCALE", 5 ) ) {
            int ins;
            sscanf( buf, "SCALE=%d", &ins );
            m_Chart_Scale = ins;
        }
        
        else if( !strncmp( buf, "NAME", 4 ) ) {
            m_Name = wxString( &buf[5], wxConvUTF8 ).BeforeFirst( '\n' );
        }
        
//        else if( !strncmp( buf, "NOGR", 4 ) ) {
//            sscanf( buf, "NOGR=%d", &nGeoFeature );
//        }
    }                       //while(!dun)
    
    //      fclose(fpx);
    
    free( buf );
    
    return ret_val;
    
}

//------------------------------------------------------------------------------
//      Local version of fgets for Binary Mode (SENC) file
//------------------------------------------------------------------------------
int Osenc::my_fgets( char *buf, int buf_len_max, wxInputStream& ifs )

{
    char chNext;
    int nLineLen = 0;
    char *lbuf;
    
    lbuf = buf;
    
    while( !ifs.Eof() && nLineLen < buf_len_max ) {
        chNext = (char) ifs.GetC();
        
        /* each CR/LF (or LF/CR) as if just "CR" */
        if( chNext == 10 || chNext == 13 ) {
            chNext = '\n';
        }
        
        *lbuf = chNext;
        lbuf++, nLineLen++;
        
        if( chNext == '\n' ) {
            *lbuf = '\0';
            return nLineLen;
        }
    }
    
    *( lbuf ) = '\0';
    
    return nLineLen;
}


int Osenc::createSenc124(const wxString& FullPath000, const wxString& SENCFileName, bool b_showProg)
{
    if(!m_poRegistrar){
        errorMessage = _T("S57 Registrar not set.");
        return ERROR_REGISTRAR_NOT_SET;
    }
    
    wxFileName SENCfile = wxFileName( SENCFileName );
    wxFileName file000 = wxFileName( FullPath000 );
    
    
    //      Make the target directory if needed
    if( true != SENCfile.DirExists( SENCfile.GetPath() ) ) {
        if( !SENCfile.Mkdir( SENCfile.GetPath() ) ) {
            errorMessage = _T("Cannot create SENC file directory for ") + SENCfile.GetFullPath();
            return ERROR_CANNOT_CREATE_SENC_DIR;
        }
    }
    
    
    //          Make a temp file to create the SENC in
    wxFileName tfn;
    wxString tmp_file = tfn.CreateTempFileName( _T("") );
    
    FILE *fps57;
    const char *pp = "wb";
    fps57 = fopen( tmp_file.mb_str(), pp );
    
    if( fps57 == NULL ) {
        errorMessage = _T("Unable to create temp SENC file: ");
        errorMessage.Append( tfn.GetFullPath() );
        return ERROR_CANNOT_CREATE_TEMP_SENC_FILE;
    }
    

    //  Take a quick scan of the 000 file to get some basic attributes of the exchange set.
    if(!GetBaseFileAttr( FullPath000 ) ){
        return ERROR_BASEFILE_ATTRIBUTES;
    }
    
    
    //  Ingest the .000 cell, with updates applied
    
    if(ingestCell(FullPath000, SENCfile.GetPath())){
        errorMessage = _T("Error ingesting: ") + FullPath000;
        return ERROR_INGESTING000;
    }
    
    
    //          Write the Header information
    
    char temp[201];
    
    fprintf( fps57, "SENC Version= %d\n", 124 );
   
    // The chart cell "nice name"
    wxString nice_name;
    s57chart::GetChartNameFromTXT( FullPath000, nice_name );
    
    wxCharBuffer buffer=nice_name.ToUTF8();
    if(buffer.data()) 
        strncpy( temp, buffer.data(), 200 );
    else
        strncpy( temp, "UTF8Error", 200 );
    
    temp[200] = '\0';
    fprintf( fps57, "NAME=%s\n", temp );
    
    
    wxString date000 = m_date000.Format( _T("%Y%m%d") );
    strncpy( temp, date000.mb_str(), 200 );
    fprintf( fps57, "DATE000=%s\n", temp );
    
    strncpy( temp, m_edtn000.mb_str(), 200 );
    fprintf( fps57, "EDTN000=%s\n", temp );
    
    //      Record .000 file date and size for primitive detection of updates to .000 file
    wxDateTime ModTime000;
    wxString mt = _T("20000101");
    if( file000.GetTimes( NULL, &ModTime000, NULL ) )
        mt = ModTime000.Format( _T("%Y%m%d") );
    strncpy( temp, mt.mb_str(), 200 );
    fprintf( fps57, "FILEMOD000=%s\n", temp );
    m_FileModeTime = mt;
    
    int size000 = file000.GetSize().GetHi();
    int size000l = file000.GetSize().GetLo();
    fprintf( fps57, "FILESIZE000=%d%d\n", size000, size000l );
    m_FileSize000 = file000.GetSize();
     
    fprintf( fps57, "NOGR=%d\n", m_nGeoRecords );
    fprintf( fps57, "SCALE=%d\n", m_native_scale );
    
    fprintf( fps57, "UPDT=%d\n", m_last_applied_update );
    
    strncpy( temp, m_LastUpdateDate.mb_str(), 200 );
    fprintf( fps57, "DATEUPD=%s\n", temp );
    

    //        Prepare Vector Edge Helper table
    //        And fill in the table
    int feid = 0;
    OGRFeature *pEdgeVectorRecordFeature = poReader->ReadVector( feid, RCNM_VE );
    while( NULL != pEdgeVectorRecordFeature ) {
        int record_id = pEdgeVectorRecordFeature->GetFieldAsInteger( "RCID" );
        
        m_vector_helper_hash[record_id] = feid;
        
        feid++;
        delete pEdgeVectorRecordFeature;
        pEdgeVectorRecordFeature = poReader->ReadVector( feid, RCNM_VE );
    }
    
    wxString Message = SENCfile.GetFullPath();
    Message.Append( _T("...Ingesting") );
    
    wxString Title( _("OpenCPN S57 SENC File Create...") );
    Title.append( SENCfile.GetFullPath() );
    
    wxStopWatch progsw;
    int nProg = poReader->GetFeatureCount();
    
    if(wxThread::IsMain() && b_showProg){
         s_ProgDialog = new wxProgressDialog( Title, Message, nProg, NULL,
                                         wxPD_AUTO_HIDE | wxPD_SMOOTH | wxSTAY_ON_TOP | wxPD_APP_MODAL);
    }
    
    
    
    //  Loop in the S57 reader, extracting Features one-by-one
    OGRFeature *objectDef;
    
    bool bcont = true;
    int iObj = 0;
    
    while( bcont ) {
        objectDef = poReader->ReadNextFeature();
        
        if( objectDef != NULL ) {
            
            iObj++;

           
            //  Update the progress dialog
            //We update only every 200 milliseconds to improve performance as updating the dialog is very expensive...
            // WXGTK is measurably slower even with 100ms here
            if( s_ProgDialog && progsw.Time() > 200 )
            {
                progsw.Start();
                
                wxString sobj = wxString( objectDef->GetDefnRef()->GetName(), wxConvUTF8 );
                sobj.Append( wxString::Format( _T("  %d/%d       "), iObj, nProg ) );
                
                bcont = s_ProgDialog->Update( iObj, sobj );
            }

            OGRwkbGeometryType geoType = wkbUnknown;
            //      This test should not be necessary for real (i.e not C_AGGR) features
            //      However... some update files contain errors, and have deleted some
            //      geometry without deleting the corresponding feature(s).
            //      So, GeometryType becomes Unknown.
            //      e.g. US5MD11M.017
            //      In this case, all we can do is skip the feature....sigh.
            
            if( objectDef->GetGeometryRef() != NULL )
                geoType = objectDef->GetGeometryRef()->getGeometryType();
            
            //      n.b  This next line causes skip of C_AGGR features w/o geometry
            if( geoType != wkbUnknown ){                             // Write only if has wkbGeometry
                CreateSENCRecord124( objectDef, fps57, 1, poReader );
            }
            
            delete objectDef;
            
        } else
            break;
        
    }

    if( bcont ) {
        //      Create and write the Vector Edge Table
        CreateSENCVectorEdgeTable( fps57, poReader );
        
        //      Create and write the Connected NodeTable
        CreateSENCConnNodeTable( fps57, poReader );
    }
    
    
    //          All done, so clean up
    fclose( fps57 );
    
    CPLPopErrorHandler();
    
    //  Delete any temporary (working) real and dummy update files,
    //  as well as .000 file created by ValidateAndCountUpdates()
    for( unsigned int iff = 0; iff < m_tmpup_array.GetCount(); iff++ )
        remove( m_tmpup_array.Item( iff ).mb_str() );
    
    int ret_code = 0;
    
    if( !bcont )                // aborted
    {
        wxRemoveFile( tmp_file );                     // kill the temp file
        ret_code = ERROR_SENCFILE_ABORT;
    }
    
    if( bcont ) {
       bool cpok = wxRenameFile( tmp_file, SENCfile.GetFullPath() );
        if( !cpok ) {
            errorMessage =  _T("   Cannot rename temporary SENC file ");
            errorMessage.Append( tmp_file );
            errorMessage.Append( _T(" to ") );
            errorMessage.Append( SENCfile.GetFullPath() );
            ret_code = ERROR_SENCFILE_ABORT;
        } else
            ret_code = SENC_NO_ERROR;
    }
    
    delete s_ProgDialog;
    
    delete poS57DS;
    
    return ret_code;
    
    
}

void Osenc::CreateSENCRecord124( OGRFeature *pFeature, FILE * fpOut, int mode, S57Reader *poReader )
{
    
    #define MAX_HDR_LINE    400
    
    char line[MAX_HDR_LINE + 1];
    wxString sheader;
    
    fprintf( fpOut, "OGRFeature(%s):%ld\n", pFeature->GetDefnRef()->GetName(), pFeature->GetFID() );
    
    //      In the interests of output file size, DO NOT report fields that are not set.
    for( int iField = 0; iField < pFeature->GetFieldCount(); iField++ ) {
        if( pFeature->IsFieldSet( iField ) ) {
            if( ( iField == 1 ) || ( iField > 7 ) ) {
                OGRFieldDefn *poFDefn = pFeature->GetDefnRef()->GetFieldDefn( iField );
                const char *pType = OGRFieldDefn::GetFieldTypeName( poFDefn->GetType() );
                const char *pAttrName = poFDefn->GetNameRef();
                const char *pAttrVal = pFeature->GetFieldAsString( iField );
                
                if(strlen( pAttrVal ) ) {
                    snprintf( line, MAX_HDR_LINE - 2, "  %s (%c) = ", pAttrName, *pType);
                    wxString AttrStringPrefix = wxString( line, wxConvUTF8 );
                    
                    wxString wxAttrValue;
                    
                    if( (0 == strncmp("NOBJNM",pAttrName, 6) ) ||
                        (0 == strncmp("NINFOM",pAttrName, 6) ) ||
                        (0 == strncmp("NPLDST",pAttrName, 6) ) ||
                        (0 == strncmp("NTXTDS",pAttrName, 6) ) )
                    {
                        if( poReader->GetNall() == 2) {     // ENC is using UCS-2 / UTF-16 encoding
                            wxMBConvUTF16 conv;
                            wxString att_conv(pAttrVal, conv);
                            att_conv.RemoveLast();      // Remove the \037 that terminates UTF-16 strings in S57
                            att_conv.Replace(_T("\n"), _T("|") );  //Replace  <new line> with special break character
                            wxAttrValue = att_conv;
                        }
                        else if( poReader->GetNall() == 1) {     // ENC is using Lex level 1 (ISO 8859_1) encoding
                            wxCSConv conv(_T("iso8859-1") );
                            wxString att_conv(pAttrVal, conv);
                            wxAttrValue = att_conv;
                        }
                    }
                    
                    if( wxAttrValue.IsEmpty()) {
                        // Attempt different conversions to accomodate different language encodings in
                        // the original ENC files.
                        
                        //  For attribute fields other that "National" Objects, (handled above)  See FS#1464
                        //  the text must be encoded in simple ASCII, or in lex level 1 (i.e. iso8859-1)
                        
                        wxAttrValue = wxString( pAttrVal, wxConvUTF8 );
                        
                        if( 0 ==wxAttrValue.Length() ) {
                            wxCSConv conv(_T("iso8859-1") );
                            wxString att_conv(pAttrVal, conv);
                            wxAttrValue = att_conv;
                        }
                        
                        if( 0 ==wxAttrValue.Length() ) {
                            wxLogError( _T("Warning: CreateSENCRecord(): Failed to convert string value to wxString.") );
                        }
                    }
                    
                    sheader += AttrStringPrefix;
                    sheader += wxAttrValue;
                    sheader += '\n';
                }
            }
        }
    }
    
    OGRGeometry *pGeo = pFeature->GetGeometryRef();
    
    //    Special geometry cases
    ///172
    if( wkbPoint == pGeo->getGeometryType() ) {
        OGRPoint *pp = (OGRPoint *) pGeo;
        int nqual = pp->getnQual();
        if( 10 != nqual )                    // only add attribute if nQual is not "precisely known"
                {
                    snprintf( line, MAX_HDR_LINE - 2, "  %s (%c) = %d", "QUALTY", 'I', nqual );
                    sheader += wxString( line, wxConvUTF8 );
                    sheader += '\n';
                }
                
    }
    
    if( mode == 1 ) {
        sprintf( line, "  %s %f %f\n", pGeo->getGeometryName(), m_ref_lat, m_ref_lon );
        sheader += wxString( line, wxConvUTF8 );
    }
    wxCharBuffer buffer=sheader.ToUTF8();
    fprintf( fpOut, "HDRLEN=%lu\n", (unsigned long) strlen(buffer) );
    fwrite( buffer.data(), 1, strlen(buffer), fpOut );
    
    if( ( pGeo != NULL ) /*&& (mode == 1)*/) {
        int wkb_len = pGeo->WkbSize();
        unsigned char *pwkb_buffer = (unsigned char *) malloc( wkb_len );
        
        //  Get the GDAL data representation
        pGeo->exportToWkb( wkbNDR, pwkb_buffer );
        
        //  Convert to opencpn SENC representation
        
        //  Set absurd bbox starting limits
        float lonmax = -1000;
        float lonmin = 1000;
        float latmax = -1000;
        float latmin = 1000;
        
        int i, ip, sb_len;
        float *pdf;
        double *psd;
        unsigned char *ps;
        unsigned char *pd;
        unsigned char *psb_buffer;
        double lat, lon;
        int nPoints;
        wxString msg;
        
        OGRwkbGeometryType gType = pGeo->getGeometryType();
        switch( gType ){
            case wkbLineString: {
                sb_len = ( ( wkb_len - 9 ) / 2 ) + 9 + 16;  // data will be 4 byte float, not double
                // and bbox limits are tacked on end
                fprintf( fpOut, "  %d\n", sb_len );
                
                psb_buffer = (unsigned char *) malloc( sb_len );
                pd = psb_buffer;
                ps = pwkb_buffer;
                
                memcpy( pd, ps, 9 );                                  // byte order, type, and count
                
                ip = *( (int *) ( ps + 5 ) );                              // point count
                
                pd += 9;
                ps += 9;
                psd = (double *) ps;
                pdf = (float *) pd;
                
                for( i = 0; i < ip; i++ )                           // convert doubles to floats
                        {                                                 // computing bbox as we go
                        
                        float lon, lat;
                double easting, northing;
                #ifdef ARMHF
                double east_d, north_d;
                memcpy(&east_d, psd++, sizeof(double));
                memcpy(&north_d, psd++, sizeof(double));
                lon = east_d;
                lat = north_d;
                
                //  Calculate SM from chart common reference point
                toSM( lat, lon, m_ref_lat, m_ref_lon, &easting, &northing );
                memcpy(pdf++, &easting, sizeof(float));
                memcpy(pdf++, &northing, sizeof(float));
                
                #else                    
                lon = (float) *psd++;
                lat = (float) *psd++;
                
                //  Calculate SM from chart common reference point
                toSM( lat, lon, m_ref_lat, m_ref_lon, &easting, &northing );
                
                *pdf++ = easting;
                *pdf++ = northing;
                #endif
                lonmax = fmax(lon, lonmax);
                lonmin = fmin(lon, lonmin);
                latmax = fmax(lat, latmax);
                latmin = fmin(lat, latmin);
                
                        }
                        
                        #ifdef ARMHF
                        float tmp;
                        tmp = lonmax;
                        memcpy(pdf++, &tmp, sizeof(float));
                        tmp = lonmin;
                        memcpy(pdf++, &tmp, sizeof(float));
                        tmp = latmax;
                        memcpy(pdf++, &tmp, sizeof(float));
                        tmp = latmin;
                        memcpy(pdf, &tmp, sizeof(float));
                        #else                
                        //      Store the Bounding Box as lat/lon
                        *pdf++ = lonmax;
                        *pdf++ = lonmin;
                        *pdf++ = latmax;
                        *pdf = latmin;
                        #endif
                        fwrite( psb_buffer, 1, sb_len, fpOut );
                        fprintf( fpOut, "\n" );
                        free( psb_buffer );
                        
                        //    Capture the Vector Table geometry indices
                        int *pNAME_RCID;
                        int *pORNT;
                        int nEdgeVectorRecords;
                        OGRFeature *pEdgeVectorRecordFeature;
                        
                        pNAME_RCID = (int *) pFeature->GetFieldAsIntegerList( "NAME_RCID",
                        &nEdgeVectorRecords );
                        
                        pORNT = (int *) pFeature->GetFieldAsIntegerList( "ORNT", NULL );
                        
                        fprintf( fpOut, "LSINDEXLIST %d\n", nEdgeVectorRecords );
                        //                    fwrite(pNAME_RCID, 1, nEdgeVectorRecords * sizeof(int), fpOut);
                        
                        //  Set up the options, adding RETURN_PRIMITIVES
                        char ** papszReaderOptions = NULL;
                        papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_UPDATES, "ON" );
                        papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_LINKAGES,
                                                              "ON" );
                        papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES,
                                                              "ON" );
                        poReader->SetOptions( papszReaderOptions );
                        
                        //    Capture the beginning and end point connected nodes for each edge vector record
                        for( i = 0; i < nEdgeVectorRecords; i++ ) {
                            int start_rcid, end_rcid;
                            int target_record_feid = m_vector_helper_hash[pNAME_RCID[i]];
                            pEdgeVectorRecordFeature = poReader->ReadVector( target_record_feid, RCNM_VE );
                            
                            if( NULL != pEdgeVectorRecordFeature ) {
                                start_rcid = pEdgeVectorRecordFeature->GetFieldAsInteger( "NAME_RCID_0" );
                                end_rcid = pEdgeVectorRecordFeature->GetFieldAsInteger( "NAME_RCID_1" );
                                
                                //    Make sure the start and end points exist....
                                //    Note this poReader method was converted to Public access to
                                //     facilitate this test.  There might be another clean way....
                                //    Problem first found on Holand ENC 1R5YM009.000
                                if( !poReader->FetchPoint( RCNM_VC, start_rcid, NULL, NULL, NULL, NULL ) ) start_rcid =
                                    -1;
                                if( !poReader->FetchPoint( RCNM_VC, end_rcid, NULL, NULL, NULL, NULL ) ) end_rcid =
                                    -2;
                                
                                int edge_ornt = 1;
                                //  Allocate some storage for converted points
                                
                                if( edge_ornt == 1 )                                    // forward
                                {
                                    fwrite( &start_rcid, 1, sizeof(int), fpOut );
                                    fwrite( &pNAME_RCID[i], 1, sizeof(int), fpOut );
                                    fwrite( &end_rcid, 1, sizeof(int), fpOut );
                                } else                                                  // reverse
                        {
                            fwrite( &end_rcid, 1, sizeof(int), fpOut );
                            fwrite( &pNAME_RCID[i], 1, sizeof(int), fpOut );
                            fwrite( &start_rcid, 1, sizeof(int), fpOut );
                        }
                        
                        delete pEdgeVectorRecordFeature;
                            } else {
                                start_rcid = -1;                                    // error indication
                                end_rcid = -2;
                                
                                fwrite( &start_rcid, 1, sizeof(int), fpOut );
                                fwrite( &pNAME_RCID[i], 1, sizeof(int), fpOut );
                                fwrite( &end_rcid, 1, sizeof(int), fpOut );
                            }
                        }
                        
                        fprintf( fpOut, "\n" );
                        
                        //  Reset the options
                        papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES,
                                                              "OFF" );
                        poReader->SetOptions( papszReaderOptions );
                        CSLDestroy( papszReaderOptions );
                        
                        //                    free(pVectorEdgeHelperTable);
                        
                        break;
            }
            case wkbMultiLineString:
                msg = _T("   Warning: Unimplemented SENC wkbMultiLineString record");
                wxLogMessage( msg );
                
                wkb_len = pGeo->WkbSize();
                fprintf( fpOut, "  %d\n", wkb_len );
                fwrite( pwkb_buffer, 1, wkb_len, fpOut );
                break;
                
            case wkbPoint: {
                int nq_len = 4;                                     // nQual length
                
                sb_len = ( ( wkb_len - ( 5 + nq_len ) ) / 2 ) + 5; // data will be 4 byte float, not double
                // and skipping nQual
                
                fprintf( fpOut, "  %d\n", sb_len );
                
                psb_buffer = (unsigned char *) malloc( sb_len );
                pd = psb_buffer;
                ps = pwkb_buffer;
                
                memcpy( pd, ps, 5 );                                 // byte order, type
                
                pd += 5;
                ps += 5 + nq_len;
                psd = (double *) ps;
                pdf = (float *) pd;
                
                #ifdef ARMHF
                double lata, lona;
                memcpy(&lona, psd, sizeof(double));
                memcpy(&lata, &psd[1], sizeof(double));
                lon = lona;
                lat = lata;
                #else                
                lon = *psd++;                                      // fetch the point
                lat = *psd;
                #endif                
                
                
                //  Calculate SM from chart common reference point
                double easting, northing;
                toSM( lat, lon, m_ref_lat, m_ref_lon, &easting, &northing );
                
                #ifdef ARMHF
                float east, north;
                east = easting;
                north = northing;
                memcpy(pdf++, &east, sizeof(float));
                memcpy(pdf,   &north, sizeof(float));
                #else                
                *pdf++ = easting;
                *pdf = northing;
                #endif
                //  And write it out
                fwrite( psb_buffer, 1, sb_len, fpOut );
                fprintf( fpOut, "\n" );
                
                free( psb_buffer );
                
                break;
            }
            
            case wkbMultiPoint:
            case wkbMultiPoint25D:
                ps = pwkb_buffer;
                ps += 5;
                nPoints = *( (int *) ps );                     // point count
                
                sb_len = ( 9 + nPoints * 3 * sizeof(float) ) + 16; // GTYPE and count, points as floats
                // and trailing bbox
                fprintf( fpOut, "  %d\n", sb_len );
                
                psb_buffer = (unsigned char *) malloc( sb_len );
                pd = psb_buffer;
                ps = pwkb_buffer;
                
                memcpy( pd, ps, 9 );                                  // byte order, type, count
                
                ps += 9;
                pd += 9;
                
                pdf = (float *) pd;
                
                for( ip = 0; ip < nPoints; ip++ ) {
                    
                    // Workaround a bug?? in OGRGeometryCollection
                    // While exporting point geometries serially, OGRPoint->exportToWkb assumes that
                    // if Z is identically 0, then the point must be a 2D point only.
                    // So, the collection Wkb is corrupted with some 3D, and some 2D points.
                    // Workaround:  Get reference to the points serially, and explicitly read X,Y,Z
                    // Ignore the previously read Wkb buffer
                    
                    OGRGeometryCollection *temp_geometry_collection = (OGRGeometryCollection *) pGeo;
                    OGRGeometry *temp_geometry = temp_geometry_collection->getGeometryRef( ip );
                    OGRPoint *pt_geom = (OGRPoint *) temp_geometry;
                    
                    lon = pt_geom->getX();
                    lat = pt_geom->getY();
                    double depth = pt_geom->getZ();
                    
                    //  Calculate SM from chart common reference point
                    double easting, northing;
                    toSM( lat, lon, m_ref_lat, m_ref_lon, &easting, &northing );
                    
                    #ifdef ARMHF
                    float east = easting;
                    float north = northing;
                    float deep = depth;
                    memcpy(pdf++, &east, sizeof(float));
                    memcpy(pdf++, &north, sizeof(float));
                    memcpy(pdf++, &deep, sizeof(float));
                    
                    #else                    
                    *pdf++ = easting;
                    *pdf++ = northing;
                    *pdf++ = (float) depth;
                    #endif
                    
                    //  Keep a running calculation of min/max
                    lonmax = fmax(lon, lonmax);
                    lonmin = fmin(lon, lonmin);
                    latmax = fmax(lat, latmax);
                    latmin = fmin(lat, latmin);
                }
                
                //      Store the Bounding Box as lat/lon
                #ifdef ARMHF
                float tmp;
                tmp = lonmax;
                memcpy(pdf++, &tmp, sizeof(float));
                tmp = lonmin;
                memcpy(pdf++, &tmp, sizeof(float));
                tmp = latmax;
                memcpy(pdf++, &tmp, sizeof(float));
                tmp = latmin;
                memcpy(pdf, &tmp, sizeof(float));
                #else                
                *pdf++ = lonmax;
                *pdf++ = lonmin;
                *pdf++ = latmax;
                *pdf = latmin;
                #endif
                //  And write it out
                fwrite( psb_buffer, 1, sb_len, fpOut );
                free( psb_buffer );
                fprintf( fpOut, "\n" );
                
                break;
                
                //      Special case, polygons are handled separately
                case wkbPolygon: {
                    int error_code;
                    
                    PolyTessGeo *ppg = NULL;
                    
                    OGRPolygon *poly = (OGRPolygon *) ( pGeo );
                    
                    ppg = new PolyTessGeo( poly, true, m_ref_lat, m_ref_lon, false, m_LOD_meters );   //try to use glu library
                    
                    error_code = ppg->ErrorCode;
                    if( error_code == ERROR_NO_DLL ) {
//                        if( !bGLUWarningSent ) {
//                            wxLogMessage( _T("   Warning...Could not find glu32.dll, trying internal tess.") );
//                            bGLUWarningSent = true;
//                        }
                        
                        delete ppg;
                        //  Try with internal tesselator
                        ppg = new PolyTessGeo( poly, true, m_ref_lat, m_ref_lon, true, m_LOD_meters );
                        error_code = ppg->ErrorCode;
                    }
                    
                    if( error_code )
                        wxLogMessage( _T("   Warning: S57 SENC Geometry Error %d, Some Features ignored."), ppg->ErrorCode );
                    else
                        ppg->Write_PolyTriGroup( fpOut );
                    
                    delete ppg;
                    
                    //    Capture the Vector Table geometry indices
                    if( !error_code ) {
                        int *pNAME_RCID;
                        int nEdgeVectorRecords;
                        OGRFeature *pEdgeVectorRecordFeature;
                        
                        pNAME_RCID = (int *) pFeature->GetFieldAsIntegerList( "NAME_RCID",
                        &nEdgeVectorRecords );
                        
                        fprintf( fpOut, "LSINDEXLIST %d\n", nEdgeVectorRecords );
                        
                        //  Set up the options, adding RETURN_PRIMITIVES
                        char ** papszReaderOptions = NULL;
                        papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_UPDATES, "ON" );
                        papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_LINKAGES,
                                                              "ON" );
                        papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES,
                                                              "ON" );
                        poReader->SetOptions( papszReaderOptions );
                        
                        //    Capture the beginning and end point connected nodes for each edge vector record
                        for( i = 0; i < nEdgeVectorRecords; i++ ) {
                            int start_rcid, end_rcid;
                            /*
                             *                        //  Filter out absurd RCIDs, found first on cell IT300017.000
                             *                        if((pNAME_RCID[i] >= 0) && (pNAME_RCID[i] <= m_nvector_table_size))
                             *                        {
                             *                        int target_record_feid = m_pVectorEdgeHelperTable[pNAME_RCID[i]];
                             *                        pEdgeVectorRecordFeature = poReader->ReadVector( target_record_feid, RCNM_VE );
                             }
                             else
                                 pEdgeVectorRecordFeature = NULL;
                             */
                            int target_record_feid = m_vector_helper_hash[pNAME_RCID[i]];
                            pEdgeVectorRecordFeature = poReader->ReadVector( target_record_feid, RCNM_VE );
                            
                            if( NULL != pEdgeVectorRecordFeature ) {
                                start_rcid = pEdgeVectorRecordFeature->GetFieldAsInteger( "NAME_RCID_0" );
                                end_rcid = pEdgeVectorRecordFeature->GetFieldAsInteger( "NAME_RCID_1" );
                                
                                fwrite( &start_rcid, 1, sizeof(int), fpOut );
                                fwrite( &pNAME_RCID[i], 1, sizeof(int), fpOut );
                                fwrite( &end_rcid, 1, sizeof(int), fpOut );
                                
                                delete pEdgeVectorRecordFeature;
                            } else {
                                start_rcid = -1;                                    // error indication
                                end_rcid = -2;
                                
                                fwrite( &start_rcid, 1, sizeof(int), fpOut );
                                fwrite( &pNAME_RCID[i], 1, sizeof(int), fpOut );
                                fwrite( &end_rcid, 1, sizeof(int), fpOut );
                            }
                        }
                        
                        fprintf( fpOut, "\n" );
                        
                        //  Reset the options
                        papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES,
                                                              "OFF" );
                        poReader->SetOptions( papszReaderOptions );
                        CSLDestroy( papszReaderOptions );
                    }
                    
                    break;
                }
                
                //      All others
                default:
                    msg = _T("   Warning: Unimplemented ogr geotype record ");
                    wxLogMessage( msg );
                    
                    wkb_len = pGeo->WkbSize();
                    fprintf( fpOut, "  %d\n", wkb_len );
                    fwrite( pwkb_buffer, 1, wkb_len, fpOut );
                    break;
        }       // switch
        
        free( pwkb_buffer );
    }
}

int Osenc::ingestCell( const wxString &FullPath000, const wxString &working_dir )
{
    //      Analyze Updates
    //      The OGR library will apply updates automatically, if enabled.
    //      Alternatively, we can explicitely find and apply updates from any source directory.
    //      We need to keep track of the last sequential update applied, to look out for new updates
    
    int last_applied_update = 0;
    wxString LastUpdateDate = m_date000.Format( _T("%Y%m%d") );
    
    m_last_applied_update = ValidateAndCountUpdates( FullPath000, working_dir, LastUpdateDate, true );
    m_LastUpdateDate = LastUpdateDate;
    
    if( m_last_applied_update > 0 ){
        wxString msg1;
        msg1.Printf( _T("Preparing to apply ENC updates, target final update is %3d."), last_applied_update );
        wxLogMessage( msg1 );
    }
    
    
    //      Insert my local error handler to catch OGR errors,
    //      Especially CE_Fatal type errors
    //      Discovered/debugged on US5MD11M.017.  VI 548 geometry deleted
    CPLPushErrorHandler( OpenCPN_OGRErrorHandler );
    
    bool bcont = true;
    int iObj = 0;
 //   OGRwkbGeometryType geoType;
    wxString sobj;
    
    //  Here comes the actual ISO8211 file reading
    poS57DS = new OGRS57DataSource;
    poS57DS->SetS57Registrar( m_poRegistrar );
    
    //  Set up the options
    char ** papszReaderOptions = NULL;
    //    papszReaderOptions = CSLSetNameValue(papszReaderOptions, S57O_LNAM_REFS, "ON" );
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_UPDATES, "ON" );
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_LINKAGES, "ON" );
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES, "ON" );
    poS57DS->SetOptionList( papszReaderOptions );
    
    //      Open the OGRS57DataSource
    //      This will ingest the .000 file from the working dir, and apply updates
    
    int open_return = poS57DS->Open( m_tmpup_array.Item( 0 ).mb_str(), TRUE, NULL/*&s_ProgressCallBack*/ ); ///172
    //if( open_return == BAD_UPDATE )         ///172
    //    bbad_update = true;
    
    //      Get a pointer to the reader
    poReader = poS57DS->GetModule( 0 );
    
    //      Update the options, removing the RETURN_PRIMITIVES flags
    //      This flag needed to be set on ingest() to create the proper field defns,
    //      but cleared to fetch normal features
    
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES, "OFF" );
    poReader->SetOptions( papszReaderOptions );
    CSLDestroy( papszReaderOptions );
    
    return 0;
}

int Osenc::ValidateAndCountUpdates( const wxFileName file000, const wxString CopyDir,
                                       wxString &LastUpdateDate, bool b_copyfiles )
{
    
    int retval = 0;
    
    //       wxString DirName000 = file000.GetPath((int)(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME));
    //       wxDir dir(DirName000);
    wxArrayString *UpFiles = new wxArrayString;
    retval = s57chart::GetUpdateFileArray( file000, UpFiles, m_date000, m_edtn000);
    
    if( UpFiles->GetCount() ) {
        //      The s57reader of ogr requires that update set be sequentially complete
        //      to perform all the updates.  However, some NOAA ENC distributions are
        //      not complete, as apparently some interim updates have been  withdrawn.
        //      Example:  as of 20 Dec, 2005, the update set for US5MD11M.000 includes
        //      US5MD11M.017, ...018, and ...019.  Updates 001 through 016 are missing.
        //
        //      Workaround.
        //      Create temporary dummy update files to fill out the set before invoking
        //      ogr file open/ingest.  Delete after SENC file create finishes.
        //      Set starts with .000, which has the effect of copying the base file to the working dir
        
        bool chain_broken_mssage_shown = false;
        
        if( b_copyfiles ) {
          
            for( int iff = 0; iff < retval + 1; iff++ ) {
                wxFileName ufile( file000 );
                wxString sext;
                sext.Printf( _T("%03d"), iff );
                ufile.SetExt( sext );
                
                //      Create the target update file name
                wxString cp_ufile = CopyDir;
                if( cp_ufile.Last() != ufile.GetPathSeparator() ) cp_ufile.Append(
                    ufile.GetPathSeparator() );
                
                cp_ufile.Append( ufile.GetFullName() );
                
                //      Explicit check for a short update file, possibly left over from a crash...
                int flen = 0;
                if( ufile.FileExists() ) {
                    wxFile uf( ufile.GetFullPath() );
                    if( uf.IsOpened() ) {
                        flen = uf.Length();
                        uf.Close();
                    }
                }
                
                if( ufile.FileExists() && ( flen > 25 ) )        // a valid update file or base file
                        {
                            //      Copy the valid file to the SENC directory
                            bool cpok = wxCopyFile( ufile.GetFullPath(), cp_ufile );
                            if( !cpok ) {
                                wxString msg( _T("   Cannot copy temporary working ENC file ") );
                                msg.Append( ufile.GetFullPath() );
                                msg.Append( _T(" to ") );
                                msg.Append( cp_ufile );
                                wxLogMessage( msg );
                            }
                        }
                        
                        else {
                            // Create a dummy ISO8211 file with no real content
                            // Correct this.  We should break the walk, and notify the user  See FS#1406
                            
//                             if( !chain_broken_mssage_shown ){
//                                 OCPNMessageBox(NULL, 
//                                                _("S57 Cell Update chain incomplete.\nENC features may be incomplete or inaccurate.\nCheck the logfile for details."),
//                                                _("OpenCPN Create SENC Warning"), wxOK | wxICON_EXCLAMATION, 30 );
//                                                chain_broken_mssage_shown = true;
//                             }
                            
                            wxString msg( _T("WARNING---ENC Update chain incomplete. Substituting NULL update file: "));
                            msg += ufile.GetFullName();
                            wxLogMessage(msg);
                            wxLogMessage(_T("   Subsequent ENC updates may produce errors.") );
                            wxLogMessage(_T("   This ENC exchange set should be updated and SENCs rebuilt.") );
                            
                            bool bstat;
                            DDFModule *dupdate = new DDFModule;
                            dupdate->Initialize( '3', 'L', 'E', '1', '0', "!!!", 3, 4, 4 );
                            bstat = !( dupdate->Create( cp_ufile.mb_str() ) == 0 );
                            dupdate->Close();
                            
                            if( !bstat ) {
                                wxString msg( _T("   Error creating dummy update file: ") );
                                msg.Append( cp_ufile );
                                wxLogMessage( msg );
                            }
                        }
                        
                        m_tmpup_array.Add( cp_ufile );
            }
        }
        
        //      Extract the date field from the last of the update files
        //      which is by definition a valid, present update file....
        
        wxFileName lastfile( file000 );
        wxString last_sext;
        last_sext.Printf( _T("%03d"), retval );
        lastfile.SetExt( last_sext );
        
        bool bSuccess;
        DDFModule oUpdateModule;
        
        bSuccess = !( oUpdateModule.Open( lastfile.GetFullPath().mb_str(), TRUE ) == 0 );
        
        if( bSuccess ) {
            //      Get publish/update date
            oUpdateModule.Rewind();
            DDFRecord *pr = oUpdateModule.ReadRecord();                     // Record 0
            
            int nSuccess;
            char *u = NULL;
            
            if( pr ) u = (char *) ( pr->GetStringSubfield( "DSID", 0, "ISDT", 0, &nSuccess ) );
            
            if( u ) {
                if( strlen( u ) ) {
                    LastUpdateDate = wxString( u, wxConvUTF8 );
                }
            } else {
                wxDateTime now = wxDateTime::Now();
                LastUpdateDate = now.Format( _T("%Y%m%d") );
            }
        }
    }
    
    delete UpFiles;
    return retval;
}

bool Osenc::GetBaseFileAttr( const wxString &FullPath000 )
{
    
    DDFModule oModule;
    if( !oModule.Open( FullPath000.mb_str() ) ) {
        return false;
    }
    
    oModule.Rewind();
    
    //    Read and parse DDFRecord 0 to get some interesting data
    //    n.b. assumes that the required fields will be in Record 0....  Is this always true?
    
    DDFRecord *pr = oModule.ReadRecord();                               // Record 0
    //    pr->Dump(stdout);
    
    //    Fetch the Geo Feature Count, or something like it....
    m_nGeoRecords = pr->GetIntSubfield( "DSSI", 0, "NOGR", 0 );
    if( !m_nGeoRecords ) {
        errorMessage = _T("GetBaseFileAttr:  DDFRecord 0 does not contain DSSI:NOGR ") ;
        
        m_nGeoRecords = 1;                // backstop
    }
    
    //  Use ISDT(Issue Date) here, which is the same as UADT(Updates Applied) for .000 files
    wxString date000;
    char *u = (char *) ( pr->GetStringSubfield( "DSID", 0, "ISDT", 0 ) );
    if( u ) date000 = wxString( u, wxConvUTF8 );
    else {
        errorMessage =  _T("GetBaseFileAttr:  DDFRecord 0 does not contain DSID:ISDT ");
        
        date000 = _T("20000101");             // backstop, very early, so any new files will update?
    }
    m_date000.ParseFormat( date000, _T("%Y%m%d") );
    if( !m_date000.IsValid() ) m_date000.ParseFormat( _T("20000101"), _T("%Y%m%d") );
    
    m_date000.ResetTime();
    
    //    Fetch the EDTN(Edition) field
    u = (char *) ( pr->GetStringSubfield( "DSID", 0, "EDTN", 0 ) );
    if( u ) m_edtn000 = wxString( u, wxConvUTF8 );
    else {
        errorMessage =  _T("GetBaseFileAttr:  DDFRecord 0 does not contain DSID:EDTN ");
        
        m_edtn000 = _T("1");                // backstop
    }
    
    //m_SE = m_edtn000;
    
    //      Fetch the Native Scale by reading more records until DSPM is found
    m_native_scale = 0;
    for( ; pr != NULL; pr = oModule.ReadRecord() ) {
        if( pr->FindField( "DSPM" ) != NULL ) {
            m_native_scale = pr->GetIntSubfield( "DSPM", 0, "CSCL", 0 );
            break;
        }
    }
    if( !m_native_scale ) {
        errorMessage = _T("GetBaseFileAttr:  ENC not contain DSPM:CSCL ");
        
        m_native_scale = 1000;                // backstop
    }
    
    return true;
}

void Osenc::CreateSENCVectorEdgeTable( FILE * fpOut, S57Reader *poReader )
{
    fprintf( fpOut, "VETableStart\n" );
    
    //  Set up the options, adding RETURN_PRIMITIVES
    char ** papszReaderOptions = NULL;
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_UPDATES, "ON" );
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_LINKAGES, "ON" );
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES, "ON" );
    poReader->SetOptions( papszReaderOptions );
    
    int feid = 0;
    OGRLineString *pLS = NULL;
    OGRGeometry *pGeo;
    OGRFeature *pEdgeVectorRecordFeature = poReader->ReadVector( feid, RCNM_VE );
    
    while( NULL != pEdgeVectorRecordFeature ) {
        int record_id = pEdgeVectorRecordFeature->GetFieldAsInteger( "RCID" );
        fwrite( &record_id, 1, sizeof(int), fpOut );
        
        int nPoints = 0;
        if( pEdgeVectorRecordFeature->GetGeometryRef() != NULL ) {
            pGeo = pEdgeVectorRecordFeature->GetGeometryRef();
            if( pGeo->getGeometryType() == wkbLineString ) {
                pLS = (OGRLineString *) pGeo;
                nPoints = pLS->getNumPoints();
            } else
                nPoints = 0;
        }
        
        //  Transcribe points to a buffer
        
        if(nPoints){
            double *ppd = (double *)malloc(nPoints * sizeof(MyPoint));
            double *ppr = ppd;
            
            for( int i = 0; i < nPoints; i++ ) {
                OGRPoint p;
                pLS->getPoint( i, &p );
                
                //  Calculate SM from chart common reference point
                double easting, northing;
                toSM( p.getY(), p.getX(), m_ref_lat, m_ref_lon, &easting, &northing );
                
                *ppr++ = easting;
                *ppr++ = northing;
            }
            
            //      Reduce the LOD of this linestring
            wxArrayInt index_keep;
            if(nPoints > 5 && (m_LOD_meters > .01)){
                index_keep.Clear();
                index_keep.Add(0);
                index_keep.Add(nPoints-1);
                
                DouglasPeucker(ppd, 0, nPoints-1, m_LOD_meters, &index_keep);
                //               printf("DP Reduction: %d/%d\n", index_keep.GetCount(), nPoints);
                
            }
            else {
                index_keep.Clear();
                for(int i = 0 ; i < nPoints ; i++)
                    index_keep.Add(i);
            }
            
            int nPointReduced = index_keep.GetCount();
            
            //  And make a new array
            double *npp = (double *)malloc(nPoints * 2 * sizeof(double));
            double *npp_run = npp;
            ppr = ppd;
            for(int ip = 0 ; ip < nPoints ; ip++)
            {
                double x = *ppr++;
                double y = *ppr++;
                
                for(unsigned int j=0 ; j < index_keep.GetCount() ; j++){
                    if(index_keep.Item(j) == ip){
                        *npp_run++ = x;
                        *npp_run++ = y;
                        break;
                    }
                }
            }
            
            fwrite( &nPointReduced, 1, sizeof(int), fpOut );
            
            fwrite( npp, nPointReduced, sizeof(MyPoint), fpOut );
            
            free( ppd );
            free( npp );
        }
        else{
            fwrite( &nPoints, 1, sizeof(int), fpOut );
        }
        
        #if 0            
        fwrite( &nPoints, 1, sizeof(int), fpOut );
        
        for( int i = 0; i < nPoints; i++ ) {
            OGRPoint p;
            pLS->getPoint( i, &p );
            
            //  Calculate SM from chart common reference point
            double easting, northing;
            toSM( p.getY(), p.getX(), ref_lat, ref_lon, &easting, &northing );
            
            MyPoint pd;
            pd.x = easting;
            pd.y = northing;
            fwrite( &pd, 1, sizeof(MyPoint), fpOut );
    }
    #endif
    
    //    Next vector record
    feid++;
    delete pEdgeVectorRecordFeature;
    pEdgeVectorRecordFeature = poReader->ReadVector( feid, RCNM_VE );
    }
    
    //    Write a finishing record
    int last_rcid = -1;
    fwrite( &last_rcid, 1, sizeof(int), fpOut );
    fprintf( fpOut, "\nVETableEnd\n" );
    
    //  Reset the options
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES, "OFF" );
    poReader->SetOptions( papszReaderOptions );
    CSLDestroy( papszReaderOptions );
}

void Osenc::CreateSENCConnNodeTable( FILE * fpOut, S57Reader *poReader )
{
    fprintf( fpOut, "VCTableStart\n" );
    
    //  Set up the options, adding RETURN_PRIMITIVES
    char ** papszReaderOptions = NULL;
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_UPDATES, "ON" );
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_LINKAGES, "ON" );
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES, "ON" );
    poReader->SetOptions( papszReaderOptions );
    
    int feid = 0;
    OGRPoint *pP;
    OGRGeometry *pGeo;
    OGRFeature *pConnNodeRecordFeature = poReader->ReadVector( feid, RCNM_VC );
    
    while( NULL != pConnNodeRecordFeature ) {
        if( pConnNodeRecordFeature->GetGeometryRef() != NULL ) {
            pGeo = pConnNodeRecordFeature->GetGeometryRef();
            if( pGeo->getGeometryType() == wkbPoint ) {
                pP = (OGRPoint *) pGeo;
                
                int record_id = pConnNodeRecordFeature->GetFieldAsInteger( "RCID" );
                
                fwrite( &record_id, 1, sizeof(int), fpOut );
                
                //  Calculate SM from chart common reference point
                double easting, northing;
                toSM( pP->getY(), pP->getX(), m_ref_lat, m_ref_lon, &easting, &northing );
                
                MyPoint pd;
                pd.x = easting;
                pd.y = northing;
                fwrite( &pd, 1, sizeof(MyPoint), fpOut );
            }
            //                  else
            //                        int yyp = 5;
        }
        //            else
        //                  int eep = 4;
        
        //    Next vector record
        feid++;
        delete pConnNodeRecordFeature;
        pConnNodeRecordFeature = poReader->ReadVector( feid, RCNM_VC );
    }
    
    //    Write a finishing record
    int last_rcid = -1;
    fwrite( &last_rcid, 1, sizeof(int), fpOut );
    fprintf( fpOut, "\nVCTableEnd\n" );
    
    //  Reset the options
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES, "OFF" );
    poReader->SetOptions( papszReaderOptions );
    CSLDestroy( papszReaderOptions );
}

