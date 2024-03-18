/******************************************************************************
 *
 * Project:  S-57 Translator
 * Purpose:  Declarations for S-57 translator not including the
 *           binding onto OGRLayer/DataSource/Driver which are found in
 *           ogr_s57.h.
 * Author:   Frank Warmerdam, warmerda@home.com
 *
 ******************************************************************************
 * Copyright (c) 1999, Frank Warmerdam
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 *
 *
 */

#ifndef _S57_H_INCLUDED
#define _S57_H_INCLUDED

#include "gdal/ogr_feature.h"
#include "iso8211.h"
#include "S57ClassRegistrar.h"

class S57Reader;

char **S57FileCollector(const char *pszDataset);

#define EMPTY_NUMBER_MARKER 2147483641 /* MAXINT-6 */

/* -------------------------------------------------------------------- */
/*      Various option strings.                                         */
/* -------------------------------------------------------------------- */
#define S57O_UPDATES "UPDATES"
#define S57O_LNAM_REFS "LNAM_REFS"
#define S57O_SPLIT_MULTIPOINT "SPLIT_MULTIPOINT"
#define S57O_ADD_SOUNDG_DEPTH "ADD_SOUNDG_DEPTH"
#define S57O_PRESERVE_EMPTY_NUMBERS "PRESERVE_EMPTY_NUMBERS"
#define S57O_RETURN_PRIMITIVES "RETURN_PRIMITIVES"
#define S57O_RETURN_LINKAGES "RETURN_LINKAGES"

#define S57M_UPDATES 0x01
#define S57M_LNAM_REFS 0x02
#define S57M_SPLIT_MULTIPOINT 0x04
#define S57M_ADD_SOUNDG_DEPTH 0x08
#define S57M_PRESERVE_EMPTY_NUMBERS 0x10
#define S57M_RETURN_PRIMITIVES 0x20
#define S57M_RETURN_LINKAGES 0x40

/* -------------------------------------------------------------------- */
/*      RCNM values.                                                    */
/* -------------------------------------------------------------------- */

#define RCNM_FE 100 /* Feature record */

#define RCNM_VI 110 /* Isolated Node */
#define RCNM_VC 120 /* Connected Node */
#define RCNM_VE 130 /* Edge */
#define RCNM_VF 140 /* Face */

#define OGRN_VI "IsolatedNode"
#define OGRN_VC "ConnectedNode"
#define OGRN_VE "Edge"
#define OGRN_VF "Face"

/* -------------------------------------------------------------------- */
/*      FRID PRIM values.                                               */
/* -------------------------------------------------------------------- */
#define PRIM_P 1 /* point feature */
#define PRIM_L 2 /* line feature */
#define PRIM_A 3 /* area feature */
#define PRIM_N 4 /* non-spatial feature  */

/************************************************************************/
/*                          S57ClassRegistrar                           */
/************************************************************************/

#define MAX_CLASSES 23000
#define MAX_ATTRIBUTES 25000

/************************************************************************/
/*                            DDFRecordIndex                            */
/*                                                                      */
/*      Maintain an index of DDF records based on an integer key.       */
/************************************************************************/

typedef struct {
  int nKey;
  DDFRecord *poRecord;
} DDFIndexedRecord;

class DDFRecordIndex {
  int bSorted;

  int nRecordCount;
  int nRecordMax;

  int nLastObjlPos; /* rjensen. added for FindRecordByObjl() */
  int nLastObjl;    /* rjensen. added for FindRecordByObjl() */

  DDFIndexedRecord *pasRecords;

  void Sort();

public:
  DDFRecordIndex();
  ~DDFRecordIndex();

  void AddRecord(int nKey, DDFRecord *);
  int RemoveRecord(int nKey);

  DDFRecord *FindRecord(int nKey);

  DDFRecord *FindRecordByObjl(
      int nObjl); /* rjensen. added for FindRecordByObjl() */

  void Clear();

  int GetCount() { return nRecordCount; }
  DDFRecord *GetByIndex(int i);
};

/************************************************************************/
/*                              S57Reader                               */
/************************************************************************/
typedef bool (*CallBackFunction)(void);

class S57Reader {
  S57ClassRegistrar *poRegistrar;

  int nFDefnCount;
  OGRFeatureDefn **papoFDefnList;

  char *pszModuleName;
  char *pszDSNM;

  DDFModule *poModule;

  int nCOMF; /* Coordinate multiplier */
  int nSOMF; /* Vertical (sounding) multiplier */
  int nCSCL; /* Chart Scale (from DSPM record) */

