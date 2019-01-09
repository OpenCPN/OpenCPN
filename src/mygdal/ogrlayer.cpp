/******************************************************************************
 *
 * Project:  OpenGIS Simple Features Reference Implementation
 * Purpose:  The generic portions of the OGRSFLayer class.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1999,  Les Technologies SoftMap Inc.
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

#include "ogrsf_frmts.h"
#include "ogr_api.h"
#include "ogr_p.h"
#include "ogr_attrind.h"

/************************************************************************/
/*                              OGRLayer()                              */
/************************************************************************/

OGRLayer::OGRLayer()

{
    m_poStyleTable = NULL;
//    m_poAttrQuery = NULL;
    m_poAttrIndex = NULL;
    m_nRefCount = 0;
}

/************************************************************************/
/*                             ~OGRLayer()                              */
/************************************************************************/

OGRLayer::~OGRLayer()

{
    if( m_poAttrIndex != NULL )
    {
        delete m_poAttrIndex;
        m_poAttrIndex = NULL;
    }

/*
    if( m_poAttrQuery != NULL )
    {
        delete m_poAttrQuery;
        m_poAttrQuery = NULL;
    }
*/
}

/************************************************************************/
/*                             Reference()                              */
/************************************************************************/

int OGRLayer::Reference()

{
    return ++m_nRefCount;
}

/************************************************************************/
/*                          OGR_L_Reference()                           */
/************************************************************************/

int OGR_L_Reference( OGRLayerH hLayer )

{
    return ((OGRLayer *) hLayer)->Reference();
}

/************************************************************************/
/*                            Dereference()                             */
/************************************************************************/

int OGRLayer::Dereference()

{
    return --m_nRefCount;
}

/************************************************************************/
/*                         OGR_L_Dereference()                          */
/************************************************************************/

int OGR_L_Dereference( OGRLayerH hLayer )

{
    return ((OGRLayer *) hLayer)->Dereference();
}

/************************************************************************/
/*                            GetRefCount()                             */
/************************************************************************/

int OGRLayer::GetRefCount() const

{
    return m_nRefCount;
}

/************************************************************************/
/*                         OGR_L_GetRefCount()                          */
/************************************************************************/

int OGR_L_GetRefCount( OGRLayerH hLayer )

{
    return ((OGRLayer *) hLayer)->GetRefCount();
}

/************************************************************************/
/*                          GetFeatureCount()                           */
/************************************************************************/

int OGRLayer::GetFeatureCount( int bForce )

{
    OGRFeature     *poFeature;
    int            nFeatureCount = 0;

    if( !bForce )
        return -1;

    ResetReading();
    while( (poFeature = GetNextFeature()) != NULL )
    {
        nFeatureCount++;
        delete poFeature;
    }
    ResetReading();

    return nFeatureCount;
}

/************************************************************************/
/*                       OGR_L_GetFeatureCount()                        */
/************************************************************************/

int OGR_L_GetFeatureCount( OGRLayerH hLayer, int bForce )

{
    return ((OGRLayer *) hLayer)->GetFeatureCount(bForce);
}

/************************************************************************/
/*                             GetExtent()                              */
/************************************************************************/

OGRErr OGRLayer::GetExtent(OGREnvelope *psExtent, int bForce )

{
    OGRFeature  *poFeature;
    OGREnvelope oEnv;
    GBool       bExtentSet = FALSE;

    if( !bForce )
        return OGRERR_FAILURE;

    ResetReading();
    while( (poFeature = GetNextFeature()) != NULL )
    {
        OGRGeometry *poGeom = poFeature->GetGeometryRef();
        if (poGeom && !bExtentSet)
        {
            poGeom->getEnvelope(psExtent);
            bExtentSet = TRUE;
        }
        else if (poGeom)
        {
            poGeom->getEnvelope(&oEnv);
            if (oEnv.MinX < psExtent->MinX)
                psExtent->MinX = oEnv.MinX;
            if (oEnv.MinY < psExtent->MinY)
                psExtent->MinY = oEnv.MinY;
            if (oEnv.MaxX > psExtent->MaxX)
                psExtent->MaxX = oEnv.MaxX;
            if (oEnv.MaxY > psExtent->MaxY)
                psExtent->MaxY = oEnv.MaxY;
        }
        delete poFeature;
    }
    ResetReading();

    return (bExtentSet ? OGRERR_NONE : OGRERR_FAILURE);
}

/************************************************************************/
/*                          OGR_L_GetExtent()                           */
/************************************************************************/

OGRErr OGR_L_GetExtent( OGRLayerH hLayer, OGREnvelope *psExtent, int bForce )

