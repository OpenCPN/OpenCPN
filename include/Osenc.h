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

#include "mygdal/ogr_s57.h"
#include "mygdal/cpl_csv.h"
#include "chartbase.h"

#include <string.h>
#include <stdint.h>
#include <vector>

WX_DECLARE_OBJARRAY(float *,   SENCFloatPtrArray);

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


//  OSENC V2 record definitions
#define HEADER_SENC_VERSION             1
#define HEADER_CELL_NAME                2
#define HEADER_CELL_PUBLISHDATE         3
#define HEADER_CELL_EDITION             4
#define HEADER_CELL_UPDATEDATE          5
#define HEADER_CELL_UPDATE              6
#define HEADER_CELL_NATIVESCALE         7
#define HEADER_CELL_SENCCREATEDATE      8

#define FEATURE_ID_RECORD               64
#define FEATURE_ATTRIBUTE_RECORD        65

#define FEATURE_GEOMETRY_RECORD_POINT           80
#define FEATURE_GEOMETRY_RECORD_LINE            81
#define FEATURE_GEOMETRY_RECORD_AREA            82
#define FEATURE_GEOMETRY_RECORD_MULTIPOINT      83

#define VECTOR_EDGE_NODE_TABLE_RECORD             96
#define VECTOR_CONNECTED_NODE_TABLE_RECORD        97

#define CELL_COVR_RECORD                        98
#define CELL_NOCOVR_RECORD                      99
#define CELL_EXTENT_RECORD                      100


//--------------------------------------------------------------------------
//      Utility Structures
//--------------------------------------------------------------------------
#pragma pack(push,1)

typedef struct _OSENC_Record_Base{
    uint16_t        record_type;
    uint32_t        record_length;
}OSENC_Record_Base;

typedef struct _OSENC_Record{
    uint16_t        record_type;
    uint32_t        record_length;
    unsigned char   payload;
}OSENC_Record;


typedef struct _OSENC_Feature_Identification_Record_Base{
    uint16_t        record_type;
    uint32_t        record_length;
    uint16_t        feature_type_code;
    uint16_t        feature_ID;
    uint8_t         feature_primitive;
}OSENC_Feature_Identification_Record_Base;


typedef struct _OSENC_Feature_Identification_Record_Payload{
    uint16_t        feature_type_code;
    uint16_t        feature_ID;
    uint8_t         feature_primitive;
}OSENC_Feature_Identification_Record_Payload;

typedef struct _OSENC_Attribute_Record_Base{
    uint16_t        record_type;
    uint32_t        record_length;
    uint16_t        attribute_type;
    unsigned char   attribute_value_type;
}OSENC_Attribute_Record_Base;

typedef struct _OSENC_Attribute_Record{
    uint16_t        record_type;
    uint32_t        record_length;
    uint16_t        attribute_type;
    unsigned char   attribute_value_type;
    void *          payload;
}OSENC_Attribute_Record;

typedef struct _OSENC_Attribute_Record_Payload{
    uint16_t        attribute_type_code;
    unsigned char   attribute_value_type;
    
    union{
        uint32_t        attribute_value_int;
        double          attribute_value_double;
        char *          attribute_value_char_ptr;
    };
}OSENC_Attribute_Record_Payload;


typedef struct _OSENC_PointGeometry_Record{
    uint16_t        record_type;
    uint32_t        record_length;
    double          lat;
    double          lon;
}OSENC_PointGeometry_Record;

typedef struct _OSENC_PointGeometry_Record_Payload{
    double          lat;
    double          lon;
}OSENC_PointGeometry_Record_Payload;


typedef struct _OSENC_MultipointGeometry_Record_Base{
    uint16_t        record_type;
    uint32_t        record_length;
    double          extent_s_lat;
    double          extent_n_lat;
    double          extent_w_lon;
    double          extent_e_lon;
    uint32_t        point_count;
}OSENC_MultipointGeometry_Record_Base;

