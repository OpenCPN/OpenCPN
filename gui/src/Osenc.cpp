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

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include <setjmp.h>

#include <wx/wfstream.h>
#include <wx/filename.h>
#include <wx/progdlg.h>

#include "Osenc.h"
#include "s52s57.h"
#include "s57chart.h"  // for one static method
#include "model/cutil.h"
#include "s57RegistrarMgr.h"
#include "gdal/cpl_csv.h"
#include "ogr_s57.h"
#include "gdal/cpl_string.h"
#include "LOD_reduce.h"

#include "mygeom.h"
#include "model/georef.h"
#include "gui_lib.h"
#include <mutex>

extern s57RegistrarMgr *m_pRegistrarMan;
extern wxString g_csv_locn;
extern bool g_bGDAL_Debug;

bool chain_broken_mssage_shown = false;

using namespace std;

#include <wx/arrimpl.cpp>
WX_DEFINE_ARRAY(float *, MyFloatPtrArray);

#define MAX_VECTOR_POINTS 1000

#ifndef __WXMSW__
sigjmp_buf env_osenc_ogrf;  // the context saved by sigsetjmp();
#endif

std::mutex m;

/************************************************************************/
/*                       OpenCPN_OGRErrorHandler()                      */
/*                       Use Global wxLog Class                         */
/************************************************************************/
bool g_OsencVerbose;

void OpenCPN_OGR_OSENC_ErrorHandler(CPLErr eErrClass, int nError,
                                    const char *pszErrorMsg) {
#define ERR_BUF_LEN 2000

  char buf[ERR_BUF_LEN + 1];

  if (eErrClass == CE_Debug) {
    if (g_OsencVerbose) sprintf(buf, " %s", pszErrorMsg);
  } else if (eErrClass == CE_Warning)
    sprintf(buf, "   Warning %d: %s\n", nError, pszErrorMsg);
  else
    sprintf(buf, "   ERROR %d: %s\n", nError, pszErrorMsg);

  if (g_bGDAL_Debug || (CE_Debug != eErrClass)) {  // log every warning or error
    wxString msg(buf, wxConvUTF8);
    wxLogMessage(msg);
  }

  //      Do not simply return on CE_Fatal errors, as we don't want to abort()

#ifndef __WXMSW__
  if (eErrClass == CE_Fatal) {
    longjmp(env_osenc_ogrf, 1);  // jump back to the setjmp() point
  }
#endif
}

//--------------------------------------------------------------------------
//      Osenc_instreamFile implementation
//      A simple file stream implementation based on wxFFileInputStream
//--------------------------------------------------------------------------
Osenc_instreamFile::Osenc_instreamFile() { Init(); }

Osenc_instreamFile::~Osenc_instreamFile() { delete m_instream; }

bool Osenc_instreamFile::Open(const wxString &senc_file_name) {
  m_instream = new wxFFileInputStream(senc_file_name);
  return m_instream->IsOk();
}

void Osenc_instreamFile::Close() {}

Osenc_instream &Osenc_instreamFile::Read(void *buffer, size_t size) {
  if (m_instream) m_ok = m_instream->Read(buffer, size).IsOk();

  return *this;
}

bool Osenc_instreamFile::IsOk() {
  if (m_instream) m_ok = m_instream->IsOk();

  return m_ok;
}

bool Osenc_instreamFile::isAvailable() { return true; }

void Osenc_instreamFile::Shutdown() {}

void Osenc_instreamFile::Init() {
  m_instream = NULL;
  m_ok = false;
}

//--------------------------------------------------------------------------
//      Osenc_outstreamFile implementation
//      A simple file stream implementation based on wxFFileOutStream
//--------------------------------------------------------------------------
Osenc_outstreamFile::Osenc_outstreamFile() { Init(); }

Osenc_outstreamFile::~Osenc_outstreamFile() { delete m_outstream; }

bool Osenc_outstreamFile::Open(const wxString &file) {
  Init();
  m_outstream = new wxFFileOutputStream(file);
  if (m_outstream) m_ok = m_outstream->IsOk();

  return m_ok;
}

void Osenc_outstreamFile::Close() {
  if (m_outstream) m_ok = m_outstream->Close();
}

Osenc_outstream &Osenc_outstreamFile::Write(const void *buffer, size_t size) {
  if (m_outstream) m_ok = m_outstream->Write(buffer, size).IsOk();

  return *this;
}

bool Osenc_outstreamFile::IsOk() {
  if (m_outstream) m_ok = m_outstream->IsOk();

  return m_ok;
}

void Osenc_outstreamFile::Init() {
  m_outstream = NULL;
  m_ok = false;
}

//--------------------------------------------------------------------------
//      Osenc implementation
//--------------------------------------------------------------------------

Osenc::Osenc() { init(); }

Osenc::~Osenc() {
  if (m_bPrivateRegistrar) delete m_poRegistrar;

  // Free the coverage arrays, if they exist
  SENCFloatPtrArray &AuxPtrArray = getSENCReadAuxPointArray();
  std::vector<int> &AuxCntArray = getSENCReadAuxPointCountArray();
  int nCOVREntries = AuxCntArray.size();
  for (unsigned int j = 0; j < (unsigned int)nCOVREntries; j++) {
    free(AuxPtrArray[j]);
  }

  SENCFloatPtrArray &AuxNoPtrArray = getSENCReadNOCOVRPointArray();
  std::vector<int> &AuxNoCntArray = getSENCReadNOCOVRPointCountArray();
  int nNoCOVREntries = AuxNoCntArray.size();
  for (unsigned int j = 0; j < (unsigned int)nNoCOVREntries; j++) {
    free(AuxNoPtrArray[j]);
  }

  free(pBuffer);

  for (unsigned int j = 0; j < (unsigned int)m_nNoCOVREntries; j++)
    free(m_pNoCOVRTable[j]);

  for (unsigned int j = 0; j < (unsigned int)m_nCOVREntries; j++)
    free(m_pCOVRTable[j]);

  free(m_pCOVRTablePoints);
  free(m_pCOVRTable);
  free(m_pNoCOVRTablePoints);
  free(m_pNoCOVRTable);
  delete m_UpFiles;
  CPLPopErrorHandler();
}

void Osenc::init(void) {
  m_LOD_meters = 0;
  m_poRegistrar = NULL;
  m_bPrivateRegistrar = false;
  m_senc_file_read_version = 0;
  m_ProgDialog = NULL;
  InitializePersistentBuffer();

  m_ref_lat = 0;
  m_ref_lon = 0;

  m_read_base_edtn = _T("-1");

  m_nNoCOVREntries = 0;
  m_nCOVREntries = 0;
  m_pCOVRTablePoints = NULL;
  m_pCOVRTable = NULL;
  m_pNoCOVRTablePoints = NULL;
  m_pNoCOVRTable = NULL;

  m_pauxOutstream = NULL;
  m_pauxInstream = NULL;
  m_pOutstream = NULL;
  m_pInstream = NULL;
  m_UpFiles = nullptr;

  m_bVerbose = true;
  g_OsencVerbose = true;
  m_NoErrDialog = false;

  //      Insert my local error handler to catch OGR errors,
  //      Especially CE_Fatal type errors
  //      Discovered/debugged on US5MD11M.017.  VI 548 geometry deleted
  CPLPushErrorHandler(OpenCPN_OGR_OSENC_ErrorHandler);

  lockCR = std::unique_lock<std::mutex>(m, std::defer_lock);
}

void Osenc::setVerbose(bool verbose) {
  m_bVerbose = verbose;
  g_OsencVerbose = verbose;
}

int Osenc::ingestHeader(const wxString &senc_file_name) {
  //  Read oSENC header records, stopping at the first Feature_ID record
  //  Then check to see if everything is defined as required.

  int ret_val = SENC_NO_ERROR;  // default is OK

  wxFileName fn(senc_file_name);

  //    Sanity check for existence of file

  //    int nProg = 0;

  //     wxString ifs( senc_file_name );
  //
  //     wxFFileInputStream fpx_u( ifs );
  //     if (!fpx_u.IsOk()) {
  //         return ERROR_SENCFILE_NOT_FOUND;
  //     }
  //     wxBufferedInputStream fpx( fpx_u );

  //    Sanity check for existence of file
  Osenc_instreamFile fpx;
  fpx.Open(senc_file_name);
  if (!fpx.IsOk()) return ERROR_SENCFILE_NOT_FOUND;

  //  For identification purposes, the very first record must be the OSENC
  //  Version Number Record
  OSENC_Record_Base record;

  fpx.Read(&record, sizeof(OSENC_Record_Base));
  if (!fpx.IsOk()) {
    return ERROR_SENCFILE_NOT_FOUND;
  }

  // Check Record
  if (HEADER_SENC_VERSION != record.record_type) {
    return ERROR_SENCFILE_NOT_FOUND;
  }

  //  This is the correct record type (OSENC Version Number Record), so read it
  unsigned char *buf =
      getBuffer(record.record_length - sizeof(OSENC_Record_Base));
  if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base)).IsOk()) {
    return ERROR_SENCFILE_NOT_FOUND;
  }
  uint16_t *pint = (uint16_t *)buf;
  m_senc_file_read_version = *pint;

  //  Read the rest of the records in the header
  int dun = 0;

  while (!dun) {
    //      Read a record Header
    OSENC_Record_Base record;

    //        off = fpx.TellI();

    fpx.Read(&record, sizeof(OSENC_Record_Base));
    if (!fpx.IsOk()) {
      dun = 1;
      break;
    }

    // Process Records
    switch (record.record_type) {
      case HEADER_SENC_VERSION: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }
        uint16_t *pint = (uint16_t *)buf;
        m_senc_file_read_version = *pint;
        break;
      }
      case HEADER_CELL_NAME: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }
        m_Name = wxString(buf, wxConvUTF8);
        break;
      }
      case HEADER_CELL_PUBLISHDATE: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }
        break;
      }

      case HEADER_CELL_EDITION: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }
        uint16_t *pint = (uint16_t *)buf;
        m_read_base_edtn.Printf(_T("%d"), *pint);
        break;
      }

      case HEADER_CELL_UPDATEDATE: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }
        break;
      }

      case HEADER_CELL_UPDATE: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }

        uint16_t *pint = (uint16_t *)buf;
        m_read_last_applied_update = *pint;
        break;
      }

      case HEADER_CELL_NATIVESCALE: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }
        uint32_t *pint = (uint32_t *)buf;
        m_Chart_Scale = *pint;

        break;
      }

      case HEADER_CELL_SENCCREATEDATE: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }
        m_readFileCreateDate = wxString(buf, wxConvUTF8);

        break;
      }

      case CELL_EXTENT_RECORD: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }
        _OSENC_EXTENT_Record_Payload *pPayload =
            (_OSENC_EXTENT_Record_Payload *)buf;
        m_extent.NLAT = pPayload->extent_nw_lat;
        m_extent.SLAT = pPayload->extent_se_lat;
        m_extent.WLON = pPayload->extent_nw_lon;
        m_extent.ELON = pPayload->extent_se_lon;

        break;
      }

      case CELL_COVR_RECORD: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }

        _OSENC_COVR_Record_Payload *pPayload =
            (_OSENC_COVR_Record_Payload *)buf;

        int point_count = pPayload->point_count;
        m_AuxCntArray.push_back(point_count);

        float *pf = (float *)malloc(point_count * 2 * sizeof(float));
        memcpy(pf, &pPayload->point_array, point_count * 2 * sizeof(float));
        m_AuxPtrArray.Add(pf);

        break;
      }

      case CELL_NOCOVR_RECORD: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }

        _OSENC_NOCOVR_Record_Payload *pPayload =
            (_OSENC_NOCOVR_Record_Payload *)buf;

        int point_count = pPayload->point_count;
        m_NoCovrCntArray.push_back(point_count);

        float *pf = (float *)malloc(point_count * 2 * sizeof(float));
        memcpy(pf, &pPayload->point_array, point_count * 2 * sizeof(float));
        m_NoCovrPtrArray.Add(pf);

        break;
      }

      case FEATURE_ID_RECORD: {
        dun = 1;
        break;
      }

      default: {
        dun = 1;
        break;
      }

    }  // switch

  }  // while

  return ret_val;
}

std::string Osenc::GetFeatureAcronymFromTypecode(int typeCode) {
  if (m_pRegistrarMan) {
    std::string acronym = m_pRegistrarMan->getFeatureAcronym(typeCode);
    return acronym.c_str();
  } else
    return "";
}

std::string Osenc::GetAttributeAcronymFromTypecode(int typeCode) {
  if (m_pRegistrarMan)
    return m_pRegistrarMan->getAttributeAcronym(typeCode);
  else
    return "";
}