{
    return ((OGRLayer *) hLayer)->GetExtent( psExtent, bForce );
}

/************************************************************************/
/*                         SetAttributeFilter()                         */
/************************************************************************/

OGRErr OGRLayer::SetAttributeFilter( const char *pszQuery )

{
/* -------------------------------------------------------------------- */
/*      Are we just clearing any existing query?                        */
/* -------------------------------------------------------------------- */
/*    if( pszQuery == NULL || strlen(pszQuery) == 0 )
    {
        if( m_poAttrQuery )
        {
            delete m_poAttrQuery;
            m_poAttrQuery = NULL;
            ResetReading();
        }
        return OGRERR_NONE;
    }
 */
/* -------------------------------------------------------------------- */
/*      Or are we installing a new query?                               */
/* -------------------------------------------------------------------- */
    OGRErr      eErr;

/*
    if( !m_poAttrQuery )
        m_poAttrQuery = new OGRFeatureQuery();

    eErr = m_poAttrQuery->Compile( GetLayerDefn(), pszQuery );
    if( eErr != OGRERR_NONE )
    {
        delete m_poAttrQuery;
        m_poAttrQuery = NULL;
    }
*/

    ResetReading();

	eErr = OGRERR_NONE;
    return eErr;
}

/************************************************************************/
/*                      OGR_L_SetAttributeFilter()                      */
/************************************************************************/

OGRErr OGR_L_SetAttributeFilter( OGRLayerH hLayer, const char *pszQuery )

{
    return ((OGRLayer *) hLayer)->SetAttributeFilter( pszQuery );
}

/************************************************************************/
/*                             GetFeature()                             */
/************************************************************************/

OGRFeature *OGRLayer::GetFeature( long nFID )

{
    OGRFeature *poFeature;

    ResetReading();
    while( (poFeature = GetNextFeature()) != NULL )
    {
        if( poFeature->GetFID() == nFID )
            return poFeature;
        else
            delete poFeature;
    }

    return NULL;
}

/************************************************************************/
/*                          OGR_L_GetFeature()                          */
/************************************************************************/

OGRFeatureH OGR_L_GetFeature( OGRLayerH hLayer, long nFeatureId )

{
    return (OGRFeatureH) ((OGRLayer *)hLayer)->GetFeature( nFeatureId );
}

/************************************************************************/
/*                        OGR_L_GetNextFeature()                        */
/************************************************************************/

OGRFeatureH OGR_L_GetNextFeature( OGRLayerH hLayer )

{
    return (OGRFeatureH) ((OGRLayer *)hLayer)->GetNextFeature();
}

/************************************************************************/
/*                             SetFeature()                             */
/************************************************************************/

OGRErr OGRLayer::SetFeature( OGRFeature * )

{
    return OGRERR_UNSUPPORTED_OPERATION;
}

/************************************************************************/
/*                          OGR_L_SetFeature()                          */
/************************************************************************/

OGRErr OGR_L_SetFeature( OGRLayerH hLayer, OGRFeatureH hFeat )

{
    return ((OGRLayer *)hLayer)->SetFeature( (OGRFeature *) hFeat );
}

/************************************************************************/
/*                           CreateFeature()                            */
/************************************************************************/

OGRErr OGRLayer::CreateFeature( OGRFeature * )

{
    return OGRERR_UNSUPPORTED_OPERATION;
}

/************************************************************************/
/*                        OGR_L_CreateFeature()                         */
/************************************************************************/

OGRErr OGR_L_CreateFeature( OGRLayerH hLayer, OGRFeatureH hFeat )

{
    return ((OGRLayer *) hLayer)->CreateFeature( (OGRFeature *) hFeat );
}

/************************************************************************/
/*                              GetInfo()                               */
/************************************************************************/

const char *OGRLayer::GetInfo( const char * pszTag )

{
    (void) pszTag;
    return NULL;
}

/************************************************************************/
/*                            CreateField()                             */
/************************************************************************/

OGRErr OGRLayer::CreateField( OGRFieldDefn * poField, int bApproxOK )

{
    (void) poField;
    (void) bApproxOK;

    CPLError( CE_Failure, CPLE_NotSupported,
              "CreateField() not supported by this layer.\n" );

    return OGRERR_UNSUPPORTED_OPERATION;
}

/************************************************************************/
/*                         OGR_L_CreateField()                          */
/************************************************************************/

OGRErr OGR_L_CreateField( OGRLayerH hLayer, OGRFieldDefnH hField,
                          int bApproxOK )

{
    return ((OGRLayer *) hLayer)->CreateField( (OGRFieldDefn *) hField,
                                               bApproxOK );
}

/************************************************************************/
/*                          StartTransaction()                          */
/************************************************************************/

