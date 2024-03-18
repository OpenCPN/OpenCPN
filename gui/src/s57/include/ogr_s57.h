/******************************************************************************
 *
 * Project:  S-57 Translator
 * Purpose:  Declarations for classes binding S57 support onto OGRLayer,
 *           OGRDataSource and OGRDriver.  See also s57.h.
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
 */

#ifndef _OGR_S57_H_INCLUDED
#define _OGR_S57_H_INCLUDED

#include "gdal/ogrsf_frmts.h"
#include "s57.h"

//    Misc error return codes
#define BAD_FILE 10
#define BAD_HEADER 11
#define BAD_OPEN 12
#define BAD_UPDATE 30

class OGRS57DataSource;

/************************************************************************/
/*                             OGRS57Layer                              */
/*                                                                      */
/*      Represents all features of a particular S57 object class.       */
/************************************************************************/

class OGRS57Layer : public OGRLayer {
  OGRGeometry *poFilterGeom;

  OGRS57DataSource *poDS;

  OGRFeatureDefn *poFeatureDefn;

  int nCurrentModule;
  int nRCNM;
  int nOBJL;
  int nNextFEIndex;
  int nFeatureCount;

public:
  OGRS57Layer(OGRS57DataSource *poDS, OGRFeatureDefn *, int nFeatureCount = -1,
              int nOBJL = -1);
  virtual ~OGRS57Layer();

  OGRGeometry *GetSpatialFilter() { return poFilterGeom; }
  void SetSpatialFilter(OGRGeometry *);

  void ResetReading();
  OGRFeature *GetNextFeature();
  OGRFeature *GetNextUnfilteredFeature();
  virtual OGRFeature *GetFeature(long nFeatureId);

  virtual int GetFeatureCount(int bForce = TRUE);
  virtual OGRErr GetExtent(OGREnvelope *psExtent, int bForce = TRUE);

  OGRFeatureDefn *GetLayerDefn() { return poFeatureDefn; }

  virtual OGRErr CreateFeature(OGRFeature *poFeature);
  int TestCapability(const char *);

  virtual OGRSpatialReference *GetSpatialRef();
};

/************************************************************************/
/*                          OGRS57DataSource                            */
/************************************************************************/
typedef bool (*CallBackFunction)(void);

class OGRS57DataSource {
  char *pszName;

  int nLayers;
  OGRS57Layer **papoLayers;

  OGRSpatialReference *poSpatialRef;

  char **papszOptions;

  int nModules;
  S57Reader **papoModules;

  S57Writer *poWriter;

  S57ClassRegistrar *poRegistrar;

  int bClassCountSet;
  int anClassCount[MAX_CLASSES];

  int bExtentsSet;
  OGREnvelope oExtents;

public:
  OGRS57DataSource();
  ~OGRS57DataSource();

  void SetOptionList(char **);
  const char *GetOption(const char *);

  int Open(const char *pszName, int bTestOpen = FALSE,
           CallBackFunction p_callback = NULL);
  int OpenMin(const char *pszName, int bTestOpen = FALSE);
  int Create(const char *pszName, char **papszOptions);

  const char *GetName() { return pszName; }
  int GetLayerCount() { return nLayers; }
  OGRLayer *GetLayer(int);
  void AddLayer(OGRS57Layer *);
  int TestCapability(const char *);

  OGRSpatialReference *GetSpatialRef() { return poSpatialRef; }

  int GetModuleCount() { return nModules; }
  S57Reader *GetModule(int);
  S57Writer *GetWriter() { return poWriter; }

  S57ClassRegistrar *GetS57Registrar() { return poRegistrar; }
  void SetS57Registrar(S57ClassRegistrar *p) { poRegistrar = p; }

  OGRErr GetDSExtent(OGREnvelope *psExtent, int bForce = TRUE);
};

/************************************************************************/
/*                            OGRS57Driver                              */
/************************************************************************/

class OGRS57Driver : public OGRSFDriver {
public:
  ~OGRS57Driver();

  const char *GetName();
  OGRDataSource *Open(const char *, int);
  virtual OGRDataSource *CreateDataSource(const char *pszName, char ** = NULL);
  int TestCapability(const char *);
};

#endif /* ndef _OGR_S57_H_INCLUDED */