int Osenc::ingest200(const wxString &senc_file_name,
                     S57ObjVector *pObjectVector, VE_ElementVector *pVEArray,
                     VC_ElementVector *pVCArray) {
  int ret_val = SENC_NO_ERROR;  // default is OK

  //    wxFileName fn(senc_file_name);
  //    m_ID = fn.GetName();                          // This will be the NOAA
  //    File name, usually

  //    int nProg = 0;

  //     wxString ifs( senc_file_name );
  //
  //     wxFFileInputStream fpx_u( ifs );
  //     if (!fpx_u.IsOk()) {
  //         return ERROR_SENCFILE_NOT_FOUND;
  //     }
  //     wxBufferedInputStream fpx( fpx_u );

  //    Sanity check for existence of file
  Osenc_instreamFile fpx;
  fpx.Open(senc_file_name);
  if (!fpx.IsOk()) return ERROR_SENCFILE_NOT_FOUND;

  S57Obj *obj = 0;
  int featureID;

  int dun = 0;

  while (!dun) {
    //      Read a record Header
    OSENC_Record_Base record;

    //        long off = fpx.TellI();

    fpx.Read(&record, sizeof(OSENC_Record_Base));
    if (!fpx.IsOk()) {
      dun = 1;
      break;
    }

    // Process Records
    switch (record.record_type) {
      case HEADER_SENC_VERSION: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }
        uint16_t *pint = (uint16_t *)buf;
        m_senc_file_read_version = *pint;
        break;
      }
      case HEADER_CELL_NAME: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }
        m_Name = wxString(buf, wxConvUTF8);
        break;
      }
      case HEADER_CELL_PUBLISHDATE: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }
        m_sdate000 = wxString(buf, wxConvUTF8);
        break;
      }

      case HEADER_CELL_EDITION: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }
        uint16_t *pint = (uint16_t *)buf;
        m_read_base_edtn.Printf(_T("%d"), *pint);

        break;
      }

      case HEADER_CELL_UPDATEDATE: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }
        m_LastUpdateDate = wxString(buf, wxConvUTF8);
        break;
      }

      case HEADER_CELL_UPDATE: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }
        uint16_t *pint = (uint16_t *)buf;
        m_read_last_applied_update = *pint;

        break;
      }

      case HEADER_CELL_NATIVESCALE: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }
        uint32_t *pint = (uint32_t *)buf;
        m_Chart_Scale = *pint;
        break;
      }

      case HEADER_CELL_SENCCREATEDATE: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }
        break;
      }

      case CELL_EXTENT_RECORD: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }
        _OSENC_EXTENT_Record_Payload *pPayload =
            (_OSENC_EXTENT_Record_Payload *)buf;
        m_extent.NLAT = pPayload->extent_nw_lat;
        m_extent.SLAT = pPayload->extent_se_lat;
        m_extent.WLON = pPayload->extent_nw_lon;
        m_extent.ELON = pPayload->extent_se_lon;

        //  We declare the ref_lat/ref_lon to be the centroid of the extents
        //  This is how the SENC was created....
        m_ref_lat = (m_extent.NLAT + m_extent.SLAT) / 2.;
        m_ref_lon = (m_extent.ELON + m_extent.WLON) / 2.;

        break;
      }

      case CELL_COVR_RECORD: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }

        break;
      }

      case CELL_NOCOVR_RECORD: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }

        break;
      }

      case FEATURE_ID_RECORD: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }

        // Starting definition of a new feature
        _OSENC_Feature_Identification_Record_Payload *pPayload =
            (_OSENC_Feature_Identification_Record_Payload *)buf;

        // Get the Feature type code and ID
        int featureTypeCode = pPayload->feature_type_code;
        featureID = pPayload->feature_ID;

        // TODO
        //                if(207 == featureID)
        //                    int yyp = 4;

        //  Look up the FeatureName from the Registrar

        std::string acronym = GetFeatureAcronymFromTypecode(featureTypeCode);

        // TODO debugging
        //                 printf("%s\n", acronym.c_str());
        //                 if(!strncmp(acronym.c_str(), "BOYLAT", 6))
        //                     int yyp = 4;

        if (acronym.length()) {
          obj = new S57Obj(acronym.c_str());
          obj->Index = featureID;

          pObjectVector->push_back(obj);
        }

        break;
      }

      case FEATURE_ATTRIBUTE_RECORD: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }

        // Get the payload
        OSENC_Attribute_Record_Payload *pPayload =
            (OSENC_Attribute_Record_Payload *)buf;

        int attributeTypeCode = pPayload->attribute_type_code;

        //  Handle Special cases...

        //  The primitive type of the Feature is encoded in the SENC as an
        //  attribute of defined type.
        //                if( ATTRIBUTE_ID_PRIM == attributeTypeCode ){
        //                    primitiveType = pPayload->attribute_value_int;
        //                }

        //  Get the standard attribute acronym
        std::string acronym =
            GetAttributeAcronymFromTypecode(attributeTypeCode);

        int attributeValueType = pPayload->attribute_value_type;

        if (acronym.length()) {
          switch (attributeValueType) {
            case 0: {
              uint32_t val = pPayload->attribute_value_int;
              if (obj) {
                obj->AddIntegerAttribute(acronym.c_str(), val);
              }
              break;
            }

            case 1:  // Integer list
            {
              // Calculate the number of elements from the record size
              // int nCount = (record.record_length -
              // sizeof(_OSENC_Attribute_Record)) ;

              break;
            }
            case 2:  // Single double precision real
            {
              double val = pPayload->attribute_value_double;
              if (obj) obj->AddDoubleAttribute(acronym.c_str(), val);
              break;
            }

            case 3:  // List of double precision real
            {
              // TODO
              break;
            }

            case 4:  // Ascii String
            {
              char *val = (char *)&pPayload->attribute_value_char_ptr;
              if (obj) obj->AddStringAttribute(acronym.c_str(), val);

              break;
            }

            default:
              break;
          }
        }

        break;
      }

      case FEATURE_GEOMETRY_RECORD_POINT: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }

        // Get the payload
        _OSENC_PointGeometry_Record_Payload *pPayload =
            (_OSENC_PointGeometry_Record_Payload *)buf;

        if (obj) {
          obj->SetPointGeometry(pPayload->lat, pPayload->lon, m_ref_lat,
                                m_ref_lon);
        }

        break;
      }

      case FEATURE_GEOMETRY_RECORD_AREA: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }

        // Get the payload
        _OSENC_AreaGeometry_Record_Payload *pPayload =
            (_OSENC_AreaGeometry_Record_Payload *)buf;

        if (obj) {
          unsigned char *next_byte;
          PolyTessGeo *pPTG = BuildPolyTessGeo(pPayload, &next_byte);

          obj->SetAreaGeometry(pPTG, m_ref_lat, m_ref_lon);

          //  Set the Line geometry for the Feature
          LineGeometryDescriptor Descriptor;

          //  Copy some simple stuff
          Descriptor.extent_e_lon = pPayload->extent_e_lon;
          Descriptor.extent_w_lon = pPayload->extent_w_lon;
          Descriptor.extent_s_lat = pPayload->extent_s_lat;
          Descriptor.extent_n_lat = pPayload->extent_n_lat;

          Descriptor.indexCount = pPayload->edgeVector_count;

          // Copy the line index table, which in this case is offset in the
          // payload
          Descriptor.indexTable =
              (int *)malloc(pPayload->edgeVector_count * 3 * sizeof(int));
          memcpy(Descriptor.indexTable, next_byte,
                 pPayload->edgeVector_count * 3 * sizeof(int));

          obj->SetLineGeometry(&Descriptor, GEO_AREA, m_ref_lat, m_ref_lon);
        }

        break;
      }

      case FEATURE_GEOMETRY_RECORD_LINE: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }

        // Get the payload & parse it
        _OSENC_LineGeometry_Record_Payload *pPayload =
            (_OSENC_LineGeometry_Record_Payload *)buf;
        LineGeometryDescriptor lD;

        //  Copy some simple stuff
        lD.extent_e_lon = pPayload->extent_e_lon;
        lD.extent_w_lon = pPayload->extent_w_lon;
        lD.extent_s_lat = pPayload->extent_s_lat;
        lD.extent_n_lat = pPayload->extent_n_lat;

        lD.indexCount = pPayload->edgeVector_count;

        // Copy the payload tables
        lD.indexTable =
            (int *)malloc(pPayload->edgeVector_count * 3 * sizeof(int));
        memcpy(lD.indexTable, &pPayload->payLoad,
               pPayload->edgeVector_count * 3 * sizeof(int));

        if (obj) obj->SetLineGeometry(&lD, GEO_LINE, m_ref_lat, m_ref_lon);

        break;
      }

      case FEATURE_GEOMETRY_RECORD_MULTIPOINT: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }

        // Get the payload & parse it
        OSENC_MultipointGeometry_Record_Payload *pPayload =
            (OSENC_MultipointGeometry_Record_Payload *)buf;

        //  Set the Multipoint geometry for the Feature
        MultipointGeometryDescriptor Descriptor;

        //  Copy some simple stuff
        Descriptor.extent_e_lon = pPayload->extent_e_lon;
        Descriptor.extent_w_lon = pPayload->extent_w_lon;
        Descriptor.extent_s_lat = pPayload->extent_s_lat;
        Descriptor.extent_n_lat = pPayload->extent_n_lat;

        Descriptor.pointCount = pPayload->point_count;
        Descriptor.pointTable = &pPayload->payLoad;

        if (obj) obj->SetMultipointGeometry(&Descriptor, m_ref_lat, m_ref_lon);

        break;
      }

      case VECTOR_EDGE_NODE_TABLE_RECORD: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }

        //  Parse the buffer
        uint8_t *pRun = (uint8_t *)buf;

        // The Feature(Object) count
        int nCount = *(int *)pRun;

        pRun += sizeof(int);

        for (int i = 0; i < nCount; i++) {
          int featureIndex = *(int *)pRun;
          pRun += sizeof(int);

          int pointCount = *(int *)pRun;
          pRun += sizeof(int);

          float *pPoints = NULL;
          if (pointCount) {
            pPoints = (float *)malloc(pointCount * 2 * sizeof(float));
            memcpy(pPoints, pRun, pointCount * 2 * sizeof(float));
          }
          pRun += pointCount * 2 * sizeof(float);

          VE_Element *pvee = new VE_Element;
          pvee->index = featureIndex;
          pvee->nCount = pointCount;
          pvee->pPoints = pPoints;
          pvee->max_priority = 0;  // Default

          pVEArray->push_back(pvee);
        }

        break;
      }

      case VECTOR_CONNECTED_NODE_TABLE_RECORD: {
        unsigned char *buf =
            getBuffer(record.record_length - sizeof(OSENC_Record_Base));
        if (!fpx.Read(buf, record.record_length - sizeof(OSENC_Record_Base))
                 .IsOk()) {
          dun = 1;
          break;
        }

        //  Parse the buffer
        uint8_t *pRun = (uint8_t *)buf;

        // The Feature(Object) count
        int nCount = *(int *)pRun;
        pRun += sizeof(int);

        for (int i = 0; i < nCount; i++) {
          int featureIndex = *(int *)pRun;
          pRun += sizeof(int);

          float *pPoint = (float *)malloc(2 * sizeof(float));
          memcpy(pPoint, pRun, 2 * sizeof(float));
          pRun += 2 * sizeof(float);

          VC_Element *pvce = new VC_Element;
          pvce->index = featureIndex;
          pvce->pPoint = pPoint;

          pVCArray->push_back(pvce);
        }

        break;
      }

      default:
        break;

    }  // switch
  }

  return ret_val;
}

int Osenc::ingestCell(OGRS57DataSource *poS57DS, const wxString &FullPath000,
                      const wxString &working_dir) {
  //      Analyze Updates
  //      The OGR library will apply updates automatically, if enabled.
  //      Alternatively, we can explicitely find and apply updates from any
  //      source directory. We need to keep track of the last sequential update
  //      applied, to look out for new updates

  wxString LastUpdateDate = m_date000.Format(_T("%Y%m%d"));

  int available_updates =
      ValidateAndCountUpdates(FullPath000, working_dir, LastUpdateDate, true);
  m_LastUpdateDate =
      LastUpdateDate;  // tentative, adjusted later on failure of update

  if (m_bVerbose && (available_updates > m_UPDN)) {
    wxString msg1;
    msg1.Printf(
        _T("Preparing to apply ENC updates, target final update is %3d."),
        available_updates);
    wxLogMessage(msg1);
  }

  wxString sobj;

  //  Here comes the actual ISO8211 file reading

  //  Set up the options
  char **papszReaderOptions = NULL;
  //    papszReaderOptions = CSLSetNameValue(papszReaderOptions, S57O_LNAM_REFS,
  //    "ON" );
  //    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_UPDATES,
  //    "ON" );
  papszReaderOptions =
      CSLSetNameValue(papszReaderOptions, S57O_RETURN_LINKAGES, "ON");
  papszReaderOptions =
      CSLSetNameValue(papszReaderOptions, S57O_RETURN_PRIMITIVES, "ON");
  poS57DS->SetOptionList(papszReaderOptions);

  //      Open the OGRS57DataSource
  //      This will ingest the .000 file from the working dir

  bool b_current_debug = g_bGDAL_Debug;
  g_bGDAL_Debug = m_bVerbose;

  // Form the .000 filename
  wxString s0_file = working_dir;
  if (s0_file.Last() != wxFileName::GetPathSeparator())
    s0_file.Append(wxFileName::GetPathSeparator());
  wxFileName f000(FullPath000);

  s0_file.Append(f000.GetFullName());

  if (poS57DS->Open(s0_file.mb_str(), TRUE, NULL)) return 1;

  //      Get a pointer to the reader
  S57Reader *poReader = poS57DS->GetModule(0);

  m_last_applied_update = m_UPDN;
  wxString last_successful_update_file;

  // Apply the updates...
  for (unsigned int i_up = 0; i_up < m_tmpup_array.GetCount(); i_up++) {
    wxFileName fn(m_tmpup_array[i_up]);
    wxString ext = fn.GetExt();
    long n_upd;
    ext.ToLong(&n_upd);

    if (n_upd > 0) {  // .000 is the base, not an update
      DDFModule oUpdateModule;
      if (!oUpdateModule.Open(m_tmpup_array[i_up].mb_str(), FALSE)) {
        break;
      }
      int upResult = poReader->ApplyUpdates(&oUpdateModule, n_upd);
      if (upResult) {
        break;
      }
      m_last_applied_update = n_upd;
      last_successful_update_file = m_tmpup_array[i_up];
    }
  }

  //  Check for bad/broken update chain....
  //  It is a "warning" condition if an update fails.
  //  We use the cell with all good updates applied so far, and so inform the
  //  user. "Better a slightly out-of-date chart than no chart at all..."

  // The logic will attempt to build a SENC on each instance of OCPN, so
  // eventually the
  //  updates may be corrected, and the chart SENC is built correctly.
  //  Or, the update files following the last good update may be manually
  //  deleted.

  if ((available_updates > 0) && (m_last_applied_update != available_updates)) {
    if (last_successful_update_file.Length()) {
      //  Get the update date from the last good update module
      bool bSuccess;
      DDFModule oUpdateModule;
      wxString LastGoodUpdateDate;
      wxDateTime now = wxDateTime::Now();
      LastGoodUpdateDate = now.Format(_T("%Y%m%d"));

      bSuccess = !(
          oUpdateModule.Open(last_successful_update_file.mb_str(), TRUE) == 0);

      if (bSuccess) {
        //      Get publish/update date
        oUpdateModule.Rewind();
        DDFRecord *pr = oUpdateModule.ReadRecord();  // Record 0

        int nSuccess;
        char *u = NULL;

        if (pr)
          u = (char *)(pr->GetStringSubfield("DSID", 0, "ISDT", 0, &nSuccess));

        if (u) {
          if (strlen(u)) {
            LastGoodUpdateDate = wxString(u, wxConvUTF8);
          }
        }
        m_LastUpdateDate = LastGoodUpdateDate;
      }

      // Inform the user
      wxString msg(
          _T("WARNING---ENC Update failed.  Last valid update file is:"));
      msg += last_successful_update_file.mb_str();
      wxLogMessage(msg);
      wxLogMessage(
          _T("   This ENC exchange set should be updated and SENCs rebuilt."));

      if (!m_NoErrDialog) {
        OCPNMessageBox(
            NULL,
            _("S57 Cell Update failed.\nENC features may be incomplete or "
              "inaccurate.\n\nCheck the logfile for details."),
            _("OpenCPN Create SENC Warning"), wxOK | wxICON_EXCLAMATION, 5);
      }
    } else {  // no updates applied.
      if (!m_NoErrDialog)
        OCPNMessageBox(NULL,
                       _("S57 Cell Update failed.\nNo updates could be "
                         "applied.\nENC features may be incomplete or "
                         "inaccurate.\n\nCheck the logfile for details."),
                       _("OpenCPN Create SENC Warning"),
                       wxOK | wxICON_EXCLAMATION, 5);
    }
  }

  //  Unset verbose debug option
  g_bGDAL_Debug = b_current_debug;

  //      Update the options, removing the RETURN_PRIMITIVES flags
  //      This flag needed to be set on ingest() to create the proper field
  //      defns, but cleared to fetch normal features

  papszReaderOptions =
      CSLSetNameValue(papszReaderOptions, S57O_RETURN_PRIMITIVES, "OFF");
  poReader->SetOptions(papszReaderOptions);
  CSLDestroy(papszReaderOptions);

  wxRemoveFile(s0_file);

  return 0;
}

