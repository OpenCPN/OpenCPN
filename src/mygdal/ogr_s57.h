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
 * $Log: ogr_s57.h,v $
 * Revision 1.4  2009/09/25 15:22:05  bdbcat
 * Improve SENC creation progress dialog
 *
 * Revision 1.3  2008/08/27 22:51:38  bdbcat
 * Add error returns to ENC update logic
 *
 * Revision 1.2  2008/03/30 23:05:02  bdbcat
 * Cleanup
 *
 * Revision 1.1.1.1  2006/08/21 05:52:20  dsr
 * Initial import as opencpn, GNU Automake compliant.
 *
 * Revision 1.1.1.1  2006/04/19 03:23:28  dsr
 * Rename/Import to OpenCPN
 *
 * Revision 1.8  2003/11/15 21:50:52  warmerda
 * Added limited creation support
 *
 * Revision 1.7  2003/09/05 19:12:05  warmerda
 * added RETURN_PRIMITIVES support to get low level prims
 *
 * Revision 1.6  2002/03/05 14:25:43  warmerda
 * expanded tabs
 *
 * Revision 1.5  2001/12/17 22:34:16  warmerda
 * added GetFeature() method on OGRS57Layer
 *
 * Revision 1.4  2001/12/14 19:40:18  warmerda
 * added optimized feature counting, and extents collection
 *
 * Revision 1.3  1999/11/18 19:01:25  warmerda
 * expanded tabs
 *
 * Revision 1.2  1999/11/08 22:23:00  warmerda
 * added object class support
 *
 * Revision 1.1  1999/11/03 22:12:43  warmerda
 * New
 *
 */

#ifndef _OGR_S57_H_INCLUDED
#define _OGR_S57_H_INCLUDED

#include "ogrsf_frmts.h"
#include "s57.h"


//    Misc error return codes
#define BAD_FILE        10
#define BAD_HEADER      11
#define BAD_OPEN        12
#define BAD_UPDATE      30

class OGRS57DataSource;

/************************************************************************/
/*                             OGRS57Layer                              */
/*                                                                      */
/*      Represents all features of a particular S57 object class.       */
/************************************************************************/

class OGRS57Layer : public OGRLayer
{
    OGRGeometry        *poFilterGeom;

    OGRS57DataSource   *poDS;

    OGRFeatureDefn     *poFeatureDefn;

    int                 nCurrentModule;
    int                 nRCNM;
    int                 nOBJL;
    int                 nNextFEIndex;
    int                 nFeatureCount;

  public:
                        OGRS57Layer( OGRS57DataSource * poDS,
                                     OGRFeatureDefn *, int nFeatureCount = -1,
                                     int nOBJL = -1 );
    virtual             ~OGRS57Layer();

    OGRGeometry *       GetSpatialFilter() { return poFilterGeom; }
    void                SetSpatialFilter( OGRGeometry * );

    void                ResetReading();
    OGRFeature *        GetNextFeature();
    OGRFeature *        GetNextUnfilteredFeature();
    virtual OGRFeature *GetFeature( long nFeatureId );

    virtual int         GetFeatureCount( int bForce = TRUE );
    virtual OGRErr      GetExtent(OGREnvelope *psExtent, int bForce = TRUE);

    OGRFeatureDefn *    GetLayerDefn() { return poFeatureDefn; }

    virtual OGRErr      CreateFeature( OGRFeature *poFeature );
    int                 TestCapability( const char * );

    virtual OGRSpatialReference *GetSpatialRef();
};

/************************************************************************/
/*                          OGRS57DataSource                            */
/************************************************************************/
typedef bool (*CallBackFunction)(void);

class OGRS57DataSource
{
    char                *pszName;

    int                 nLayers;
    OGRS57Layer         **papoLayers;

    OGRSpatialReference *poSpatialRef;

    char                **papszOptions;

    int                 nModules;
    S57Reader           **papoModules;

    S57Writer           *poWriter;

    static S57ClassRegistrar *poRegistrar;

    int                 bClassCountSet;
    int                 anClassCount[MAX_CLASSES];

    int                 bExtentsSet;
    OGREnvelope         oExtents;

  public:
                        OGRS57DataSource();
                        ~OGRS57DataSource();

    void                SetOptionList( char ** );
    const char         *GetOption( const char * );

    int                 Open( const char * pszName, int bTestOpen = FALSE, CallBackFunction p_callback = NULL );
    int                 OpenMin( const char * pszName, int bTestOpen = FALSE );
    int                 Create( const char *pszName, char **papszOptions );

    const char          *GetName() { return pszName; }
    int                 GetLayerCount() { return nLayers; }
    OGRLayer            *GetLayer( int );
    void                AddLayer( OGRS57Layer * );
    int                 TestCapability( const char * );

    OGRSpatialReference *GetSpatialRef() { return poSpatialRef; }

    int                 GetModuleCount() { return nModules; }
    S57Reader          *GetModule( int );
    S57Writer          *GetWriter() { return poWriter; }

    S57ClassRegistrar  *GetS57Registrar() { return poRegistrar; }
    void                SetS57Registrar(S57ClassRegistrar *p) { poRegistrar = p; }

    OGRErr      GetDSExtent(OGREnvelope *psExtent, int bForce = TRUE);
};

/************************************************************************/
/*                            OGRS57Driver                              */
/************************************************************************/

class OGRS57Driver : public OGRSFDriver
{
  public:
                ~OGRS57Driver();

    const char *GetName();
    OGRDataSource *Open( const char *, int );
    virtual OGRDataSource *CreateDataSource( const char *pszName,
                                             char ** = NULL );
    int                 TestCapability( const char * );
};

#endif /* ndef _OGR_S57_H_INCLUDED */