typedef struct _OSENC_MultipointGeometry_Record_Payload{
    double          extent_s_lat;
    double          extent_n_lat;
    double          extent_w_lon;
    double          extent_e_lon;
    uint32_t        point_count;
    void *          payLoad;
}OSENC_MultipointGeometry_Record_Payload;


typedef struct _OSENC_LineGeometry_Record_Base{
    uint16_t        record_type;
    uint32_t        record_length;
    double          extent_s_lat;
    double          extent_n_lat;
    double          extent_w_lon;
    double          extent_e_lon;
    uint32_t        edgeVector_count;
}OSENC_LineGeometry_Record_Base;

typedef struct _OSENC_LineGeometry_Record_Payload{
    double          extent_s_lat;
    double          extent_n_lat;
    double          extent_w_lon;
    double          extent_e_lon;
    uint32_t        edgeVector_count;
    void *          payLoad;
}OSENC_LineGeometry_Record_Payload;


typedef struct _OSENC_AreaGeometry_Record_Base{
    uint16_t        record_type;
    uint32_t        record_length;
    double          extent_s_lat;
    double          extent_n_lat;
    double          extent_w_lon;
    double          extent_e_lon;
    uint32_t        contour_count;
    uint32_t        triprim_count;
    uint32_t        edgeVector_count;
}OSENC_AreaGeometry_Record_Base;

typedef struct _OSENC_AreaGeometry_Record_Payload{
    double          extent_s_lat;
    double          extent_n_lat;
    double          extent_w_lon;
    double          extent_e_lon;
    uint32_t        contour_count;
    uint32_t        triprim_count;
    uint32_t        edgeVector_count;
    void *          payLoad;
}OSENC_AreaGeometry_Record_Payload;

typedef struct _OSENC_VET_Record{
    uint16_t        record_type;
    uint32_t        record_length;
    unsigned char   payload;
}OSENC_VET_Record;

typedef struct _OSENC_VET_Record_Base{
    uint16_t        record_type;
    uint32_t        record_length;
}OSENC_VET_Record_Base;

typedef struct _OSENC_VCT_Record{
    uint16_t        record_type;
    uint32_t        record_length;
    unsigned char   payload;
}OSENC_VCT_Record;

typedef struct _OSENC_VCT_Record_Base{
    uint16_t        record_type;
    uint32_t        record_length;
}OSENC_VCT_Record_Base;

typedef struct _OSENC_COVR_Record{
    uint16_t        record_type;
    uint32_t        record_length;
    unsigned char   payload;
}_OSENC_COVR_Record;

typedef struct _OSENC_COVR_Record_Base{
    uint16_t        record_type;
    uint32_t        record_length;
}_OSENC_COVR_Record_Base;

typedef struct _OSENC_COVR_Record_Payload{
    uint32_t        point_count;
    float           point_array;
}_OSENC_COVR_Record_Payload;

typedef struct _OSENC_NOCOVR_Record{
    uint16_t        record_type;
    uint32_t        record_length;
    unsigned char   payload;
}_OSENC_NOCOVR_Record;

typedef struct _OSENC_NOCOVR_Record_Base{
    uint16_t        record_type;
    uint32_t        record_length;
}_OSENC_NOCOVR_Record_Base;

typedef struct _OSENC_NOCOVR_Record_Payload{
    uint32_t        point_count;
    float           point_array;
}_OSENC_NOCOVR_Record_Payload;


typedef struct _OSENC_EXTENT_Record{
    uint16_t        record_type;
    uint32_t        record_length;
    double          extent_sw_lat;
    double          extent_sw_lon;
    double          extent_nw_lat;
    double          extent_nw_lon;
    double          extent_ne_lat;
    double          extent_ne_lon;
    double          extent_se_lat;
    double          extent_se_lon;
 }_OSENC_EXTENT_Record;