int Osenc::ValidateAndCountUpdates(const wxFileName file000,
                                   const wxString CopyDir,
                                   wxString &LastUpdateDate, bool b_copyfiles) {
  int retval = 0;
  wxFileName last_up_added;

  //       wxString DirName000 = file000.GetPath((int)(wxPATH_GET_SEPARATOR |
  //       wxPATH_GET_VOLUME)); wxDir dir(DirName000);
  m_UpFiles = new wxArrayString;
  retval =
      s57chart::GetUpdateFileArray(file000, m_UpFiles, m_date000, m_edtn000);
  int upmax = retval;

  if (m_UpFiles->GetCount()) {
    //      The s57reader of ogr requires that update set be sequentially
    //      complete to perform all the updates.  However, some NOAA ENC
    //      distributions are not complete, as apparently some interim updates
    //      have been  withdrawn. Example:  as of 20 Dec, 2005, the update set
    //      for US5MD11M.000 includes US5MD11M.017, ...018, and ...019.  Updates
    //      001 through 016 are missing.
    //
    //      Workaround.
    //      Create temporary dummy update files to fill out the set before
    //      invoking ogr file open/ingest.  Delete after SENC file create
    //      finishes. Set starts with .000, which has the effect of copying the
    //      base file to the working dir

    //        bool chain_broken_mssage_shown = false;

    if (b_copyfiles) {
      unsigned int jup = 0;
      for (int iff = 0; iff < retval + 1; iff++) {
        wxString upFile;
        wxString targetFile;

        if (jup < m_UpFiles->GetCount()) upFile = m_UpFiles->Item(jup);
        wxFileName upCheck(upFile);
        long tl = -1;
        wxString text = upCheck.GetExt();
        text.ToLong(&tl);
        if (tl == iff) {
          targetFile = upFile;
          jup++;  // used this one
        } else {
          targetFile = file000.GetFullName();  // ext will be updated
        }

        wxFileName ufile(targetFile);
        wxString sext;
        sext.Printf(_T("%03d"), iff);
        ufile.SetExt(sext);

        //      Create the target update file name
        wxString cp_ufile = CopyDir;
        if (cp_ufile.Last() != ufile.GetPathSeparator())
          cp_ufile.Append(ufile.GetPathSeparator());

        cp_ufile.Append(ufile.GetFullName());

        wxString tfile = ufile.GetFullPath();

        //      Explicit check for a short update file, possibly left over from
        //      a crash...
        int flen = 0;
        if (ufile.FileExists()) {
          wxFile uf(ufile.GetFullPath());
          if (uf.IsOpened()) {
            flen = uf.Length();
            uf.Close();
          }
        }

        if (ufile.FileExists() &&
            (flen > 25))  // a valid update file or base file
        {
          //      Copy the valid file to the SENC directory
          bool cpok = wxCopyFile(ufile.GetFullPath(), cp_ufile);
          if (!cpok) {
            wxString msg(_T("   Cannot copy temporary working ENC file "));
            msg.Append(ufile.GetFullPath());
            msg.Append(_T(" to "));
            msg.Append(cp_ufile);
            wxLogMessage(msg);
          }
        }

        else {
          // Create a dummy ISO8211 file with no real content
          // Correct this.  We should break the walk, and notify the user  See
          // FS#1406

          //                             if( !chain_broken_mssage_shown ){
          //                                 OCPNMessageBox(NULL,
          //                                                _("S57 Cell Update
          //                                                chain
          //                                                incomplete.\nENC
          //                                                features may be
          //                                                incomplete or
          //                                                inaccurate.\nCheck
          //                                                the logfile for
          //                                                details."),
          //                                                _("OpenCPN Create
          //                                                SENC Warning"), wxOK
          //                                                |
          //                                                wxICON_EXCLAMATION,
          //                                                30 );
          //                                                chain_broken_mssage_shown
          //                                                = true;
          //                             }

          wxString msg(
              _T("WARNING---ENC Update chain incomplete. Substituting NULL ")
              _T("update file: "));
          msg += ufile.GetFullName();
          wxLogMessage(msg);
          wxLogMessage(_T("   Subsequent ENC updates may produce errors."));
          wxLogMessage(
              _T("   This ENC exchange set should be updated and SENCs ")
              _T("rebuilt."));

          bool bstat;
          DDFModule dupdate;
          dupdate.Initialize('3', 'L', 'E', '1', '0', "!!!", 3, 4, 4);
          bstat = !(dupdate.Create(cp_ufile.mb_str()) == 0);
          dupdate.Close();

          if (!bstat) {
            wxString msg(_T("   Error creating dummy update file: "));
            msg.Append(cp_ufile);
            wxLogMessage(msg);
          }
        }

        m_tmpup_array.Add(cp_ufile);
        last_up_added = cp_ufile;
      }
    }

    //      Extract the date field from the last of the update files
    //      which is by definition a valid, present update file....

    wxFileName lastfile(last_up_added);
    wxString last_sext;
    last_sext.Printf(_T("%03d"), upmax);
    lastfile.SetExt(last_sext);

    bool bSuccess;
    DDFModule oUpdateModule;

    bSuccess =
        !(oUpdateModule.Open(lastfile.GetFullPath().mb_str(), TRUE) == 0);

    if (bSuccess) {
      //      Get publish/update date
      oUpdateModule.Rewind();
      DDFRecord *pr = oUpdateModule.ReadRecord();  // Record 0

      int nSuccess;
      char *u = NULL;

      if (pr)
        u = (char *)(pr->GetStringSubfield("DSID", 0, "ISDT", 0, &nSuccess));

      if (u) {
        if (strlen(u)) {
          LastUpdateDate = wxString(u, wxConvUTF8);
        }
      } else {
        wxDateTime now = wxDateTime::Now();
        LastUpdateDate = now.Format(_T("%Y%m%d"));
      }
    }
  }

  return retval;
}

bool Osenc::GetBaseFileAttr(const wxString &FullPath000) {
  DDFModule oModule;
  if (!oModule.Open(FullPath000.mb_str())) {
    return false;
  }

  oModule.Rewind();

  //    Read and parse DDFRecord 0 to get some interesting data
  //    n.b. assumes that the required fields will be in Record 0....  Is this
  //    always true?

  DDFRecord *pr = oModule.ReadRecord();  // Record 0
  //    pr->Dump(stdout);

  //    Fetch the Geo Feature Count, or something like it....
  m_nGeoRecords = pr->GetIntSubfield("DSSI", 0, "NOGR", 0);
  if (!m_nGeoRecords) {
    errorMessage =
        _T("GetBaseFileAttr:  DDFRecord 0 does not contain DSSI:NOGR ");

    m_nGeoRecords = 1;  // backstop
  }

  //  Use ISDT(Issue Date) here, which is the same as UADT(Updates Applied) for
  //  .000 files
  wxString date000;
  char *u = (char *)(pr->GetStringSubfield("DSID", 0, "ISDT", 0));
  if (u)
    date000 = wxString(u, wxConvUTF8);
  else {
    errorMessage =
        _T("GetBaseFileAttr:  DDFRecord 0 does not contain DSID:ISDT ");

    date000 =
        _T("20000101");  // backstop, very early, so any new files will update?
  }
  m_date000.ParseFormat(date000, _T("%Y%m%d"));
  if (!m_date000.IsValid()) m_date000.ParseFormat(_T("20000101"), _T("%Y%m%d"));

  m_date000.ResetTime();

  //    Fetch the EDTN(Edition) field
  u = (char *)(pr->GetStringSubfield("DSID", 0, "EDTN", 0));
  if (u)
    m_edtn000 = wxString(u, wxConvUTF8);
  else {
    errorMessage =
        _T("GetBaseFileAttr:  DDFRecord 0 does not contain DSID:EDTN ");

    m_edtn000 = _T("1");  // backstop
  }

  // m_SE = m_edtn000;

  //    Fetch the UPDN(Updates Applied) field
  u = (char *)(pr->GetStringSubfield("DSID", 0, "UPDN", 0));
  if (u) {
    long updn = 0;
    wxString tmp_updn = wxString(u, wxConvUTF8);
    if (tmp_updn.ToLong(&updn)) m_UPDN = updn;

  } else {
    errorMessage =
        _T("GetBaseFileAttr:  DDFRecord 0 does not contain DSID:UPDN ");

    m_UPDN = 0;  // backstop
  }

  //      Fetch the Native Scale by reading more records until DSPM is found
  m_native_scale = 0;
  for (; pr != NULL; pr = oModule.ReadRecord()) {
    if (pr->FindField("DSPM") != NULL) {
      m_native_scale = pr->GetIntSubfield("DSPM", 0, "CSCL", 0);
      break;
    }
  }
  if (!m_native_scale) {
    errorMessage = _T("GetBaseFileAttr:  ENC not contain DSPM:CSCL ");

    m_native_scale = 1000;  // backstop
  }

  return true;
}

//---------------------------------------------------------------------------------------------------
/*
 *      OpenCPN OSENC Version 2 Implementation
 */
//---------------------------------------------------------------------------------------------------