  int bFileIngested;
  DDFRecordIndex oVI_Index;
  DDFRecordIndex oVC_Index;
  DDFRecordIndex oVE_Index;
  DDFRecordIndex oVF_Index;

  int nNextVIIndex;
  int nNextVCIndex;
  int nNextVEIndex;
  int nNextVFIndex;

  int nNextFEIndex;
  DDFRecordIndex oFE_Index;

  char **papszOptions;

  int nOptionFlags;

  int iPointOffset;
  OGRFeature *poMultiPoint;

  void ClearPendingMultiPoint();
  OGRFeature *NextPendingMultiPoint();

  OGRFeature *AssembleFeature(DDFRecord *, OGRFeatureDefn *);

  void ApplyObjectClassAttributes(DDFRecord *, OGRFeature *);
  void GenerateLNAMAndRefs(DDFRecord *, OGRFeature *);
  void GenerateFSPTAttributes(DDFRecord *, OGRFeature *);

  void AssembleSoundingGeometry(DDFRecord *, OGRFeature *);
  void AssemblePointGeometry(DDFRecord *, OGRFeature *);
  void AssembleLineGeometry(DDFRecord *, OGRFeature *);
  void AssembleAreaGeometry(DDFRecord *, OGRFeature *);

  OGRFeatureDefn *FindFDefn(DDFRecord *);
  int ParseName(DDFField *, int = 0, int * = NULL);

  int ApplyRecordUpdate(DDFRecord *, DDFRecord *);

  int bMissingWarningIssued;
  int bAttrWarningIssued;

  int Nall;
  int Aall;

public:
  S57Reader(const char *);
  ~S57Reader();

  int FetchPoint(int, int, double *, double *, double * = NULL, int * = NULL);

  void SetClassBased(S57ClassRegistrar *);
  void SetOptions(char **);
  int GetOptionFlags() { return nOptionFlags; }

  int Open(int bTestOpen);
  void Close();
  DDFModule *GetModule() { return poModule; }
  const char *GetDSNM() { return pszDSNM; }
  int GetCSCL() { return nCSCL; }

  int Ingest(CallBackFunction pcallback = NULL);
  int ApplyUpdates(DDFModule *, int);
  int FindAndApplyUpdates(const char *pszPath = NULL);

  void Rewind();
  OGRFeature *ReadNextFeature(OGRFeatureDefn * = NULL);
  OGRFeature *ReadFeature(int nFID, OGRFeatureDefn * = NULL);
  OGRFeature *ReadVector(int nFID, int nRCNM);

  int GetNextFEIndex(int nRCNM = 100);
  void SetNextFEIndex(int nNewIndex, int nRCNM = 100);

  void AddFeatureDefn(OGRFeatureDefn *);

  int CollectClassList(int *, int);

  OGRErr GetExtent(OGREnvelope *psExtent, int bForce);

  int GetNall() { return Nall; }
  int GetAall() { return Aall; }

  int GetFeatureCount() { return oFE_Index.GetCount(); }
};

/************************************************************************/
/*                              S57Writer                               */
/************************************************************************/

class S57Writer {
public:
  S57Writer();
  ~S57Writer();

  void SetClassBased(S57ClassRegistrar *);
  int CreateS57File(const char *pszFilename);
  int Close();

  int WriteGeometry(DDFRecord *, int, double *, double *, double *);
  int WriteATTF(DDFRecord *, OGRFeature *);
  int WritePrimitive(OGRFeature *poFeature);
  int WriteCompleteFeature(OGRFeature *poFeature);
  int WriteDSID(const char *pszDSNM = NULL, const char *pszISDT = NULL,
                const char *pszSTED = NULL, int nAGEN = 0,
                const char *pszCOMT = NULL);
  int WriteDSPM(int nScale = 0);

private:
  DDFModule *poModule;
  S57ClassRegistrar *poRegistrar;

  int nNext0001Index;

  DDFRecord *MakeRecord();

  int nCOMF; /* Coordinate multiplier */
  int nSOMF; /* Vertical (sounding) multiplier */
};

/* -------------------------------------------------------------------- */
/*      Functions to create OGRFeatureDefns.                            */
/* -------------------------------------------------------------------- */
void CPL_DLL S57GenerateStandardAttributes(OGRFeatureDefn *, int);
OGRFeatureDefn CPL_DLL *S57GenerateGeomFeatureDefn(OGRwkbGeometryType, int);
OGRFeatureDefn CPL_DLL *S57GenerateObjectClassDefn(S57ClassRegistrar *, int,
                                                   int);
OGRFeatureDefn CPL_DLL *S57GenerateVectorPrimitiveFeatureDefn(int, int);

#endif /* ndef _S57_H_INCLUDED */