typedef struct _OSENC_EXTENT_Record_Payload{
     double          extent_sw_lat;
     double          extent_sw_lon;
     double          extent_nw_lat;
     double          extent_nw_lon;
     double          extent_ne_lat;
     double          extent_ne_lon;
     double          extent_se_lat;
     double          extent_se_lon;
 }_OSENC_EXTENT_Record_Payload;
 
 #pragma pack(pop)
 








//  Some special defined attribute type codes
//  Should also be defined in a57attributes.csv

#define ATTRIBUTE_ID_PRIM       50000


const char *MyCSVGetField( const char * pszFilename,
                           const char * pszKeyFieldName,
                           const char * pszKeyFieldValue,
                           CSVCompareCriteria eCriteria,
                           const char * pszTargetField ) ;
                           



//      Fwd Definitions
class wxProgressDialog;
class S57Obj;
class VE_Element;
class VC_Element;
class PolyTessGeo;
class LineGeometryDescriptor;

typedef std::vector<S57Obj *> S57ObjVector;
typedef std::vector<VE_Element *> VE_ElementVector;
typedef std::vector<VC_Element *> VC_ElementVector;

WX_DECLARE_HASH_MAP( int, int, wxIntegerHash, wxIntegerEqual, VectorHelperHash );

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
    
    int ingest200(const wxString &senc_file_name,
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
    
    wxString getSENCFileCreateDate(){ return m_readFileCreateDate; }
    //wxULongLong getFileSize000(){ return m_FileSize000; }
    //wxString getsFileSize000(){ return m_sFileSize000; }

    int getSencReadVersion(){ return m_senc_file_read_version; }
    wxString getSENCReadBaseEdition(){ return m_read_base_edtn; }
    int getSENCReadLastUpdate(){ return m_read_last_applied_update; }
    int getSENCReadScale(){ return m_Chart_Scale; }
    wxString getReadName(){ return m_Name; }
    wxString getReadID(){ return m_ID; }
    Extent &getReadExtent(){ return m_extent; }
    
    SENCFloatPtrArray &getSENCReadAuxPointArray(){ return m_AuxPtrArray;}
    wxArrayInt &getSENCReadAuxPointCountArray(){ return m_AuxCntArray;}
    SENCFloatPtrArray &getSENCReadNOCOVRPointArray(){ return m_NoCovrPtrArray;}
    wxArrayInt &getSENCReadNOCOVRPointCountArray(){ return m_NoCovrCntArray;}
    
    int createSenc200(const wxString& FullPath000, const wxString& SENCFileName, bool b_showProg = true);
    
    void CreateSENCVectorEdgeTableRecord200( FILE * fpOut, S57Reader *poReader );
    void CreateSENCVectorConnectedTableRecord200( FILE * fpOut, S57Reader *poReader );
    
    void InitializePersistentBuffer( void );
    unsigned char *getBuffer( size_t length);
    
        
private:
    void init();
    
    int my_fgets( char *buf, int buf_len_max, wxInputStream& ifs );
    int ingestCell( OGRS57DataSource *poS57DS, const wxString &FullPath000, const wxString &working_dir );
    int ingestCell( const wxString &FullPath000, const wxString &working_dir );
    int ValidateAndCountUpdates( const wxFileName file000, const wxString CopyDir,
                                 wxString &LastUpdateDate, bool b_copyfiles);
    int GetUpdateFileArray(const wxFileName file000, wxArrayString *UpFiles);
    bool GetBaseFileAttr( const wxString &FullPath000 );
    unsigned char *getObjectVectorIndexTable( S57Reader *poReader, OGRFeature *poFeature, int &nEntries );
    
    OGRFeature *GetChartFirstM_COVR( int &catcov, S57Reader *pENCReader, S57ClassRegistrar *poRegistrar );
    OGRFeature *GetChartNextM_COVR( int &catcov, S57Reader *pENCReader );
    bool CreateCOVRTables( S57Reader *pENCReader, S57ClassRegistrar *poRegistrar );
    bool CreateCovrRecords(FILE *fileOut);
    
    void CreateSENCRecord124( OGRFeature *pFeature, FILE * fpOut, int mode, S57Reader *poReader );
    void  CreateSENCVectorEdgeTable(FILE * fpOut, S57Reader *poReader);
    void  CreateSENCConnNodeTable(FILE * fpOut, S57Reader *poReader);

    bool CreateSENCRecord200( OGRFeature *pFeature, FILE * fpOut, int mode, S57Reader *poReader );
    bool WriteFIDRecord200( FILE *fileOut, int nOBJL, int featureID, int prim);
    bool WriteHeaderRecord200( FILE *fileOut, int recordType, std::string payload);
    bool WriteHeaderRecord200( FILE *fileOut, int recordType, uint16_t value);
    bool WriteHeaderRecord200( FILE *fileOut, int recordType, uint32_t value);
    bool CreateAreaFeatureGeometryRecord200( S57Reader *poReader, OGRFeature *pFeature, FILE *fpOut );
    bool CreateLineFeatureGeometryRecord200( S57Reader *poReader, OGRFeature *pFeature, FILE *fpOut );
    bool CreateMultiPointFeatureGeometryRecord200( OGRFeature *pFeature, FILE *fpOut);
    
    std::string GetFeatureAcronymFromTypecode( int typeCode );
    std::string GetAttributeAcronymFromTypecode( int typeCode );
    
    PolyTessGeo *BuildPolyTessGeo(_OSENC_AreaGeometry_Record_Payload *record, unsigned char **bytes_consumed );
    
    LineGeometryDescriptor *BuildLineGeometry( _OSENC_LineGeometry_Record_Payload *pPayload );
    
    wxString            errorMessage;
    
    wxString            m_Name;
    wxString m_ID;
    wxString            m_FullPath000;
    
    int                 m_Chart_Scale;
    int                 m_senc_file_read_version;
    int                 m_senc_file_create_version;
    wxString            m_read_base_edtn;
    int                 m_read_last_applied_update;
    
    S57Reader           *poReader;
    OGRS57DataSource    *poS57DS;
    
    wxDateTime          m_date000;
    wxString            m_sdate000;
    
    wxString            m_edtn000;
    int                 m_nGeoRecords;
    int                 m_last_applied_update;
    wxString            m_LastUpdateDate;
    int                 m_native_scale;
    wxString            m_readFileCreateDate;
    
    double              m_ref_lat, m_ref_lon;             // Common reference point, derived from FullExtent
    VectorHelperHash    m_vector_helper_hash;
    double              m_LOD_meters;
    S57ClassRegistrar   *m_poRegistrar;
    wxArrayString       m_tmpup_array;
    
    wxProgressDialog    *s_ProgDialog;
    
    
    unsigned char *     pBuffer;
    size_t              bufferSize;
    

    Extent              m_extent;
    
    //  Clone of Chartbase structures of the same name and purpose
    //  Use mainly for SENC creation for ENC(.000) file
    int         m_nCOVREntries;                       // number of coverage table entries
    int         *m_pCOVRTablePoints;                  // int table of number of points in each coverage table entry
    float       **m_pCOVRTable;                       // table of pointers to list of floats describing valid COVR
    
    int         m_nNoCOVREntries;                       // number of NoCoverage table entries
    int         *m_pNoCOVRTablePoints;                  // int table of number of points in each NoCoverage table entry
    float       **m_pNoCOVRTable;                       // table of pointers to list of floats describing valid NOCOVR
    
    
    //  Arrays used to accumulate coverage regions on oSENC load
    SENCFloatPtrArray     m_AuxPtrArray;
    wxArrayInt            m_AuxCntArray;
    SENCFloatPtrArray     m_NoCovrPtrArray;
    wxArrayInt            m_NoCovrCntArray;
    
    
};


#endif          // Guard