int Osenc::createSenc200(const wxString &FullPath000,
                         const wxString &SENCFileName, bool b_showProg) {
  lockCR.lock();

  m_FullPath000 = FullPath000;

  m_senc_file_create_version = 201;

  if (!m_poRegistrar) {
    m_poRegistrar = new S57ClassRegistrar();
    m_poRegistrar->LoadInfo(g_csv_locn.mb_str(), FALSE);
    m_bPrivateRegistrar = true;
    // errorMessage = _T("S57 Registrar not set.");
    // return ERROR_REGISTRAR_NOT_SET;
  }

  wxFileName SENCfile = wxFileName(SENCFileName);
  wxFileName file000 = wxFileName(FullPath000);

  //      Make the target directory if needed
  if (true != SENCfile.DirExists(SENCfile.GetPath())) {
    if (!SENCfile.Mkdir(SENCfile.GetPath())) {
      errorMessage =
          _T("Cannot create SENC file directory for ") + SENCfile.GetFullPath();
      lockCR.unlock();
      return ERROR_CANNOT_CREATE_SENC_DIR;
    }
  }

  //          Make a temp file to create the SENC in
  wxFileName tfn;
  wxString tmp_file = tfn.CreateTempFileName(_T(""));

  //     FILE *fps57;
  //     const char *pp = "wb";
  //     fps57 = fopen( tmp_file.mb_str(), pp );
  //
  //     if( fps57 == NULL ) {
  //         errorMessage = _T("Unable to create temp SENC file: ");
  //         errorMessage.Append( tfn.GetFullPath() );
  //         return ERROR_CANNOT_CREATE_TEMP_SENC_FILE;
  //     }

  if (m_pauxOutstream) {
    m_pOutstream = m_pauxOutstream;
  } else {
    m_pOutstream = new Osenc_outstreamFile();
  }

  Osenc_outstream *stream = m_pOutstream;

  if (!stream->Open(tmp_file)) {
    errorMessage = _T("Unable to create temp SENC file: ");
    errorMessage += tmp_file;
    delete m_pOutstream;
    lockCR.unlock();
    return ERROR_CANNOT_CREATE_TEMP_SENC_FILE;
  }

  //  Take a quick scan of the 000 file to get some basic attributes of the
  //  exchange set.
  if (!GetBaseFileAttr(FullPath000)) {
    delete m_pOutstream;
    lockCR.unlock();
    return ERROR_BASEFILE_ATTRIBUTES;
  }

  OGRS57DataSource S57DS;
  OGRS57DataSource *poS57DS = &S57DS;
  poS57DS->SetS57Registrar(m_poRegistrar);

  //  Ingest the .000 cell, with updates applied

  if (ingestCell(poS57DS, FullPath000, SENCfile.GetPath())) {
    errorMessage = _T("Error ingesting: ") + FullPath000;
    delete m_pOutstream;
    lockCR.unlock();
    return ERROR_INGESTING000;
  }

  S57Reader *poReader = poS57DS->GetModule(0);

  //  Create the Coverage table Records, which also calculates the chart extents
  if (!CreateCOVRTables(poReader, m_poRegistrar)) {
    delete m_pOutstream;
    lockCR.unlock();
    return ERROR_SENCFILE_ABORT;
  }

  //  Establish a common reference point for the chart, from the extent
  m_ref_lat = (m_extent.NLAT + m_extent.SLAT) / 2.;
  m_ref_lon = (m_extent.WLON + m_extent.ELON) / 2.;

  bool bcont = true;

  //          Write the Header information

  // char temp[201];

  // fprintf( fps57, "SENC Version= %d\n", 200 );

  // The chart cell "nice name"
  wxString nice_name;
  s57chart::GetChartNameFromTXT(FullPath000, nice_name);

  string sname = "UTF8Error";
  wxCharBuffer buffer = nice_name.ToUTF8();
  if (buffer.data()) sname = buffer.data();

  if (!WriteHeaderRecord200(stream, HEADER_SENC_VERSION,
                            (uint16_t)m_senc_file_create_version)) {
    stream->Close();
    delete m_pOutstream;
    lockCR.unlock();
    return ERROR_SENCFILE_ABORT;
  }

  if (!WriteHeaderRecord200(stream, HEADER_CELL_NAME, sname)) {
    stream->Close();
    delete m_pOutstream;
    lockCR.unlock();
    return ERROR_SENCFILE_ABORT;
  }

  wxString date000 = m_date000.Format(_T("%Y%m%d"));
  string sdata = date000.ToStdString();
  if (!WriteHeaderRecord200(stream, HEADER_CELL_PUBLISHDATE, sdata)) {
    stream->Close();
    delete m_pOutstream;
    lockCR.unlock();
    return ERROR_SENCFILE_ABORT;
  }

  long n000 = 0;
  m_edtn000.ToLong(&n000);
  if (!WriteHeaderRecord200(stream, HEADER_CELL_EDITION, (uint16_t)n000)) {
    stream->Close();
    delete m_pOutstream;
    lockCR.unlock();
    return ERROR_SENCFILE_ABORT;
  }

  sdata = m_LastUpdateDate.ToStdString();
  if (!WriteHeaderRecord200(stream, HEADER_CELL_UPDATEDATE, sdata)) {
    stream->Close();
    delete m_pOutstream;
    lockCR.unlock();
    return ERROR_SENCFILE_ABORT;
  }

  if (!WriteHeaderRecord200(stream, HEADER_CELL_UPDATE,
                            (uint16_t)m_last_applied_update)) {
    stream->Close();
    delete m_pOutstream;
    lockCR.unlock();
    return ERROR_SENCFILE_ABORT;
  }

  if (!WriteHeaderRecord200(stream, HEADER_CELL_NATIVESCALE,
                            (uint32_t)m_native_scale)) {
    stream->Close();
    delete m_pOutstream;
    lockCR.unlock();
    return ERROR_SENCFILE_ABORT;
  }

  wxDateTime now = wxDateTime::Now();
  wxString dateNow = now.Format(_T("%Y%m%d"));
  sdata = dateNow.ToStdString();
  if (!WriteHeaderRecord200(stream, HEADER_CELL_SENCCREATEDATE, sdata)) {
    stream->Close();
    delete m_pOutstream;
    lockCR.unlock();
    return ERROR_SENCFILE_ABORT;
  }

  //  Write the Coverage table Records
  if (!CreateCovrRecords(stream)) {
    stream->Close();
    delete m_pOutstream;
    lockCR.unlock();
    return ERROR_SENCFILE_ABORT;
  }

  poReader->Rewind();

  //        Prepare Vector Edge Helper table
  //        And fill in the table
  int feid = 0;
  OGRFeature *pEdgeVectorRecordFeature = poReader->ReadVector(feid, RCNM_VE);
  while (NULL != pEdgeVectorRecordFeature) {
    int record_id = pEdgeVectorRecordFeature->GetFieldAsInteger("RCID");

    m_vector_helper_hash[record_id] = feid;

    feid++;
    delete pEdgeVectorRecordFeature;
    pEdgeVectorRecordFeature = poReader->ReadVector(feid, RCNM_VE);
  }

  wxString Message = SENCfile.GetFullPath();
  Message.Append(_T("...Ingesting"));

  wxString Title(_("OpenCPN S57 SENC File Create..."));
  Title.append(SENCfile.GetFullPath());

#if wxUSE_PROGRESSDLG

  wxStopWatch progsw;
  int nProg = poReader->GetFeatureCount();

  if (wxThread::IsMain() && b_showProg) {
    m_ProgDialog = new wxGenericProgressDialog();

    wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
    m_ProgDialog->SetFont(*qFont);

    m_ProgDialog->Create(Title, Message, nProg, NULL,
                         wxPD_AUTO_HIDE | wxPD_SMOOTH);
  }
#endif

  //  Loop in the S57 reader, extracting Features one-by-one
  OGRFeature *objectDef;

  int iObj = 0;

  while (bcont) {
    objectDef = poReader->ReadNextFeature();

    if (objectDef != NULL) {
      iObj++;

#if wxUSE_PROGRESSDLG

      //  Update the progress dialog
      // We update only every 200 milliseconds to improve performance as
      // updating the dialog is very expensive...
      // WXGTK is measurably slower even with 100ms here
      if (m_ProgDialog && progsw.Time() > 200) {
        progsw.Start();

        wxString sobj =
            wxString(objectDef->GetDefnRef()->GetName(), wxConvUTF8);
        sobj.Append(wxString::Format(_T("  %d/%d       "), iObj, nProg));

        bcont = m_ProgDialog->Update(iObj, sobj);
#if defined(__WXMSW__) || defined(__WXOSX__)
        wxSafeYield();
#endif
      }
#endif

      OGRwkbGeometryType geoType = wkbUnknown;
      //      This test should not be necessary for real (i.e not C_AGGR)
      //      features However... some update files contain errors, and have
      //      deleted some geometry without deleting the corresponding
      //      feature(s). So, GeometryType becomes Unknown. e.g. US5MD11M.017 In
      //      this case, all we can do is skip the feature....sigh.

      if (objectDef->GetGeometryRef() != NULL)
        geoType = objectDef->GetGeometryRef()->getGeometryType();

      //      n.b  This next line causes skip of C_AGGR features w/o geometry
      if (geoType != wkbUnknown) {  // Write only if has wkbGeometry
        CreateSENCRecord200(objectDef, stream, 1, poReader);
      }

      delete objectDef;

    } else
      break;
  }

  if (bcont) {
    //      Create and write the Vector Edge Table
    CreateSENCVectorEdgeTableRecord200(stream, poReader);

    //      Create and write the Connected NodeTable
    CreateSENCVectorConnectedTableRecord200(stream, poReader);
  }

  //          All done, so clean up
  stream->Close();
  delete m_pOutstream;

  //  Delete any temporary (working) real and dummy update files,
  //  as well as .000 file created by ValidateAndCountUpdates()
  for (unsigned int iff = 0; iff < m_tmpup_array.GetCount(); iff++)
    remove(m_tmpup_array[iff].mb_str());

  int ret_code = 0;

  if (!bcont)  // aborted
  {
    wxRemoveFile(tmp_file);  // kill the temp file
    ret_code = ERROR_SENCFILE_ABORT;
  }

  if (bcont) {
    bool cpok = wxRenameFile(tmp_file, SENCfile.GetFullPath());
    if (!cpok) {
      errorMessage = _T("   Cannot rename temporary SENC file ");
      errorMessage.Append(tmp_file);
      errorMessage.Append(_T(" to "));
      errorMessage.Append(SENCfile.GetFullPath());
      ret_code = ERROR_SENCFILE_ABORT;
    } else
      ret_code = SENC_NO_ERROR;
  }

#if wxUSE_PROGRESSDLG
  delete m_ProgDialog;
#endif

  lockCR.unlock();

  return ret_code;
}

bool Osenc::CreateCovrRecords(Osenc_outstream *stream) {
  // First, create the Extent record
  _OSENC_EXTENT_Record record;
  record.record_type = CELL_EXTENT_RECORD;
  record.record_length = sizeof(_OSENC_EXTENT_Record);
  record.extent_sw_lat = m_extent.SLAT;
  record.extent_sw_lon = m_extent.WLON;
  record.extent_nw_lat = m_extent.NLAT;
  record.extent_nw_lon = m_extent.WLON;
  record.extent_ne_lat = m_extent.NLAT;
  record.extent_ne_lon = m_extent.ELON;
  record.extent_se_lat = m_extent.SLAT;
  record.extent_se_lon = m_extent.ELON;

  size_t targetCount = sizeof(record);
  if (!stream->Write(&record, targetCount).IsOk()) return false;

  for (int i = 0; i < m_nCOVREntries; i++) {
    int nPoints = m_pCOVRTablePoints[i];

    float *fpbuf = m_pCOVRTable[i];

    //  Ready to write the record
    _OSENC_COVR_Record_Base record;
    record.record_type = CELL_COVR_RECORD;
    record.record_length = sizeof(_OSENC_COVR_Record_Base) + sizeof(uint32_t) +
                           (nPoints * 2 * sizeof(float));

    //  Write the base record
    size_t targetCount = sizeof(record);
    if (!stream->Write(&record, targetCount).IsOk()) return false;

    // Write the point count
    targetCount = sizeof(uint32_t);
    if (!stream->Write(&nPoints, targetCount).IsOk()) return false;

    //  Write the point array
    targetCount = nPoints * 2 * sizeof(float);
    if (!stream->Write(fpbuf, targetCount).IsOk()) return false;
  }

  for (int i = 0; i < m_nNoCOVREntries; i++) {
    int nPoints = m_pNoCOVRTablePoints[i];

    float *fpbuf = m_pNoCOVRTable[i];

    //  Ready to write the record
    _OSENC_NOCOVR_Record_Base record;
    record.record_type = CELL_NOCOVR_RECORD;
    record.record_length = sizeof(_OSENC_NOCOVR_Record_Base) +
                           sizeof(uint32_t) + (nPoints * 2 * sizeof(float));

    //  Write the base record
    size_t targetCount = sizeof(record);
    if (!stream->Write(&record, targetCount).IsOk()) return false;

    // Write the point count
    targetCount = sizeof(uint32_t);
    if (!stream->Write(&nPoints, targetCount).IsOk()) return false;

    //  Write the point array
    targetCount = nPoints * 2 * sizeof(float);
    if (!stream->Write(fpbuf, targetCount).IsOk()) return false;
  }

  return true;
}

bool Osenc::WriteHeaderRecord200(Osenc_outstream *stream, int recordType,
                                 std::string payload) {
  int payloadLength = payload.length() + 1;
  int recordLength = payloadLength + sizeof(OSENC_Record_Base);

  //  Get a reference to the class persistent buffer
  unsigned char *pBuffer = getBuffer(recordLength);

  OSENC_Record *pRecord = (OSENC_Record *)pBuffer;
  memset(pRecord, 0, recordLength);
  pRecord->record_type = recordType;
  pRecord->record_length = recordLength;
  memcpy(&pRecord->payload, payload.c_str(), payloadLength);

  size_t targetCount = recordLength;
  if (!stream->Write(pBuffer, targetCount).IsOk())
    return false;
  else
    return true;
}

bool Osenc::WriteHeaderRecord200(Osenc_outstream *stream, int recordType,
                                 uint16_t val) {
  int payloadLength = sizeof(uint16_t);
  int recordLength = payloadLength + sizeof(OSENC_Record_Base);

  //  Get a reference to the class persistent buffer
  unsigned char *pBuffer = getBuffer(recordLength);

  OSENC_Record *pRecord = (OSENC_Record *)pBuffer;
  memset(pRecord, 0, recordLength);
  pRecord->record_type = recordType;
  pRecord->record_length = recordLength;
  memcpy(&pRecord->payload, &val, payloadLength);

  size_t targetCount = recordLength;
  if (!stream->Write(pBuffer, targetCount).IsOk())
    return false;
  else
    return true;
}

bool Osenc::WriteHeaderRecord200(Osenc_outstream *stream, int recordType,
                                 uint32_t val) {
  int payloadLength = sizeof(uint32_t);
  int recordLength = payloadLength + sizeof(OSENC_Record_Base);

  //  Get a reference to the class persistent buffer
  unsigned char *pBuffer = getBuffer(recordLength);

  OSENC_Record *pRecord = (OSENC_Record *)pBuffer;
  memset(pRecord, 0, recordLength);
  pRecord->record_type = recordType;
  pRecord->record_length = recordLength;
  memcpy(&pRecord->payload, &val, payloadLength);

  size_t targetCount = recordLength;
  if (!stream->Write(pBuffer, targetCount).IsOk())
    return false;
  else
    return true;
}

bool Osenc::WriteFIDRecord200(Osenc_outstream *stream, int nOBJL, int featureID,
                              int prim) {
  OSENC_Feature_Identification_Record_Base record;
  memset(&record, 0, sizeof(record));

  record.record_type = FEATURE_ID_RECORD;
  record.record_length = sizeof(record);

  record.feature_ID = featureID;
  record.feature_type_code = nOBJL;
  record.feature_primitive = prim;

  size_t targetCount = sizeof(record);
  if (!stream->Write(&record, targetCount).IsOk())
    return false;
  else
    return true;
}

bool Osenc::CreateMultiPointFeatureGeometryRecord200(OGRFeature *pFeature,
                                                     Osenc_outstream *stream) {
  OGRGeometry *pGeo = pFeature->GetGeometryRef();

  int wkb_len = pGeo->WkbSize();
  unsigned char *pwkb_buffer = (unsigned char *)malloc(wkb_len);

  //  Get the GDAL data representation
  pGeo->exportToWkb(wkbNDR, pwkb_buffer);

  //  Capture a buffer of the raw geometry

  unsigned char *ps = pwkb_buffer;
  ps += 5;
  int nPoints = *((int *)ps);  // point count

  int sb_len = (nPoints * 3 * sizeof(float));  // points as floats

  unsigned char *psb_buffer = (unsigned char *)malloc(sb_len);
  unsigned char *pd = psb_buffer;

  ps = pwkb_buffer;
  ps += 9;  // skip byte order, type, and count

  float *pdf = (float *)pd;

  //  Set absurd bbox starting limits
  float lonmax = -1000;
  float lonmin = 1000;
  float latmax = -1000;
  float latmin = 1000;

  for (int ip = 0; ip < nPoints; ip++) {
    // Workaround a bug?? in OGRGeometryCollection
    // While exporting point geometries serially, OGRPoint->exportToWkb assumes
    // that if Z is identically 0, then the point must be a 2D point only. So,
    // the collection Wkb is corrupted with some 3D, and some 2D points.
    // Workaround:  Get reference to the points serially, and explicitly read
    // X,Y,Z Ignore the previously read Wkb buffer

    OGRGeometryCollection *temp_geometry_collection =
        (OGRGeometryCollection *)pGeo;
    OGRGeometry *temp_geometry = temp_geometry_collection->getGeometryRef(ip);
    OGRPoint *pt_geom = (OGRPoint *)temp_geometry;

    double lon = pt_geom->getX();
    double lat = pt_geom->getY();
    double depth = pt_geom->getZ();

    //  Calculate SM from chart common reference point
    double easting, northing;
    toSM(lat, lon, m_ref_lat, m_ref_lon, &easting, &northing);

#ifdef __ARM_ARCH
    float __attribute__((aligned(16))) east = easting;
    float __attribute__((aligned(16))) north = northing;
    float __attribute__((aligned(16))) deep = depth;
    unsigned char *puceast = (unsigned char *)&east;
    unsigned char *pucnorth = (unsigned char *)&north;
    unsigned char *pucdeep = (unsigned char *)&deep;

    memcpy(pdf++, puceast, sizeof(float));
    memcpy(pdf++, pucnorth, sizeof(float));
    memcpy(pdf++, pucdeep, sizeof(float));

#else
    *pdf++ = easting;
    *pdf++ = northing;
    *pdf++ = (float)depth;
#endif

    //  Keep a running calculation of min/max
    lonmax = fmax(lon, lonmax);
    lonmin = fmin(lon, lonmin);
    latmax = fmax(lat, latmax);
    latmin = fmin(lat, latmin);
  }

  //  Ready to write the record
  OSENC_MultipointGeometry_Record_Base record;
  record.record_type = FEATURE_GEOMETRY_RECORD_MULTIPOINT;
  record.record_length = sizeof(OSENC_MultipointGeometry_Record_Base) +
                         (nPoints * 3 * sizeof(float));
  record.extent_e_lon = lonmax;
  record.extent_w_lon = lonmin;
  record.extent_n_lat = latmax;
  record.extent_s_lat = latmin;
  record.point_count = nPoints;

  //  Write the base record
  size_t targetCount = sizeof(record);
  if (!stream->Write(&record, targetCount).IsOk()) goto failure;
  //  Write the 3D point array
  targetCount = nPoints * 3 * sizeof(float);
  if (!stream->Write(psb_buffer, targetCount).IsOk()) goto failure;

  //  Free the buffers
  free(psb_buffer);
  free(pwkb_buffer);
  return true;
failure:
  //  Free the buffers
  free(psb_buffer);
  free(pwkb_buffer);
  return false;
}