OGRErr OGRLayer::StartTransaction()

{
    return OGRERR_NONE;
}

/************************************************************************/
/*                       OGR_L_StartTransaction()                       */
/************************************************************************/

OGRErr OGR_L_StartTransaction( OGRLayerH hLayer )

{
    return ((OGRLayer *)hLayer)->StartTransaction();
}

/************************************************************************/
/*                         CommitTransaction()                          */
/************************************************************************/

OGRErr OGRLayer::CommitTransaction()

{
    return OGRERR_NONE;
}

/************************************************************************/
/*                       OGR_L_CommitTransaction()                      */
/************************************************************************/

OGRErr OGR_L_CommitTransaction( OGRLayerH hLayer )

{
    return ((OGRLayer *)hLayer)->CommitTransaction();
}

/************************************************************************/
/*                        RollbackTransaction()                         */
/************************************************************************/

OGRErr OGRLayer::RollbackTransaction()

{
    return OGRERR_UNSUPPORTED_OPERATION;
}

/************************************************************************/
/*                     OGR_L_RollbackTransaction()                      */
/************************************************************************/

OGRErr OGR_L_RollbackTransaction( OGRLayerH hLayer )

{
    return ((OGRLayer *)hLayer)->RollbackTransaction();
}

/************************************************************************/
/*                         OGR_L_GetLayerDefn()                         */
/************************************************************************/

OGRFeatureDefnH OGR_L_GetLayerDefn( OGRLayerH hLayer )

{
    return (OGRFeatureDefnH) ((OGRLayer *)hLayer)->GetLayerDefn();
}

/************************************************************************/
/*                        OGR_L_GetSpatialRef()                         */
/************************************************************************/

OGRSpatialReferenceH OGR_L_GetSpatialRef( OGRLayerH hLayer )

{
    return (OGRSpatialReferenceH) ((OGRLayer *) hLayer)->GetSpatialRef();
}

/************************************************************************/
/*                        OGR_L_TestCapability()                        */
/************************************************************************/

int OGR_L_TestCapability( OGRLayerH hLayer, const char *pszCap )

{
    return ((OGRLayer *) hLayer)->TestCapability( pszCap );
}

/************************************************************************/
/*                       OGR_L_GetSpatialFilter()                       */
/************************************************************************/

OGRGeometryH OGR_L_GetSpatialFilter( OGRLayerH hLayer )

{
    return (OGRGeometryH) ((OGRLayer *) hLayer)->GetSpatialFilter();
}

/************************************************************************/
/*                       OGR_L_SetSpatialFilter()                       */
/************************************************************************/

void OGR_L_SetSpatialFilter( OGRLayerH hLayer, OGRGeometryH hGeom )

{
    ((OGRLayer *) hLayer)->SetSpatialFilter( (OGRGeometry *) hGeom );
}

/************************************************************************/
/*                         OGR_L_ResetReading()                         */
/************************************************************************/

void OGR_L_ResetReading( OGRLayerH hLayer )

{
    ((OGRLayer *) hLayer)->ResetReading();
}

/************************************************************************/
/*                       InitializeIndexSupport()                       */
/*                                                                      */
/*      This is only intended to be called by driver layer              */
/*      implementations but we don't make it protected so that the      */
/*      datasources can do it too if that is more appropriate.          */
/************************************************************************/

OGRErr OGRLayer::InitializeIndexSupport( const char *pszFilename )

{
    OGRErr eErr;

//    m_poAttrIndex = OGRCreateDefaultLayerIndex();

    eErr = m_poAttrIndex->Initialize( pszFilename, this );
    if( eErr != OGRERR_NONE )
    {
        delete m_poAttrIndex;
        m_poAttrIndex = NULL;
    }

    return eErr;
}

/************************************************************************/
/*                             SyncToDisk()                             */
/************************************************************************/

OGRErr OGRLayer::SyncToDisk()

{
    return OGRERR_NONE;
}

/************************************************************************/
/*                          OGR_L_SyncToDisk()                          */
/************************************************************************/

OGRErr OGR_L_SyncToDisk( OGRLayerH hDS )

{
    return ((OGRLayer *) hDS)->SyncToDisk();
}

/************************************************************************/
/*                           DeleteFeature()                            */
/************************************************************************/

OGRErr OGRLayer::DeleteFeature( long nFID )

{
    return OGRERR_UNSUPPORTED_OPERATION;
}

/************************************************************************/
/*                        OGR_L_DeleteFeature()                         */
/************************************************************************/

OGRErr OGR_L_DeleteFeature( OGRLayerH hDS, long nFID )

{
    return ((OGRLayer *) hDS)->DeleteFeature( nFID );
}