bool Osenc::CreateLineFeatureGeometryRecord200(S57Reader *poReader,
                                               OGRFeature *pFeature,
                                               Osenc_outstream *stream) {
  OGRGeometry *pGeo = pFeature->GetGeometryRef();

  int wkb_len = pGeo->WkbSize();
  unsigned char *pwkb_buffer = (unsigned char *)malloc(wkb_len);

  //  Get the GDAL data representation
  pGeo->exportToWkb(wkbNDR, pwkb_buffer);

  //  Capture a buffer of the raw geometry

  int sb_len =
      ((wkb_len - 9) / 2) + 9 + 16;  // data will be 4 byte float, not double

  unsigned char *psb_buffer = (unsigned char *)malloc(sb_len);
  unsigned char *pd = psb_buffer;
  unsigned char *ps = pwkb_buffer;

  memcpy(pd, ps, 9);  // byte order, type, and count

  int ip = *((int *)(ps + 5));  // point count

  pd += 9;
  ps += 9;
  double *psd = (double *)ps;
  float *pdf = (float *)pd;

  //  Set absurd bbox starting limits
  float lonmax = -1000;
  float lonmin = 1000;
  float latmax = -1000;
  float latmin = 1000;

  for (int i = 0; i < ip; i++) {  // convert doubles to floats
                                  // computing bbox as we go
    float lon, lat;
    double easting, northing;
#ifdef __ARM_ARCH
    double __attribute__((aligned(16))) east_d, north_d;
    unsigned char *pucd = (unsigned char *)psd;

    memcpy(&east_d, pucd, sizeof(double));
    psd += 1;
    pucd += sizeof(double);
    memcpy(&north_d, pucd, sizeof(double));
    psd += 1;
    lon = east_d;
    lat = north_d;

    //  Calculate SM from chart common reference point
    toSM(lat, lon, m_ref_lat, m_ref_lon, &easting, &northing);
    unsigned char *puceasting = (unsigned char *)&easting;
    unsigned char *pucnorthing = (unsigned char *)&northing;

    memcpy(pdf++, puceasting, sizeof(float));
    memcpy(pdf++, pucnorthing, sizeof(float));

#else
    lon = (float)*psd++;
    lat = (float)*psd++;

    //  Calculate SM from chart common reference point
    toSM(lat, lon, m_ref_lat, m_ref_lon, &easting, &northing);

    *pdf++ = easting;
    *pdf++ = northing;
#endif

    lonmax = fmax(lon, lonmax);
    lonmin = fmin(lon, lonmin);
    latmax = fmax(lat, latmax);
    latmin = fmin(lat, latmin);
  }

  int nEdgeVectorRecords = 0;
  unsigned char *pvec_buffer =
      getObjectVectorIndexTable(poReader, pFeature, nEdgeVectorRecords);

#if 0

    //    Capture the Vector Table geometry indices into a memory buffer
    int *pNAME_RCID;
    int *pORNT;
    int nEdgeVectorRecords;
    OGRFeature *pEdgeVectorRecordFeature;

    pNAME_RCID = (int *) pFeature->GetFieldAsIntegerList( "NAME_RCID", &nEdgeVectorRecords );
    pORNT = (int *) pFeature->GetFieldAsIntegerList( "ORNT", NULL );

//fprintf( fpOut, "LSINDEXLIST %d\n", nEdgeVectorRecords );
//                    fwrite(pNAME_RCID, 1, nEdgeVectorRecords * sizeof(int), fpOut);



    unsigned char *pvec_buffer = (unsigned char *)malloc(nEdgeVectorRecords * 3 * sizeof(int));
    unsigned char *pvRun = pvec_buffer;

    //  Set up the options, adding RETURN_PRIMITIVES
    char ** papszReaderOptions = NULL;
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_UPDATES, "ON" );
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_LINKAGES,"ON" );
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES,"ON" );
    poReader->SetOptions( papszReaderOptions );

//    Capture the beginning and end point connected nodes for each edge vector record
    for( int i = 0; i < nEdgeVectorRecords; i++ ) {

        int *pI = (int *)pvRun;

        int edge_rcid = pNAME_RCID[i];

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
            if( !poReader->FetchPoint( RCNM_VC, start_rcid, NULL, NULL, NULL, NULL ) )
                start_rcid = -1;
            if( !poReader->FetchPoint( RCNM_VC, end_rcid, NULL, NULL, NULL, NULL ) )
                end_rcid = -2;

            OGRLineString *poLS = (OGRLineString *)pEdgeVectorRecordFeature->GetGeometryRef();

            int edge_ornt = 1;

             if( edge_ornt == 1 ){                                    // forward
                *pI++ = start_rcid;
                *pI++ = edge_rcid;
                *pI++ = end_rcid;
            } else {                                                  // reverse
                *pI++ = end_rcid;
                *pI++ = edge_rcid;
                *pI++ = start_rcid;
            }

            delete pEdgeVectorRecordFeature;
        } else {
            start_rcid = -1;                                    // error indication
            end_rcid = -2;

            *pI++ = start_rcid;
            *pI++ = edge_rcid;
            *pI++ = end_rcid;
        }

        pvRun += 3 * sizeof(int);
    }


        //  Reset the options
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES,"OFF" );
    poReader->SetOptions( papszReaderOptions );
    CSLDestroy( papszReaderOptions );
#endif

  //  Ready to write the record
  OSENC_LineGeometry_Record_Base record;
  record.record_type = FEATURE_GEOMETRY_RECORD_LINE;
  record.record_length = sizeof(OSENC_LineGeometry_Record_Base) +
                         (nEdgeVectorRecords * 3 * sizeof(int));
  record.extent_e_lon = lonmax;
  record.extent_w_lon = lonmin;
  record.extent_n_lat = latmax;
  record.extent_s_lat = latmin;
  record.edgeVector_count = nEdgeVectorRecords;

  //  Write the base record
  size_t targetCount = sizeof(record);
  if (!stream->Write(&record, targetCount).IsOk()) return false;

  //  Write the table index array
  targetCount = nEdgeVectorRecords * 3 * sizeof(int);
  if (!stream->Write(pvec_buffer, targetCount).IsOk()) return false;

  //  Free the buffers
  free(pvec_buffer);
  free(psb_buffer);
  free(pwkb_buffer);

  return true;
}

bool Osenc::CreateAreaFeatureGeometryRecord200(S57Reader *poReader,
                                               OGRFeature *pFeature,
                                               Osenc_outstream *stream) {
  int error_code;

  PolyTessGeo *ppg = NULL;

  OGRGeometry *pGeo = pFeature->GetGeometryRef();
  OGRPolygon *poly = (OGRPolygon *)(pGeo);

  if (!poly->getExteriorRing()) return false;

  lockCR.unlock();
  ppg = new PolyTessGeo(poly, true, m_ref_lat, m_ref_lon, m_LOD_meters);
  lockCR.lock();

  error_code = ppg->ErrorCode;

  if (error_code) {
    wxLogMessage(
        _T("   Warning: S57 SENC Geometry Error %d, Some Features ignored."),
        ppg->ErrorCode);
    delete ppg;

    return false;
  }

  //  Ready to create the record

  //  The base record, with writing deferred until length is known
  OSENC_AreaGeometry_Record_Base baseRecord;
  memset(&baseRecord, 0, sizeof(baseRecord));

  baseRecord.record_type = FEATURE_GEOMETRY_RECORD_AREA;

  //  Length calculation is deferred...

  baseRecord.extent_s_lat = ppg->Get_ymin();
  baseRecord.extent_n_lat = ppg->Get_ymax();
  baseRecord.extent_e_lon = ppg->Get_xmax();
  baseRecord.extent_w_lon = ppg->Get_xmin();

  baseRecord.contour_count = ppg->GetnContours();

  //  Create the array of contour point counts

  int contourPointCountArraySize = ppg->GetnContours() * sizeof(uint32_t);
  uint32_t *contourPointCountArray =
      (uint32_t *)malloc(contourPointCountArraySize);

  uint32_t *pr = contourPointCountArray;

  for (int i = 0; i < ppg->GetnContours(); i++) {
    *pr++ = ppg->Get_PolyTriGroup_head()->pn_vertex[i];
  }

  //  All that is left is the tesselation result Triangle lists...
  //  This could be a large array, and we don't want to use a buffer.
  //  Rather, we want to write directly to the output file.

  //  So, we
  //  a  walk the TriPrim chain once to get it's length,
  //  b update the total record length,
  //  c write everything before the TriPrim chain,
  //  d and then directly write the TriPrim chain.

  //  Walk the TriPrim chain
  int geoLength = 0;

  TriPrim *pTP = ppg->Get_PolyTriGroup_head()
                     ->tri_prim_head;  // head of linked list of TriPrims

  int n_TriPrims = 0;
  while (pTP) {
    geoLength += sizeof(uint8_t) + sizeof(uint32_t);  // type, nvert
    geoLength += pTP->nVert * 2 * sizeof(float);      // vertices
    geoLength += 4 * sizeof(double);                  // Primitive bounding box
    pTP = pTP->p_next;

    n_TriPrims++;
  }

  baseRecord.triprim_count = n_TriPrims;  //  Set the number of TriPrims

  int nEdgeVectorRecords = 0;
  unsigned char *pvec_buffer =
      getObjectVectorIndexTable(poReader, pFeature, nEdgeVectorRecords);

#if 0
     //  Create the Vector Edge Index table into a memory buffer
     //  This buffer will follow the triangle buffer in the output stream
    int *pNAME_RCID;
    int *pORNT;
    int nEdgeVectorRecords;
    OGRFeature *pEdgeVectorRecordFeature;

    pNAME_RCID = (int *) pFeature->GetFieldAsIntegerList( "NAME_RCID", &nEdgeVectorRecords );
    pORNT = (int *) pFeature->GetFieldAsIntegerList( "ORNT", NULL );

    baseRecord.edgeVector_count = nEdgeVectorRecords;

    unsigned char *pvec_buffer = (unsigned char *)malloc(nEdgeVectorRecords * 3 * sizeof(int));
    unsigned char *pvRun = pvec_buffer;

    //  Set up the options, adding RETURN_PRIMITIVES
    char ** papszReaderOptions = NULL;
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_UPDATES, "ON" );
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_LINKAGES,"ON" );
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES,"ON" );
    poReader->SetOptions( papszReaderOptions );

    //    Capture the beginning and end point connected nodes for each edge vector record
    for( int i = 0; i < nEdgeVectorRecords; i++ ) {

        int *pI = (int *)pvRun;

        int start_rcid, end_rcid;
        int target_record_feid = m_vector_helper_hash[pNAME_RCID[i]];
        pEdgeVectorRecordFeature = poReader->ReadVector( target_record_feid, RCNM_VE );

        int edge_rcid = pNAME_RCID[i];

        if( NULL != pEdgeVectorRecordFeature ) {
            start_rcid = pEdgeVectorRecordFeature->GetFieldAsInteger( "NAME_RCID_0" );
            end_rcid = pEdgeVectorRecordFeature->GetFieldAsInteger( "NAME_RCID_1" );
            //    Make sure the start and end points exist....
            //    Note this poReader method was converted to Public access to
            //     facilitate this test.  There might be another clean way....
            //    Problem first found on Holand ENC 1R5YM009.000
            if( !poReader->FetchPoint( RCNM_VC, start_rcid, NULL, NULL, NULL, NULL ) )
                start_rcid = -1;
            if( !poReader->FetchPoint( RCNM_VC, end_rcid, NULL, NULL, NULL, NULL ) )
                end_rcid = -2;

            int edge_ornt = 1;
            //  Allocate some storage for converted points

            if( edge_ornt == 1 ){                                    // forward
                *pI++ = start_rcid;
                *pI++ = edge_rcid;
                *pI++ = end_rcid;
            } else {                                                  // reverse
                *pI++ = end_rcid;
                *pI++ = edge_rcid;
                *pI++ = start_rcid;
            }

            delete pEdgeVectorRecordFeature;
        } else {
            start_rcid = -1;                                    // error indication
            end_rcid = -2;

            *pI++ = start_rcid;
            *pI++ = edge_rcid;
            *pI++ = end_rcid;
        }

        pvRun += 3 * sizeof(int);
    }


    //  Reset the options
    papszReaderOptions = CSLSetNameValue( papszReaderOptions, S57O_RETURN_PRIMITIVES,"OFF" );
    poReader->SetOptions( papszReaderOptions );
    CSLDestroy( papszReaderOptions );

#endif

  baseRecord.edgeVector_count = nEdgeVectorRecords;

  //  Calculate the total record length
  int recordLength = sizeof(OSENC_AreaGeometry_Record_Base);
  recordLength += contourPointCountArraySize;
  recordLength += geoLength;
  recordLength += nEdgeVectorRecords * 3 * sizeof(int);
  baseRecord.record_length = recordLength;

  //  Write the base record
  size_t targetCount = sizeof(baseRecord);
  if (!stream->Write(&baseRecord, targetCount).IsOk()) return false;

  //  Write the contour point count array
  targetCount = contourPointCountArraySize;
  if (!stream->Write(contourPointCountArray, targetCount).IsOk()) return false;

  //  Walk and transcribe the TriPrim chain
  pTP = ppg->Get_PolyTriGroup_head()
            ->tri_prim_head;  // head of linked list of TriPrims
  while (pTP) {
    if (!stream->Write(&pTP->type, sizeof(uint8_t)).IsOk()) return false;
    if (!stream->Write(&pTP->nVert, sizeof(uint32_t)).IsOk()) return false;

    //         fwrite (&pTP->minxt , sizeof(double), 1, fpOut);
    //         fwrite (&pTP->maxxt , sizeof(double), 1, fpOut);
    //         fwrite (&pTP->minyt , sizeof(double), 1, fpOut);
    //         fwrite (&pTP->maxyt , sizeof(double), 1, fpOut);

    double minlat, minlon, maxlat, maxlon;
    minlat = pTP->tri_box.GetMinLat();
    minlon = pTP->tri_box.GetMinLon();
    maxlat = pTP->tri_box.GetMaxLat();
    maxlon = pTP->tri_box.GetMaxLon();

    if (!stream->Write(&minlon, sizeof(double)).IsOk()) return false;
    if (!stream->Write(&maxlon, sizeof(double)).IsOk()) return false;
    if (!stream->Write(&minlat, sizeof(double)).IsOk()) return false;
    if (!stream->Write(&maxlat, sizeof(double)).IsOk()) return false;

    // Testing TODO
    //         float *pf = (float *)pTP->p_vertex;
    //         float a = *pf++;
    //         float b = *pf;

    if (!stream->Write(pTP->p_vertex, pTP->nVert * 2 * sizeof(float)).IsOk())
      return false;

    pTP = pTP->p_next;
  }

  //  Write the Edge Vector index table
  targetCount = nEdgeVectorRecords * 3 * sizeof(int);
  if (!stream->Write(pvec_buffer, targetCount).IsOk()) return false;

  delete ppg;
  free(contourPointCountArray);
  free(pvec_buffer);

  return true;
}

unsigned char *Osenc::getObjectVectorIndexTable(S57Reader *poReader,
                                                OGRFeature *pFeature,
                                                int &nEntries) {
  //  Create the Vector Edge Index table into a memory buffer
  //  This buffer will follow the triangle buffer in the output stream
  int *pNAME_RCID;
  int *pORNT;
  int nEdgeVectorRecords;
  OGRFeature *pEdgeVectorRecordFeature;

  pNAME_RCID =
      (int *)pFeature->GetFieldAsIntegerList("NAME_RCID", &nEdgeVectorRecords);
  pORNT = (int *)pFeature->GetFieldAsIntegerList("ORNT", NULL);

  nEntries = nEdgeVectorRecords;

  unsigned char *pvec_buffer =
      (unsigned char *)malloc(nEdgeVectorRecords * 3 * sizeof(int));
  unsigned char *pvRun = pvec_buffer;

  //  Set up the options, adding RETURN_PRIMITIVES
  char **papszReaderOptions = NULL;
  papszReaderOptions = CSLSetNameValue(papszReaderOptions, S57O_UPDATES, "ON");
  papszReaderOptions =
      CSLSetNameValue(papszReaderOptions, S57O_RETURN_LINKAGES, "ON");
  papszReaderOptions =
      CSLSetNameValue(papszReaderOptions, S57O_RETURN_PRIMITIVES, "ON");
  poReader->SetOptions(papszReaderOptions);

  //    Capture the beginning and end point connected nodes for each edge vector
  //    record
  for (int i = 0; i < nEdgeVectorRecords; i++) {
    int *pI = (int *)pvRun;

    int edge_rcid = pNAME_RCID[i];

    int start_rcid, end_rcid;
    int target_record_feid = m_vector_helper_hash[pNAME_RCID[i]];
    pEdgeVectorRecordFeature =
        poReader->ReadVector(target_record_feid, RCNM_VE);

    if (NULL != pEdgeVectorRecordFeature) {
      start_rcid = pEdgeVectorRecordFeature->GetFieldAsInteger("NAME_RCID_0");
      end_rcid = pEdgeVectorRecordFeature->GetFieldAsInteger("NAME_RCID_1");

      //    Make sure the start and end points exist....
      //    Note this poReader method was converted to Public access to
      //     facilitate this test.  There might be another clean way....
      //    Problem first found on Holand ENC 1R5YM009.000
      if (!poReader->FetchPoint(RCNM_VC, start_rcid, NULL, NULL, NULL, NULL))
        start_rcid = -1;
      if (!poReader->FetchPoint(RCNM_VC, end_rcid, NULL, NULL, NULL, NULL))
        end_rcid = -2;

      OGRLineString *poLS =
          (OGRLineString *)pEdgeVectorRecordFeature->GetGeometryRef();
      if (!poLS)
        edge_rcid = 0;
      else if (poLS->getNumPoints() < 1)
        edge_rcid = 0;

      int edge_ornt = pORNT[i];

      if (edge_ornt == 1) {  // forward
        *pI++ = start_rcid;
        *pI++ = edge_rcid;
        *pI++ = end_rcid;
      } else {  // reverse
        *pI++ = end_rcid;
        *pI++ = -edge_rcid;
        *pI++ = start_rcid;
      }

      delete pEdgeVectorRecordFeature;
    } else {
      start_rcid = -1;  // error indication
      end_rcid = -2;

      *pI++ = start_rcid;
      *pI++ = edge_rcid;
      *pI++ = end_rcid;
    }

    pvRun += 3 * sizeof(int);
  }

  //  Reset the options
  papszReaderOptions =
      CSLSetNameValue(papszReaderOptions, S57O_RETURN_PRIMITIVES, "OFF");
  poReader->SetOptions(papszReaderOptions);
  CSLDestroy(papszReaderOptions);

  return pvec_buffer;
}

void Osenc::CreateSENCVectorEdgeTableRecord200(Osenc_outstream *stream,
                                               S57Reader *poReader) {
  //  We create the payload first, so we can calculate the total record length
  uint8_t *pPayload = NULL;
  int payloadSize = 0;
  uint8_t *pRun = pPayload;

  //  Set up the S57Reader options, adding RETURN_PRIMITIVES
  char **papszReaderOptions = NULL;
  papszReaderOptions = CSLSetNameValue(papszReaderOptions, S57O_UPDATES, "ON");
  papszReaderOptions =
      CSLSetNameValue(papszReaderOptions, S57O_RETURN_LINKAGES, "ON");
  papszReaderOptions =
      CSLSetNameValue(papszReaderOptions, S57O_RETURN_PRIMITIVES, "ON");
  poReader->SetOptions(papszReaderOptions);

  int feid = 0;
  OGRLineString *pLS = NULL;
  OGRGeometry *pGeo;
  OGRFeature *pEdgeVectorRecordFeature = poReader->ReadVector(feid, RCNM_VE);

  int nFeatures = 0;

  //  Read all the EdgeVector Features
  while (NULL != pEdgeVectorRecordFeature) {
    //  Check for a zero point count.  Dunno why this should happen, other than
    //  bad ENC encoding

    int nPoints = 0;
    if (pEdgeVectorRecordFeature->GetGeometryRef() != NULL) {
      pGeo = pEdgeVectorRecordFeature->GetGeometryRef();
      if (pGeo->getGeometryType() == wkbLineString) {
        pLS = (OGRLineString *)pGeo;
        nPoints = pLS->getNumPoints();
      } else
        nPoints = 0;
    }

    if (nPoints) {
      int new_size = payloadSize + (2 * sizeof(int));
      pPayload = (uint8_t *)realloc(pPayload, new_size);
      pRun = pPayload + payloadSize;  //  recalculate the running pointer,
                                      //  since realloc may have moved memory
      payloadSize = new_size;

      //  Fetch and store the Record ID
      int record_id = pEdgeVectorRecordFeature->GetFieldAsInteger("RCID");
      *(int *)pRun = record_id;
      pRun += sizeof(int);

      //  Transcribe points to a buffer
      // We reduce the maximum number of points in the table to
      // MAX_VECTOR_POINTS using a naive algorithm skipping the proportional
      // part of them to avoid too deep a recursion and crash in DouglasPeucker
      // later
      int reduction_ratio = nPoints / MAX_VECTOR_POINTS + 1;
      int reduced_points = 0;

      double *ppd = (double *)malloc((nPoints / reduction_ratio + 1) * 2 *
                                     sizeof(double));
      double *ppr = ppd;

      for (int i = 0; i < nPoints; i++) {
        if (i % reduction_ratio == 0) {
          OGRPoint p;
          pLS->getPoint(i, &p);

          //  Calculate SM from chart common reference point
          double easting, northing;
          toSM(p.getY(), p.getX(), m_ref_lat, m_ref_lon, &easting, &northing);

          *ppr++ = easting;
          *ppr++ = northing;
          reduced_points++;
        }
      }
      nPoints = reduced_points;

      //      Reduce the LOD of this linestring
      std::vector<int> index_keep;
      if (nPoints > 5 && (m_LOD_meters > .01)) {
        index_keep.push_back(0);
        index_keep.push_back(nPoints - 1);

        DouglasPeucker(ppd, 0, nPoints - 1, m_LOD_meters, &index_keep);
        //               printf("DP Reduction: %d/%d\n", index_keep.GetCount(),
        //               nPoints);

      } else {
        index_keep.resize(nPoints);
        for (int i = 0; i < nPoints; i++) index_keep[i] = i;
      }

      //  Store the point count in the payload
      int nPointReduced = index_keep.size();
      *(int *)pRun = nPointReduced;
      pRun += sizeof(int);

      //  transcribe the (possibly) reduced linestring to the payload

      //  Grow the payload buffer
      int new_size_red = payloadSize + (nPointReduced * 2 * sizeof(float));
      pPayload = (uint8_t *)realloc(pPayload, new_size_red);
      pRun = pPayload + payloadSize;  //  recalculate the running pointer,
                                      //  since realloc may have moved memory
      payloadSize = new_size_red;

      float *npp = (float *)pRun;
      float *npp_run = npp;
      ppr = ppd;
      for (int ip = 0; ip < nPoints; ip++) {
        double x = *ppr++;
        double y = *ppr++;

        for (unsigned int j = 0; j < index_keep.size(); j++) {
          if (index_keep[j] == ip) {
            *npp_run++ = x;
            *npp_run++ = y;
            pRun += 2 * sizeof(float);
            break;
          }
        }
      }

      nFeatures++;

      free(ppd);
    }

    //    Next vector record
    feid++;
    delete pEdgeVectorRecordFeature;
    pEdgeVectorRecordFeature = poReader->ReadVector(feid, RCNM_VE);

  }  // while

  // Now we know the payload length and the Feature count
  if (nFeatures) {
    //  Now write the record out
    OSENC_VET_Record record;

    record.record_type = VECTOR_EDGE_NODE_TABLE_RECORD;
    record.record_length =
        sizeof(OSENC_VET_Record_Base) + payloadSize + sizeof(uint32_t);

    // Write out the record
    stream->Write(&record, sizeof(OSENC_VET_Record_Base));

    //  Write out the Feature(Object) count
    stream->Write(&nFeatures, sizeof(uint32_t));

    //  Write out the payload
    stream->Write(pPayload, payloadSize);
  }
  //  All done with buffer
  free(pPayload);

  //  Reset the S57Reader options
  papszReaderOptions =
      CSLSetNameValue(papszReaderOptions, S57O_RETURN_PRIMITIVES, "OFF");
  poReader->SetOptions(papszReaderOptions);
  CSLDestroy(papszReaderOptions);
}

void Osenc::CreateSENCVectorConnectedTableRecord200(Osenc_outstream *stream,
                                                    S57Reader *poReader) {
  //  We create the payload first, so we can calculate the total record length
  uint8_t *pPayload = NULL;
  int payloadSize = 0;
  uint8_t *pRun = pPayload;

  //  Set up the S57Reader options, adding RETURN_PRIMITIVES
  char **papszReaderOptions = NULL;
  papszReaderOptions = CSLSetNameValue(papszReaderOptions, S57O_UPDATES, "ON");
  papszReaderOptions =
      CSLSetNameValue(papszReaderOptions, S57O_RETURN_LINKAGES, "ON");
  papszReaderOptions =
      CSLSetNameValue(papszReaderOptions, S57O_RETURN_PRIMITIVES, "ON");
  poReader->SetOptions(papszReaderOptions);

  int feid = 0;
  OGRPoint *pP;
  OGRGeometry *pGeo;
  OGRFeature *pConnNodeRecordFeature = poReader->ReadVector(feid, RCNM_VC);
  int featureCount = 0;

  //  Read all the ConnectedVector Features
  while (NULL != pConnNodeRecordFeature) {
    if (pConnNodeRecordFeature->GetGeometryRef() != NULL) {
      pGeo = pConnNodeRecordFeature->GetGeometryRef();
      if (pGeo->getGeometryType() == wkbPoint) {
        int new_size = payloadSize + sizeof(int) + (2 * sizeof(float));
        pPayload = (uint8_t *)realloc(pPayload, new_size);
        pRun = pPayload + payloadSize;  //  recalculate the running pointer,
                                        //  since realloc may have moved memory
        payloadSize = new_size;

        //  Fetch and store the Record ID
        int record_id = pConnNodeRecordFeature->GetFieldAsInteger("RCID");
        *(int *)pRun = record_id;
        pRun += sizeof(int);

        pP = (OGRPoint *)pGeo;

        //  Calculate SM from chart common reference point
        double easting, northing;
        toSM(pP->getY(), pP->getX(), m_ref_lat, m_ref_lon, &easting, &northing);

        //                 MyPoint pd;
        //                 pd.x = easting;
        //                 pd.y = northing;
        //                 memcpy(pRun, &pd, sizeof(MyPoint));
        float *ps = (float *)pRun;
        *ps++ = easting;
        *ps = northing;

        featureCount++;
      }
    }

    //    Next vector record
    feid++;
    delete pConnNodeRecordFeature;
    pConnNodeRecordFeature = poReader->ReadVector(feid, RCNM_VC);
  }  // while

  // Now we know the payload length and the Feature count

  //  Now write the record out
  if (featureCount) {
    OSENC_VCT_Record record;

    record.record_type = VECTOR_CONNECTED_NODE_TABLE_RECORD;
    record.record_length =
        sizeof(OSENC_VCT_Record_Base) + payloadSize + sizeof(int);

    // Write out the record
    stream->Write(&record, sizeof(OSENC_VCT_Record_Base));

    //  Write out the Feature(Object) count
    stream->Write(&featureCount, sizeof(uint32_t));

    //  Write out the payload
    stream->Write(pPayload, payloadSize);
  }

  //  All done with buffer
  free(pPayload);

  //  Reset the S57Reader options
  papszReaderOptions =
      CSLSetNameValue(papszReaderOptions, S57O_RETURN_PRIMITIVES, "OFF");
  poReader->SetOptions(papszReaderOptions);
  CSLDestroy(papszReaderOptions);
}

bool Osenc::CreateSENCRecord200(OGRFeature *pFeature, Osenc_outstream *stream,
                                int mode, S57Reader *poReader) {
  // TODO
  //    if(pFeature->GetFID() == 207)
  //        int yyp = 4;

  //  Create the Feature Identification Record

  // Fetch the S57 Standard Object Class identifier
  OGRFeatureDefn *pFD = pFeature->GetDefnRef();
  int nOBJL = pFD->GetOBJL();

  OGRGeometry *pGeo = pFeature->GetGeometryRef();
  OGRwkbGeometryType gType = pGeo->getGeometryType();

  int primitive = 0;
  switch (gType) {
    case wkbLineString:
      primitive = GEO_LINE;
      break;
    case wkbPoint:
      primitive = GEO_POINT;
      break;
    case wkbPolygon:
      primitive = GEO_AREA;
      break;
    default:
      primitive = 0;
  }

  if (!WriteFIDRecord200(stream, nOBJL, pFeature->GetFID(), primitive))
    return false;

#define MAX_HDR_LINE 400

  //    char line[MAX_HDR_LINE + 1];
  wxString sheader;

  //    fprintf( fpOut, "OGRFeature(%s):%ld\n",
  //    pFeature->GetDefnRef()->GetName(), pFeature->GetFID() );

  //  In a loop, fetch attributes, and create OSENC Feature Attribute Records
  //  In the interests of output file size, DO NOT report fields that are not
  //  set.

  // TODO Debugging
  //     if(!strncmp(pFeature->GetDefnRef()->GetName(), "BOYLAT", 6))
  //         int yyp = 4;

  if (pFeature->GetFID() == 290) int yyp = 4;

  int payloadLength = 0;
  void *payloadBuffer = NULL;
  unsigned int payloadBufferLength = 0;

  for (int iField = 0; iField < pFeature->GetFieldCount(); iField++) {
    if (pFeature->IsFieldSet(iField)) {
      if ((iField == 1) || (iField > 7)) {
        OGRFieldDefn *poFDefn = pFeature->GetDefnRef()->GetFieldDefn(iField);
        // const char *pType = OGRFieldDefn::GetFieldTypeName(
        // poFDefn->GetType() );
        const char *pAttrName = poFDefn->GetNameRef();
        const char *pAttrVal = pFeature->GetFieldAsString(iField);

        //  Use the OCPN Registrar Manager to map attribute acronym to an
        //  identifier. The mapping is defined by the file
        //  {csv_dir}/s57attributes.csv
        int attributeID = m_pRegistrarMan->getAttributeID(pAttrName);

        //  Determine the {attribute_value_type} needed in the record
        int OGRvalueType = (int)poFDefn->GetType();
        int valueType = 0;

        // Check for special cases
        if (-1 == attributeID) {
          if (!strncmp(pAttrName, "PRIM", 4)) {
            attributeID = ATTRIBUTE_ID_PRIM;
          }
        }

#if 0
                /** Simple 32bit integer */                   OFTInteger = 0,
                /** List of 32bit integers */                 OFTIntegerList = 1,
                /** Double Precision floating point */        OFTReal = 2,
                /** List of doubles */                        OFTRealList = 3,
                /** String of ASCII chars */                  OFTString = 4,
                /** Array of strings */                       OFTStringList = 5,
                /** Double byte string (unsupported) */       OFTWideString = 6,
                /** List of wide strings (unsupported) */     OFTWideStringList = 7,
                /** Raw Binary data (unsupported) */          OFTBinary = 8
#endif
        switch (OGRvalueType) {
          case 0:  // Single integer
          {
            valueType = OGRvalueType;

            if (payloadBufferLength < 4) {
              payloadBuffer = realloc(payloadBuffer, 4);
              payloadBufferLength = 4;
            }

            int aValue = pFeature->GetFieldAsInteger(iField);
            memcpy(payloadBuffer, &aValue, sizeof(int));
            payloadLength = sizeof(int);

            break;
          }
          case 1:  // Integer list
          {
            valueType = OGRvalueType;

            int nCount = 0;
            const int *aValueList =
                pFeature->GetFieldAsIntegerList(iField, &nCount);

            if (payloadBufferLength < nCount * sizeof(int)) {
              payloadBuffer = realloc(payloadBuffer, nCount * sizeof(int));
              payloadBufferLength = nCount * sizeof(int);
            }

            int *pBuffRun = (int *)payloadBuffer;
            for (int i = 0; i < nCount; i++) {
              *pBuffRun++ = aValueList[i];
            }
            payloadLength = nCount * sizeof(int);

            break;
          }
          case 2:  // Single double precision real
          {
            valueType = OGRvalueType;

            if (payloadBufferLength < sizeof(double)) {
              payloadBuffer = realloc(payloadBuffer, sizeof(double));
              payloadBufferLength = sizeof(double);
            }

            double aValue = pFeature->GetFieldAsDouble(iField);
            memcpy(payloadBuffer, &aValue, sizeof(double));
            payloadLength = sizeof(double);

            break;
          }

          case 3:  // List of double precision real
          {
            valueType = OGRvalueType;

            int nCount = 0;
            const double *aValueList =
                pFeature->GetFieldAsDoubleList(iField, &nCount);

            if (payloadBufferLength < nCount * sizeof(double)) {
              payloadBuffer = realloc(payloadBuffer, nCount * sizeof(double));
              payloadBufferLength = nCount * sizeof(double);
            }

            double *pBuffRun = (double *)payloadBuffer;
            for (int i = 0; i < nCount; i++) {
              *pBuffRun++ = aValueList[i];
            }
            payloadLength = nCount * sizeof(double);

            break;
          }

          case 4:  // Ascii String
          {
            valueType = OGRvalueType;
            const char *pAttrVal = pFeature->GetFieldAsString(iField);

            wxString wxAttrValue;

            if ((0 == strncmp("NOBJNM", pAttrName, 6)) ||
                (0 == strncmp("NINFOM", pAttrName, 6)) ||
                (0 == strncmp("NPLDST", pAttrName, 6)) ||
                (0 == strncmp("NTXTDS", pAttrName, 6))) {
              if (poReader->GetNall() ==
                  2) {  // ENC is using UCS-2 / UTF-16 encoding
                wxMBConvUTF16 conv;
                wxString att_conv(pAttrVal, conv);
                att_conv.RemoveLast();  // Remove the \037 that terminates
                                        // UTF-16 strings in S57
                att_conv.Replace(_T("\n"),
                                 _T("|"));  // Replace  <new line> with special
                                            // break character
                wxAttrValue = att_conv;
              } else if (poReader->GetNall() ==
                         1) {  // ENC is using Lex level 1 (ISO 8859_1) encoding
                wxCSConv conv(_T("iso8859-1"));
                wxString att_conv(pAttrVal, conv);
                wxAttrValue = att_conv;
              }
            } else {
              if (poReader->GetAall() ==
                  1) {  // ENC is using Lex level 1 (ISO 8859_1) encoding for
                        // "General Text"
                wxCSConv conv(_T("iso8859-1"));
                wxString att_conv(pAttrVal, conv);
                wxAttrValue = att_conv;
              } else
                wxAttrValue =
                    wxString(pAttrVal);  // ENC must be using Lex level 0
                                         // (ASCII) encoding for "General Text"
            }

            unsigned int stringPayloadLength = 0;

            wxCharBuffer buffer;
            if (wxAttrValue.Length()) {  // need to explicitely encode as UTF8
              buffer = wxAttrValue.ToUTF8();
              pAttrVal = buffer.data();
              stringPayloadLength = strlen(buffer.data());
            }

            if (stringPayloadLength) {
              if (payloadBufferLength < stringPayloadLength + 1) {
                payloadBuffer = realloc(payloadBuffer, stringPayloadLength + 1);
                payloadBufferLength = stringPayloadLength + 1;
              }

              strcpy((char *)payloadBuffer, pAttrVal);
              payloadLength = stringPayloadLength + 1;
            } else
              attributeID = -1;  // cancel this attribute record

            break;
          }

          default:
            valueType = -1;
            break;
        }

        if (-1 != attributeID) {
          // Build the record
          int recordLength =
              sizeof(OSENC_Attribute_Record_Base) + payloadLength;

          //  Get a reference to the class persistent buffer
          unsigned char *pBuffer = getBuffer(recordLength);

          OSENC_Attribute_Record *pRecord = (OSENC_Attribute_Record *)pBuffer;
          memset(pRecord, 0, sizeof(OSENC_Attribute_Record));
          pRecord->record_type = FEATURE_ATTRIBUTE_RECORD;
          pRecord->record_length = recordLength;
          pRecord->attribute_type = attributeID;
          pRecord->attribute_value_type = valueType;
          memcpy(&pRecord->payload, payloadBuffer, payloadLength);

          // Write the record out....
          size_t targetCount = recordLength;
          if (!stream->Write(pBuffer, targetCount).IsOk()) {
            free(payloadBuffer);
            return false;
          }
        }
      }
    }
  }
  if (wkbPoint == pGeo->getGeometryType()) {
    OGRPoint *pp = (OGRPoint *)pGeo;
    int nqual = pp->getnQual();
    if (10 != nqual)  // only add attribute if nQual is not "precisely known"
    {
      int attributeID = m_pRegistrarMan->getAttributeID("QUAPOS");
      int valueType = 0;
      if (-1 != attributeID) {
        if (payloadBufferLength < 4) {
          payloadBuffer = realloc(payloadBuffer, 4);
          payloadBufferLength = 4;
        }

        memcpy(payloadBuffer, &nqual, sizeof(int));
        payloadLength = sizeof(int);
        // Build the record
        int recordLength = sizeof(OSENC_Attribute_Record_Base) + payloadLength;

        //  Get a reference to the class persistent buffer
        unsigned char *pBuffer = getBuffer(recordLength);

        OSENC_Attribute_Record *pRecord = (OSENC_Attribute_Record *)pBuffer;
        memset(pRecord, 0, sizeof(OSENC_Attribute_Record));
        pRecord->record_type = FEATURE_ATTRIBUTE_RECORD;
        pRecord->record_length = recordLength;
        pRecord->attribute_type = attributeID;
        pRecord->attribute_value_type = valueType;
        memcpy(&pRecord->payload, payloadBuffer, payloadLength);

        // Write the record out....
        size_t targetCount = recordLength;
        if (!stream->Write(pBuffer, targetCount).IsOk()) {
          free(payloadBuffer);
          return false;
        }
      }
    }
  }

  free(payloadBuffer);

#if 0
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

#endif

  if ((pGeo != NULL)) {
    wxString msg;

    OGRwkbGeometryType gType = pGeo->getGeometryType();
    switch (gType) {
      case wkbLineString: {
        if (!CreateLineFeatureGeometryRecord200(poReader, pFeature, stream))
          return false;

        break;
      }

      case wkbPoint: {
        OSENC_PointGeometry_Record record;
        record.record_type = FEATURE_GEOMETRY_RECORD_POINT;
        record.record_length = sizeof(record);

        int wkb_len = pGeo->WkbSize();
        unsigned char *pwkb_buffer = (unsigned char *)malloc(wkb_len);

        //  Get the GDAL data representation
        pGeo->exportToWkb(wkbNDR, pwkb_buffer);

        int nq_len = 4;  // nQual length
        unsigned char *ps = pwkb_buffer;

        ps += 5 + nq_len;
        double *psd = (double *)ps;

        double lat, lon;
#ifdef __ARM_ARCH
        __attribute__((aligned(16))) double lata, lona;
        unsigned char *pucsd = (unsigned char *)psd;

        memcpy(&lona, pucsd, sizeof(double));
        pucsd += sizeof(double);
        memcpy(&lata, pucsd, sizeof(double));
        lon = lona;
        lat = lata;
#else
        lon = *psd++;  // fetch the point
        lat = *psd;
#endif

        free(pwkb_buffer);

        record.lat = lat;
        record.lon = lon;

        // Write the record out....
        size_t targetCount = record.record_length;
        if (!stream->Write(&record, targetCount).IsOk()) return false;

        break;
      }

      case wkbMultiPoint:
      case wkbMultiPoint25D: {
        if (!CreateMultiPointFeatureGeometryRecord200(pFeature, stream))
          return false;
        break;
      }

#if 1
      //      Special case, polygons are handled separately
      case wkbPolygon: {
        if (!CreateAreaFeatureGeometryRecord200(poReader, pFeature, stream))
          return false;

        break;
      }
#endif
      //      All others
      default:
        msg = _T("   Warning: Unimplemented ogr geotype record ");
        wxLogMessage(msg);

        break;
    }  // switch
  }
  return true;
}

//      Build PolyGeo Object from OSENC200 file record
//      Return an integer count of bytes consumed from the record in creating
//      the PolyTessGeo
PolyTessGeo *Osenc::BuildPolyTessGeo(_OSENC_AreaGeometry_Record_Payload *record,
                                     unsigned char **next_byte) {
  PolyTessGeo *pPTG = new PolyTessGeo();

  pPTG->SetExtents(record->extent_w_lon, record->extent_s_lat,
                   record->extent_e_lon, record->extent_n_lat);

  unsigned int n_TriPrim = record->triprim_count;
  int nContours = record->contour_count;

  //  Get a pointer to the payload
  void *payLoad = &record->payLoad;

  //  skip over the contour vertex count array, for now  TODO
  // uint8_t *pTriPrims = (uint8_t *)payLoad + (nContours * sizeof(uint32_t));

  //  Create the head of the linked list of TriPrims
  PolyTriGroup *ppg = new PolyTriGroup;
  ppg->m_bSMSENC = true;
  ppg->data_type = DATA_TYPE_DOUBLE;

  ppg->nContours = nContours;

  ppg->pn_vertex = (int *)malloc(nContours * sizeof(int));
  int *pctr = ppg->pn_vertex;

  //  The point count array is the first element in the payload, length is known
  int *contour_pointcount_array_run = (int *)payLoad;
  for (int i = 0; i < nContours; i++) {
    *pctr++ = *contour_pointcount_array_run++;
  }

  //  Read Raw Geometry
  ppg->pgroup_geom = NULL;

  //  Now the triangle primitives

  TriPrim **p_prev_triprim = &(ppg->tri_prim_head);

  //  Read the PTG_Triangle Geometry in a loop
  unsigned int tri_type;
  int nvert;
  int nvert_max = 0;
  int total_byte_size = 2 * sizeof(float);

  uint8_t *pPayloadRun =
      (uint8_t *)contour_pointcount_array_run;  // Points to the start of the
                                                // triangle primitives

  for (unsigned int i = 0; i < n_TriPrim; i++) {
    tri_type = *pPayloadRun++;
    nvert = *(uint32_t *)pPayloadRun;
    pPayloadRun += sizeof(uint32_t);

    TriPrim *tp = new TriPrim;
    *p_prev_triprim = tp;  // make the link
    p_prev_triprim = &(tp->p_next);
    tp->p_next = NULL;

    tp->type = tri_type;
    tp->nVert = nvert;

    nvert_max =
        wxMax(nvert_max, nvert);  // Keep a running tab of largest vertex count

    //  Read the triangle primitive bounding box as lat/lon
    double *pbb = (double *)pPayloadRun;

    double minxt, minyt, maxxt, maxyt;

#ifdef __ARM_ARCH
    double __attribute__((aligned(16))) abox[4];
    unsigned char *pucbb = (unsigned char *)pPayloadRun;
    memcpy(&abox[0], pucbb, 4 * sizeof(double));

    minxt = abox[0];
    maxxt = abox[1];
    minyt = abox[2];
    maxyt = abox[3];
#else
    minxt = *pbb++;
    maxxt = *pbb++;
    minyt = *pbb++;
    maxyt = *pbb;
#endif

    tp->tri_box.Set(minyt, minxt, maxyt, maxxt);

    pPayloadRun += 4 * sizeof(double);

    int byte_size = nvert * 2 * sizeof(float);  // the vertices
    total_byte_size += byte_size;

    tp->p_vertex = (double *)pPayloadRun;

    pPayloadRun += byte_size;
  }

  if (next_byte) *next_byte = pPayloadRun;

  //  Convert the vertex arrays into a single float memory allocation to enable
  //  efficient access later
  unsigned char *vbuf = (unsigned char *)malloc(total_byte_size);

  TriPrim *p_tp = ppg->tri_prim_head;
  unsigned char *p_run = vbuf;
  while (p_tp) {
    memcpy(p_run, p_tp->p_vertex, p_tp->nVert * 2 * sizeof(float));
    p_tp->p_vertex = (double *)p_run;
    p_run += p_tp->nVert * 2 * sizeof(float);
    p_tp = p_tp->p_next;  // pick up the next in chain
  }
  ppg->bsingle_alloc = true;
  ppg->single_buffer = vbuf;
  ppg->single_buffer_size = total_byte_size;
  ppg->data_type = DATA_TYPE_FLOAT;

  pPTG->SetPPGHead(ppg);
  pPTG->SetnVertexMax(nvert_max);

  pPTG->Set_OK(true);

  return pPTG;
}

bool Osenc::CreateCOVRTables(S57Reader *poReader,
                             S57ClassRegistrar *poRegistrar) {
  poReader->Rewind();

  OGRFeature *pFeat;
  int catcov;
  float LatMax, LatMin, LonMax, LonMin;
  LatMax = -90.;
  LatMin = 90.;
  LonMax = -179.;
  LonMin = 179.;

  m_pCOVRTablePoints = NULL;
  m_pCOVRTable = NULL;

  //  Create arrays to hold geometry objects temporarily
  MyFloatPtrArray *pAuxPtrArray = new MyFloatPtrArray;
  std::vector<int> auxCntArray, noCovrCntArray;

  MyFloatPtrArray *pNoCovrPtrArray = new MyFloatPtrArray;

  // Get the first M_COVR object
  pFeat = GetChartFirstM_COVR(catcov, poReader, poRegistrar);

  while (pFeat) {
    //    Get the next M_COVR feature, and create possible additional entries
    //    for COVR
    OGRPolygon *poly = (OGRPolygon *)(pFeat->GetGeometryRef());
    OGRLinearRing *xring = poly->getExteriorRing();

    int npt = xring->getNumPoints();

    float *pf = NULL;

    if (npt >= 3) {
      pf = (float *)malloc(2 * npt * sizeof(float));
      float *pfr = pf;

      for (int i = 0; i < npt; i++) {
        OGRPoint p;
        xring->getPoint(i, &p);

        if (catcov == 1) {
          LatMax = fmax(LatMax, p.getY());
          LatMin = fmin(LatMin, p.getY());
          LonMax = fmax(LonMax, p.getX());
          LonMin = fmin(LonMin, p.getX());
        }

        pfr[0] = p.getY();  // lat
        pfr[1] = p.getX();  // lon

        pfr += 2;
      }

      if (catcov == 1) {
        pAuxPtrArray->Add(pf);
        auxCntArray.push_back(npt);
      } else if (catcov == 2) {
        pNoCovrPtrArray->Add(pf);
        noCovrCntArray.push_back(npt);
      } else
        free(pf);
    }

    delete pFeat;
    pFeat = GetChartNextM_COVR(catcov, poReader);
  }  // while

  //    Allocate the storage

  m_nCOVREntries = auxCntArray.size();

  //    If only one M_COVR,CATCOV=1 object was found,
  //    assign the geometry to the one and only COVR

  if (m_nCOVREntries == 1) {
    m_pCOVRTablePoints = (int *)malloc(sizeof(int));
    *m_pCOVRTablePoints = auxCntArray[0];
    m_pCOVRTable = (float **)malloc(sizeof(float *));
    *m_pCOVRTable = (float *)malloc(auxCntArray[0] * 2 * sizeof(float));
    memcpy(*m_pCOVRTable, pAuxPtrArray->Item(0),
           auxCntArray[0] * 2 * sizeof(float));
  }

  else if (m_nCOVREntries > 1) {
    //    Create new COVR entries
    m_pCOVRTablePoints = (int *)malloc(m_nCOVREntries * sizeof(int));
    m_pCOVRTable = (float **)malloc(m_nCOVREntries * sizeof(float *));

    for (unsigned int j = 0; j < (unsigned int)m_nCOVREntries; j++) {
      m_pCOVRTablePoints[j] = auxCntArray[j];
      m_pCOVRTable[j] = (float *)malloc(auxCntArray[j] * 2 * sizeof(float));
      memcpy(m_pCOVRTable[j], pAuxPtrArray->Item(j),
             auxCntArray[j] * 2 * sizeof(float));
    }
  }

  else  // strange case, found no CATCOV=1 M_COVR objects
  {
    wxString msg(_T("   ENC contains no useable M_COVR, CATCOV=1 features:  "));
    msg.Append(m_FullPath000);
    wxLogMessage(msg);
  }

  //      And for the NoCovr regions
  m_nNoCOVREntries = noCovrCntArray.size();

  if (m_nNoCOVREntries) {
    //    Create new NoCOVR entries
    m_pNoCOVRTablePoints = (int *)malloc(m_nNoCOVREntries * sizeof(int));
    m_pNoCOVRTable = (float **)malloc(m_nNoCOVREntries * sizeof(float *));

    for (unsigned int j = 0; j < (unsigned int)m_nNoCOVREntries; j++) {
      int npoints = noCovrCntArray[j];
      m_pNoCOVRTablePoints[j] = npoints;
      m_pNoCOVRTable[j] = (float *)malloc(npoints * 2 * sizeof(float));
      memcpy(m_pNoCOVRTable[j], pNoCovrPtrArray->Item(j),
             npoints * 2 * sizeof(float));
    }
  } else {
    m_pNoCOVRTablePoints = NULL;
    m_pNoCOVRTable = NULL;
  }

  for (unsigned int j = 0; j < (unsigned int)m_nNoCOVREntries; j++)
    free(pNoCovrPtrArray->Item(j));
  for (unsigned int j = 0; j < (unsigned int)m_nCOVREntries; j++)
    free(pAuxPtrArray->Item(j));

  delete pAuxPtrArray;
  delete pNoCovrPtrArray;

  if (0 == m_nCOVREntries) {  // fallback
    wxString msg(_T("   ENC contains no M_COVR features:  "));
    msg.Append(m_FullPath000);
    wxLogMessage(msg);

    msg = _T("   Calculating Chart Extents as fallback.");
    wxLogMessage(msg);

    OGREnvelope Env;

    if (poReader->GetExtent(&Env, true) == OGRERR_NONE) {
      LatMax = Env.MaxY;
      LonMax = Env.MaxX;
      LatMin = Env.MinY;
      LonMin = Env.MinX;

      m_nCOVREntries = 1;
      m_pCOVRTablePoints = (int *)malloc(sizeof(int));
      *m_pCOVRTablePoints = 4;
      m_pCOVRTable = (float **)malloc(sizeof(float *));
      float *pf = (float *)malloc(2 * 4 * sizeof(float));
      *m_pCOVRTable = pf;
      float *pfe = pf;

      *pfe++ = LatMax;
      *pfe++ = LonMin;

      *pfe++ = LatMax;
      *pfe++ = LonMax;

      *pfe++ = LatMin;
      *pfe++ = LonMax;

      *pfe++ = LatMin;
      *pfe++ = LonMin;

    } else {
      wxString msg(_T("   Cannot calculate Extents for ENC:  "));
      msg.Append(m_FullPath000);
      wxLogMessage(msg);

      return false;  // chart is completely unusable
    }
  }

  //    Populate the oSENC clone of the chart's extent structure
  m_extent.NLAT = LatMax;
  m_extent.SLAT = LatMin;
  m_extent.ELON = LonMax;
  m_extent.WLON = LonMin;

  return true;
}

OGRFeature *Osenc::GetChartFirstM_COVR(int &catcov, S57Reader *pENCReader,
                                       S57ClassRegistrar *poRegistrar) {
  OGRFeature *rv = NULL;

  if ((NULL != pENCReader) && (NULL != poRegistrar)) {
    //      Select the proper class
    poRegistrar->SelectClass("M_COVR");

    //        OGRFeatureDefn *M_COVRdef = S57GenerateObjectClassDefn(
    //        poRegistrar, 302, 0);

    //      find this feature
    bool bFound = false;
    OGRFeature *pobjectDef = pENCReader->ReadNextFeature(/*M_COVRdef*/);
    while (!bFound) {
      if (pobjectDef) {
        OGRFeatureDefn *poDefn = pobjectDef->GetDefnRef();
        if (poDefn && (poDefn->GetOBJL() == 302 /*poRegistrar->GetOBJL()*/)) {
          //  Fetch the CATCOV attribute
          catcov = pobjectDef->GetFieldAsInteger("CATCOV");
          rv = pobjectDef;
          break;
        } else
          delete pobjectDef;
      } else {
        break;
      }
      pobjectDef = pENCReader->ReadNextFeature();
    }
  }

  return rv;
}

OGRFeature *Osenc::GetChartNextM_COVR(int &catcov, S57Reader *pENCReader) {
  catcov = -1;

  if (pENCReader) {
    bool bFound = false;
    OGRFeature *pobjectDef = pENCReader->ReadNextFeature();

    while (!bFound) {
      if (pobjectDef) {
        OGRFeatureDefn *poDefn = pobjectDef->GetDefnRef();
        if (poDefn && (poDefn->GetOBJL() == 302)) {
          //  Fetch the CATCOV attribute
          catcov = pobjectDef->GetFieldAsInteger("CATCOV");
          return pobjectDef;
        } else
          delete pobjectDef;

      } else
        return NULL;

      pobjectDef = pENCReader->ReadNextFeature();
    }
  }

  return NULL;
}

int Osenc::GetBaseFileInfo(const wxString &FullPath000,
                           const wxString &SENCFileName) {
  wxFileName SENCfile = wxFileName(SENCFileName);

  //  Take a quick scan of the 000 file to get some basic attributes of the
  //  exchange set.
  if (!GetBaseFileAttr(FullPath000)) {
    return ERROR_BASEFILE_ATTRIBUTES;
  }

  OGRS57DataSource oS57DS;
  oS57DS.SetS57Registrar(m_poRegistrar);

  bool b_current_debug = g_bGDAL_Debug;
  g_bGDAL_Debug = false;

  //  Ingest the .000 cell, with updates applied

  if (ingestCell(&oS57DS, FullPath000, SENCfile.GetPath())) {
    errorMessage = _T("Error ingesting: ") + FullPath000;
    return ERROR_INGESTING000;
  }

  S57Reader *poReader = oS57DS.GetModule(0);

  CalculateExtent(poReader, m_poRegistrar);

  g_bGDAL_Debug = b_current_debug;

  //    delete poReader;

  return SENC_NO_ERROR;
}

bool Osenc::CalculateExtent(S57Reader *poReader,
                            S57ClassRegistrar *poRegistrar) {
  poReader->Rewind();

  OGRFeature *pFeat;
  int catcov;
  float LatMax, LatMin, LonMax, LonMin;
  LatMax = -90.;
  LatMin = 90.;
  LonMax = -179.;
  LonMin = 179.;

  m_pCOVRTablePoints = NULL;
  m_pCOVRTable = NULL;

  //     //  Create arrays to hold geometry objects temporarily
  //     MyFloatPtrArray *pAuxPtrArray = new MyFloatPtrArray;
  //     wxArrayInt *pAuxCntArray = new wxArrayInt;
  //
  //     MyFloatPtrArray *pNoCovrPtrArray = new MyFloatPtrArray;
  //     wxArrayInt *pNoCovrCntArray = new wxArrayInt;

  // Get the first M_COVR object
  pFeat = GetChartFirstM_COVR(catcov, poReader, poRegistrar);

  while (pFeat) {
    //    Get the next M_COVR feature, and create possible additional entries
    //    for COVR
    OGRPolygon *poly = (OGRPolygon *)(pFeat->GetGeometryRef());
    OGRLinearRing *xring = poly->getExteriorRing();

    int npt = xring->getNumPoints();

    if (npt >= 3) {
      for (int i = 0; i < npt; i++) {
        OGRPoint p;
        xring->getPoint(i, &p);

        if (catcov == 1) {
          LatMax = fmax(LatMax, p.getY());
          LatMin = fmin(LatMin, p.getY());
          LonMax = fmax(LonMax, p.getX());
          LonMin = fmin(LonMin, p.getX());
        }
      }
    }

    delete pFeat;
    pFeat = GetChartNextM_COVR(catcov, poReader);
  }  // while

  //    Populate the oSENC clone of the chart's extent structure
  m_extent.NLAT = LatMax;
  m_extent.SLAT = LatMin;
  m_extent.ELON = LonMax;
  m_extent.WLON = LonMin;

  return true;
}

void Osenc::InitializePersistentBuffer(void) {
  pBuffer = (unsigned char *)malloc(1024);
  bufferSize = 1024;
}

unsigned char *Osenc::getBuffer(size_t length) {
  if (length > bufferSize) {
    pBuffer = (unsigned char *)realloc(pBuffer, length * 2);
    bufferSize = length * 2;
  }

  return pBuffer;
}
