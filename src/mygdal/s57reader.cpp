/******************************************************************************
 *
 * Project:  S-57 Translator
 * Purpose:  Implements S57Reader class.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1999, 2001, Frank Warmerdam
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
 * *
 */

#include "s57.h"
#include "ogr_api.h"
#include "cpl_conv.h"
#include "cpl_string.h"
#include "ogr_s57.h"

/************************************************************************/
/*                             S57Reader()                              */
/************************************************************************/

S57Reader::S57Reader( const char * pszFilename )

{
    pszModuleName = CPLStrdup( pszFilename );
    pszDSNM = NULL;

    poModule = NULL;

    nFDefnCount = 0;
    papoFDefnList = NULL;

    nCOMF = 1000000;
    nSOMF = 10;

    poRegistrar = NULL;
    bFileIngested = FALSE;

    nNextFEIndex = 0;
    nNextVIIndex = 0;
    nNextVCIndex = 0;
    nNextVEIndex = 0;
    nNextVFIndex = 0;

    iPointOffset = 0;
    poMultiPoint = NULL;

    papszOptions = NULL;

    nOptionFlags = S57M_UPDATES;

    bMissingWarningIssued = FALSE;
    bAttrWarningIssued = FALSE;
    
    Nall = 0;
    Aall = 0;
}

/************************************************************************/
/*                             ~S57Reader()                             */
/************************************************************************/

S57Reader::~S57Reader()

{
    Close();

    CPLFree( pszModuleName );
    CSLDestroy( papszOptions );

    CPLFree( papoFDefnList );
}

/************************************************************************/
/*                                Open()                                */
/************************************************************************/

int S57Reader::Open( int bTestOpen )

{
    if( poModule != NULL )
    {
        Rewind();
        return TRUE;
    }

    poModule = new DDFModule();
    if( !poModule->Open( pszModuleName ) )
    {
        // notdef: test bTestOpen.
        delete poModule;
        poModule = NULL;
        return FALSE;
    }

    // note that the following won't work for catalogs.
    if( poModule->FindFieldDefn("DSID") == NULL )
    {
        if( !bTestOpen )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "%s is an ISO8211 file, but not an S-57 data file.\n",
                      pszModuleName );
        }
        delete poModule;
        poModule = NULL;
        return FALSE;
    }

    // Make sure the FSPT field is marked as repeating.
    DDFFieldDefn *poFSPT = poModule->FindFieldDefn( "FSPT" );
    if( poFSPT != NULL && !poFSPT->IsRepeating() )
    {
        CPLDebug( "S57", "Forcing FSPT field to be repeating." );
        poFSPT->SetRepeatingFlag( TRUE );
    }

    nNextFEIndex = 0;
    nNextVIIndex = 0;
    nNextVCIndex = 0;
    nNextVEIndex = 0;
    nNextVFIndex = 0;

    return TRUE;
}

/************************************************************************/
/*                               Close()                                */
/************************************************************************/

void S57Reader::Close()

{
    if( poModule != NULL )
    {
        oVI_Index.Clear();
        oVC_Index.Clear();
        oVE_Index.Clear();
        oVF_Index.Clear();
        oFE_Index.Clear();

        ClearPendingMultiPoint();

        delete poModule;
        poModule = NULL;

        bFileIngested = FALSE;

        CPLFreeConfig();

        CPLFree( pszDSNM );
        pszDSNM = NULL;
    }
}

/************************************************************************/
/*                       ClearPendingMultiPoint()                       */
/************************************************************************/

void S57Reader::ClearPendingMultiPoint()

{
    if( poMultiPoint != NULL )
    {
        delete poMultiPoint;
        poMultiPoint = NULL;
    }
}

/************************************************************************/
/*                       NextPendingMultiPoint()                        */
/************************************************************************/

OGRFeature *S57Reader::NextPendingMultiPoint()

{
    CPLAssert( poMultiPoint != NULL );
    CPLAssert( wkbFlatten(poMultiPoint->GetGeometryRef()->getGeometryType())
                                                        == wkbMultiPoint );

    OGRFeatureDefn *poDefn = poMultiPoint->GetDefnRef();
    OGRFeature  *poPoint = new OGRFeature( poDefn );
    OGRMultiPoint *poMPGeom = (OGRMultiPoint *) poMultiPoint->GetGeometryRef();
    OGRPoint    *poSrcPoint;

    poPoint->SetFID( poMultiPoint->GetFID() );

    for( int i = 0; i < poDefn->GetFieldCount(); i++ )
    {
        poPoint->SetField( i, poMultiPoint->GetRawFieldRef(i) );
    }

    poSrcPoint = (OGRPoint *) poMPGeom->getGeometryRef( iPointOffset++ );
    poPoint->SetGeometry( poSrcPoint );

    poPoint->SetField( "DEPTH", poSrcPoint->getZ() );

    if( iPointOffset >= poMPGeom->getNumGeometries() )
        ClearPendingMultiPoint();

    return poPoint;
}

/************************************************************************/
/*                             SetOptions()                             */
/************************************************************************/

void S57Reader::SetOptions( char ** papszOptionsIn )

{
    const char * pszOptionValue;

    CSLDestroy( papszOptions );
    papszOptions = CSLDuplicate( papszOptionsIn );

    pszOptionValue = CSLFetchNameValue( papszOptions, S57O_SPLIT_MULTIPOINT );
    if( pszOptionValue != NULL && !EQUAL(pszOptionValue,"OFF") )
        nOptionFlags |= S57M_SPLIT_MULTIPOINT;
    else
        nOptionFlags &= ~S57M_SPLIT_MULTIPOINT;

    pszOptionValue = CSLFetchNameValue( papszOptions, S57O_ADD_SOUNDG_DEPTH );
    if( pszOptionValue != NULL && !EQUAL(pszOptionValue,"OFF") )
        nOptionFlags |= S57M_ADD_SOUNDG_DEPTH;
    else
        nOptionFlags &= ~S57M_ADD_SOUNDG_DEPTH;

    CPLAssert( ! (nOptionFlags & S57M_ADD_SOUNDG_DEPTH)
               || (nOptionFlags & S57M_SPLIT_MULTIPOINT) );

    pszOptionValue = CSLFetchNameValue( papszOptions, S57O_LNAM_REFS );
    if( pszOptionValue != NULL && !EQUAL(pszOptionValue,"OFF") )
        nOptionFlags |= S57M_LNAM_REFS;
    else
        nOptionFlags &= ~S57M_LNAM_REFS;

    pszOptionValue = CSLFetchNameValue( papszOptions, S57O_UPDATES );
    if( pszOptionValue != NULL && !EQUAL(pszOptionValue,"OFF") )
        nOptionFlags |= S57M_UPDATES;
    else
        nOptionFlags &= ~S57M_UPDATES;

    pszOptionValue = CSLFetchNameValue(papszOptions,
                                       S57O_PRESERVE_EMPTY_NUMBERS);
    if( pszOptionValue != NULL && !EQUAL(pszOptionValue,"OFF") )
        nOptionFlags |= S57M_PRESERVE_EMPTY_NUMBERS;
    else
        nOptionFlags &= ~S57M_PRESERVE_EMPTY_NUMBERS;

    pszOptionValue = CSLFetchNameValue( papszOptions, S57O_RETURN_PRIMITIVES );
    if( pszOptionValue != NULL && !EQUAL(pszOptionValue,"OFF") )
        nOptionFlags |= S57M_RETURN_PRIMITIVES;
    else
        nOptionFlags &= ~S57M_RETURN_PRIMITIVES;

    pszOptionValue = CSLFetchNameValue( papszOptions, S57O_RETURN_LINKAGES );
    if( pszOptionValue != NULL && !EQUAL(pszOptionValue,"OFF") )
        nOptionFlags |= S57M_RETURN_LINKAGES;
    else
        nOptionFlags &= ~S57M_RETURN_LINKAGES;
}

/************************************************************************/
/*                           SetClassBased()                            */
/************************************************************************/

void S57Reader::SetClassBased( S57ClassRegistrar * poReg )

{
    poRegistrar = poReg;
}

/************************************************************************/
/*                               Rewind()                               */
/************************************************************************/

void S57Reader::Rewind()

{
    ClearPendingMultiPoint();
    nNextFEIndex = 0;
    nNextVIIndex = 0;
    nNextVCIndex = 0;
    nNextVEIndex = 0;
    nNextVFIndex = 0;
}

/************************************************************************/
/*                               Ingest()                               */
/*                                                                      */
/*      Read all the records into memory, adding to the appropriate     */
/*      indexes.                                                        */
/************************************************************************/

int S57Reader::Ingest(CallBackFunction pcallback)
{
    DDFRecord   *poRecord;

    CPLSetConfigOption( "CPL_DEBUG", "S57" );

    if( poModule == NULL || bFileIngested )
        return 0;

/* -------------------------------------------------------------------- */
/*      Read all the records in the module, and place them in           */
/*      appropriate indexes.                                            */
/* -------------------------------------------------------------------- */
    while( (poRecord = poModule->ReadRecord()) != NULL )
    {
          if(pcallback)
          {
            if (!(*pcallback)())
              return 0;
          }

        DDFField        *poKeyField = poRecord->GetField(1);
        const char *pszname = poKeyField->GetFieldDefn()->GetName();

        if( EQUAL(pszname,"VRID") )
        {
#if 0
            int         nRCNM = poRecord->GetIntSubfield( "VRID",0, "RCNM",0);
            int         nRCID = poRecord->GetIntSubfield( "VRID",0, "RCID",0);
#else
            int nRCNM = 0, nRCID = 0;
            DDFField *poField = poRecord->FindField( "VRID", 0 );
            if( poField ) {
                int         nBytesRemaining;
                DDFSubfieldDefn     *poSFDefn;
                poSFDefn = poField->GetFieldDefn()->FindSubfieldDefn( "RCNM" );
                if( poSFDefn ) {
                    const char *pachData = poField->GetSubfieldData(poSFDefn, &nBytesRemaining, 0);
                    nRCNM = poSFDefn->ExtractIntData( pachData, nBytesRemaining, NULL );
                }

                poSFDefn = poField->GetFieldDefn()->FindSubfieldDefn( "RCID" );
                if( poSFDefn ) {
                    const char *pachData = poField->GetSubfieldData(poSFDefn, &nBytesRemaining, 0);
                    nRCID = poSFDefn->ExtractIntData( pachData, nBytesRemaining, NULL );
                }
            }
#endif

            switch( nRCNM )
            {
              case RCNM_VI:
                oVI_Index.AddRecord( nRCID, poRecord->Copy() );
                break;

              case RCNM_VC:
                oVC_Index.AddRecord( nRCID, poRecord->Copy() );
                break;

              case RCNM_VE:
                oVE_Index.AddRecord( nRCID, poRecord->Copy() );
                break;

              case RCNM_VF:
                oVF_Index.AddRecord( nRCID, poRecord->Copy() );
                break;

              default:
                CPLAssert( FALSE );
                break;
            }

        }

        //  Feature records
        else if( EQUAL(pszname,"FRID") )
        {

//              poRecord->Dump(stderr);               //  for debugging, try ./opencpn &>test.dbg

            int         nRCID = poRecord->GetIntSubfield( "FRID",0, "RCID",0);
            oFE_Index.AddRecord( nRCID, poRecord->Copy() );

        }


        //  Convenience values
        else if( EQUAL(pszname,"DSPM") )
        {
            nCOMF = MAX(1,poRecord->GetIntSubfield( "DSPM",0, "COMF",0));
            nSOMF = MAX(1,poRecord->GetIntSubfield( "DSPM",0, "SOMF",0));
            nCSCL = MAX(1,poRecord->GetIntSubfield( "DSPM",0, "CSCL",0));

        }

        else if( EQUAL(pszname,"DSID") )
        {
            CPLFree( pszDSNM );
            pszDSNM = CPLStrdup(poRecord->GetStringSubfield( "DSID", 0, "DSNM", 0 ));
            Nall = poRecord->GetIntSubfield( "DSSI", 0, "NALL", 0 );
            Aall = poRecord->GetIntSubfield( "DSSI", 0, "AALL", 0 );
        }

        else
        {
            CPLDebug( "S57",
                      "Skipping %s record in S57Reader::Ingest().\n",
                      poKeyField->GetFieldDefn()->GetName() );
        }

    }

    bFileIngested = TRUE;


/* -------------------------------------------------------------------- */
/*      If update support is enabled, read and apply them.              */
/* -------------------------------------------------------------------- */
    int update_return = 0;
    if( nOptionFlags & S57M_UPDATES )
        update_return = FindAndApplyUpdates();

    return update_return;
}

/************************************************************************/
/*                           SetNextFEIndex()                           */
/************************************************************************/

void S57Reader::SetNextFEIndex( int nNewIndex, int nRCNM )

{
    if( nRCNM == RCNM_VI )
        nNextVIIndex = nNewIndex;
    else if( nRCNM == RCNM_VC )
        nNextVCIndex = nNewIndex;
    else if( nRCNM == RCNM_VE )
        nNextVEIndex = nNewIndex;
    else if( nRCNM == RCNM_VF )
        nNextVFIndex = nNewIndex;
    else
    {
        if( nNextFEIndex != nNewIndex )
            ClearPendingMultiPoint();

        nNextFEIndex = nNewIndex;
    }
}

/************************************************************************/
/*                           GetNextFEIndex()                           */
/************************************************************************/

int S57Reader::GetNextFEIndex( int nRCNM )

{
    if( nRCNM == RCNM_VI )
        return nNextVIIndex;
    else if( nRCNM == RCNM_VC )
        return nNextVCIndex;
    else if( nRCNM == RCNM_VE )
        return nNextVEIndex;
    else if( nRCNM == RCNM_VF )
        return nNextVFIndex;
    else
        return nNextFEIndex;
}

/************************************************************************/
/*                          ReadNextFeature()                           */
/************************************************************************/

OGRFeature * S57Reader::ReadNextFeature( OGRFeatureDefn * poTarget )

{
    if( !bFileIngested )
        Ingest();

/* -------------------------------------------------------------------- */
/*      Special case for "in progress" multipoints being split up.      */
/* -------------------------------------------------------------------- */
    if( poMultiPoint != NULL )
    {
        if( poTarget == NULL || poTarget == poMultiPoint->GetDefnRef() )
        {
            return NextPendingMultiPoint();
        }
        else
        {
            ClearPendingMultiPoint();
        }
    }

/* -------------------------------------------------------------------- */
/*      Next vector feature?                                            */
/* -------------------------------------------------------------------- */
    if( nOptionFlags & S57M_RETURN_PRIMITIVES )
    {
        int nRCNM = 0;
        int *pnCounter = NULL;

        if( poTarget == NULL )
        {
            if( nNextVIIndex < oVI_Index.GetCount() )
            {
                nRCNM = RCNM_VI;
                pnCounter = &nNextVIIndex;
            }
            else if( nNextVCIndex < oVC_Index.GetCount() )
            {
                nRCNM = RCNM_VC;
                pnCounter = &nNextVCIndex;
            }
            else if( nNextVEIndex < oVE_Index.GetCount() )
            {
                nRCNM = RCNM_VE;
                pnCounter = &nNextVEIndex;
            }
            else if( nNextVFIndex < oVF_Index.GetCount() )
            {
                nRCNM = RCNM_VF;
                pnCounter = &nNextVFIndex;
            }
        }
        else
        {
            if( EQUAL(poTarget->GetName(),OGRN_VI) )
            {
                nRCNM = RCNM_VI;
                pnCounter = &nNextVIIndex;
            }
            else if( EQUAL(poTarget->GetName(),OGRN_VC) )
            {
                nRCNM = RCNM_VC;
                pnCounter = &nNextVCIndex;
            }
            else if( EQUAL(poTarget->GetName(),OGRN_VE) )
            {
                nRCNM = RCNM_VE;
                pnCounter = &nNextVEIndex;
            }
            else if( EQUAL(poTarget->GetName(),OGRN_VF) )
            {
                nRCNM = RCNM_VF;
                pnCounter = &nNextVFIndex;
            }
        }

        if( nRCNM != 0 )
        {
            OGRFeature *poFeature = ReadVector( *pnCounter, nRCNM );
            if( poFeature != NULL )
            {
                *pnCounter += 1;
                return poFeature;
            }
        }
    }

/* -------------------------------------------------------------------- */
/*      Next feature.                                                   */
/* -------------------------------------------------------------------- */
    while( nNextFEIndex < oFE_Index.GetCount() )
    {
        OGRFeature      *poFeature;

        poFeature = ReadFeature( nNextFEIndex++, poTarget );
        if( poFeature != NULL )
        {
            if( (nOptionFlags & S57M_SPLIT_MULTIPOINT)
                && poFeature->GetGeometryRef() != NULL
                && wkbFlatten(poFeature->GetGeometryRef()->getGeometryType())
                                                        == wkbMultiPoint)
            {
                poMultiPoint = poFeature;
                iPointOffset = 0;
                return NextPendingMultiPoint();
            }

            return poFeature;
        }
    }

    return NULL;
}


/************************************************************************/
/*                            ReadFeature()                             */
/*                                                                      */
/*      Read the features who's id is provided.                         */
/************************************************************************/
/*
OGRFeature *S57Reader::ReadFeature( int nFeatureId, OGRFeatureDefn *poTarget )

{
    OGRFeature  *poFeature;

    if( nFeatureId < 0 || nFeatureId >= oFE_Index.GetCount() )
        return NULL;


    poFeature = AssembleFeature( oFE_Index.GetByIndex(nFeatureId),
                                 poTarget );
    if( poFeature != NULL )
        poFeature->SetFID( nFeatureId );

    return poFeature;
}

*/
/************************************************************************/
/*                            ReadFeature()                             */
/*                                                                      */
/*      Read the features who's id is provided.                         */
/************************************************************************/

OGRFeature *S57Reader::ReadFeature( int nFeatureId, OGRFeatureDefn *poTarget )

{
    OGRFeature  *poFeature;

    if( nFeatureId < 0 || nFeatureId >= oFE_Index.GetCount() )
        return NULL;

    DDFRecord *poRecord = oFE_Index.GetByIndex(nFeatureId);

    if(poTarget)
    {
            int nRecord_OBJL = poRecord->GetIntSubfield( "FRID", 0, "OBJL", 0 );
            int nOBJL = poTarget->GetOBJL();

            if(nRecord_OBJL == nOBJL)              // tentative match
            {
                  poFeature = AssembleFeature( oFE_Index.GetByIndex(nFeatureId),
                                 poTarget );

                  if( poFeature != NULL )
                        poFeature->SetFID( nFeatureId );

                  return poFeature;
            }
            else
                  return NULL;
    }
    else
    {
            poFeature = AssembleFeature( oFE_Index.GetByIndex(nFeatureId),
                                 poTarget );
            if( poFeature != NULL )
                  poFeature->SetFID( nFeatureId );

            return poFeature;
    }

}


/************************************************************************/
/*                          AssembleFeature()                           */
/*                                                                      */
/*      Assemble an OGR feature based on a feature record.              */
/************************************************************************/

OGRFeature *S57Reader::AssembleFeature( DDFRecord * poRecord,
                                        OGRFeatureDefn * poTarget )

{
    int         nPRIM, nOBJL;
    OGRFeatureDefn *poFDefn;

/* -------------------------------------------------------------------- */
/*      Find the feature definition to use.  Currently this is based    */
/*      on the primitive, but eventually this should be based on the    */
/*      object class (FRID.OBJL) in some cases, and the primitive in    */
/*      others.                                                         */
/* -------------------------------------------------------------------- */
    poFDefn = FindFDefn( poRecord );
    if( poFDefn == NULL )
        return NULL;

/* -------------------------------------------------------------------- */
/*      Does this match our target feature definition?  If not skip     */
/*      this feature.                                                   */
/* -------------------------------------------------------------------- */
    if( poTarget != NULL && poFDefn != poTarget )
        return NULL;

/* -------------------------------------------------------------------- */
/*      Create the new feature object.                                  */
/* -------------------------------------------------------------------- */
    OGRFeature          *poFeature;

    poFeature = new OGRFeature( poFDefn );

/* -------------------------------------------------------------------- */
/*      Assign a few standard feature attribues.                        */
/* -------------------------------------------------------------------- */
    nOBJL = poRecord->GetIntSubfield( "FRID", 0, "OBJL", 0 );
    poFeature->SetField( "OBJL", nOBJL );

    poFeature->SetField( "RCID",
                         poRecord->GetIntSubfield( "FRID", 0, "RCID", 0 ));
    poFeature->SetField( "PRIM",
                         poRecord->GetIntSubfield( "FRID", 0, "PRIM", 0 ));
    poFeature->SetField( "GRUP",
                         poRecord->GetIntSubfield( "FRID", 0, "GRUP", 0 ));
    poFeature->SetField( "RVER",
                         poRecord->GetIntSubfield( "FRID", 0, "RVER", 0 ));
    poFeature->SetField( "AGEN",
                         poRecord->GetIntSubfield( "FOID", 0, "AGEN", 0 ));
    poFeature->SetField( "FIDN",
                         poRecord->GetIntSubfield( "FOID", 0, "FIDN", 0 ));
    poFeature->SetField( "FIDS",
                         poRecord->GetIntSubfield( "FOID", 0, "FIDS", 0 ));

/* -------------------------------------------------------------------- */
/*      Generate long name, if requested.                               */
/* -------------------------------------------------------------------- */
    if( nOptionFlags & S57M_LNAM_REFS )
    {
        GenerateLNAMAndRefs( poRecord, poFeature );
    }

/* -------------------------------------------------------------------- */
/*      Generate primitive references if requested.                     */
/* -------------------------------------------------------------------- */
    if( nOptionFlags & S57M_RETURN_LINKAGES )
        GenerateFSPTAttributes( poRecord, poFeature );

/* -------------------------------------------------------------------- */
/*      Apply object class specific attributes, if supported.           */
/* -------------------------------------------------------------------- */
    if( poRegistrar != NULL )
        ApplyObjectClassAttributes( poRecord, poFeature );

/* -------------------------------------------------------------------- */
/*      Find and assign spatial component.                              */
/* -------------------------------------------------------------------- */
    nPRIM = poRecord->GetIntSubfield( "FRID", 0, "PRIM", 0 );

    if( nPRIM == PRIM_P )
    {
        if( nOBJL == 129 ) /* SOUNDG */
            AssembleSoundingGeometry( poRecord, poFeature );
        else
            AssemblePointGeometry( poRecord, poFeature );
    }
    else if( nPRIM == PRIM_L )
    {
        AssembleLineGeometry( poRecord, poFeature );
    }
    else if( nPRIM == PRIM_A )
    {
        AssembleAreaGeometry( poRecord, poFeature );
    }

    return poFeature;
}

/************************************************************************/
/*                     ApplyObjectClassAttributes()                     */
/************************************************************************/

void S57Reader::ApplyObjectClassAttributes( DDFRecord * poRecord,
                                            OGRFeature * poFeature )

{
/* -------------------------------------------------------------------- */
/*      ATTF Attributes                                                 */
/* -------------------------------------------------------------------- */
    DDFField    *poATTF = poRecord->FindField( "ATTF" );
    int         nAttrCount, iAttr;

    if( poATTF == NULL )
        return;

    DDFFieldDefn *poDefn = poATTF->GetFieldDefn();
    
    nAttrCount = poATTF->GetRepeatCount();
    for( iAttr = 0; iAttr < nAttrCount; iAttr++ )
    {
        int     nAttrId = poRecord->GetIntSubfield("ATTF",0,"ATTL",iAttr);
        const char *pszAcronym;

        if( nAttrId < 1 || nAttrId > poRegistrar->GetMaxAttrIndex()
            || (pszAcronym = poRegistrar->GetAttrAcronym(nAttrId)) == NULL )
        {
            if( !bAttrWarningIssued )
            {
                bAttrWarningIssued = TRUE;
                CPLError( CE_Warning, CPLE_AppDefined,
                        "Illegal feature attribute id (ATTF:ATTL[%d]) of %d\n"
                        "on feature FIDN=%d, FIDS=%d.\n"
                        "Skipping attribute, no more warnings will be issued.",
                          iAttr, nAttrId,
                          poFeature->GetFieldAsInteger( "FIDN" ),
                          poFeature->GetFieldAsInteger( "FIDS" ) );
            }

            continue;
        }

        /* Fetch the attribute value */
        const char *pszValue;
        pszValue = poRecord->GetStringSubfield("ATTF",0,"ATVL",iAttr);

        /* Apply to feature in an appropriate way */
        int iField;
        OGRFieldDefn *poFldDefn;

        iField = poFeature->GetDefnRef()->GetFieldIndex(pszAcronym);
        if( iField < 0 )
        {
            if( !bMissingWarningIssued )
            {
                bMissingWarningIssued = TRUE;
                CPLError( CE_Warning, CPLE_AppDefined,
                          "For feature \"%s\", attribute \"%s\" ignored, not in expected schema.\n",
                          poFeature->GetDefnRef()->GetName(), pszAcronym );
            }
            continue;
        }

        // Handle deleted attributes
        // If the first char of the attribute is 0x7f, then unset this field.
        // Any later requests for the attribute value will retrun an empty string.
        if(pszValue[0] == 0x7f)
        {
            poFeature->UnsetField( iField );
            continue;
        }
        
        poFldDefn = poFeature->GetDefnRef()->GetFieldDefn( iField );
        if( poFldDefn->GetType() == OFTInteger
            || poFldDefn->GetType() == OFTReal )
        {
            if( strlen(pszValue) == 0 )
            {
                if( nOptionFlags & S57M_PRESERVE_EMPTY_NUMBERS )
                    poFeature->SetField( iField, EMPTY_NUMBER_MARKER );
                else
                    /* leave as null if value was empty string */;
            }
            else
                poFeature->SetField( iField, pszValue );
        }
        else
            poFeature->SetField( iField, pszValue );
    }

/* -------------------------------------------------------------------- */
/*      NATF (national) attributes                                      */
/* -------------------------------------------------------------------- */
    DDFField    *poNATF = poRecord->FindField( "NATF" );

    if( poNATF == NULL )
        return;

    nAttrCount = poNATF->GetRepeatCount();
    for( iAttr = 0; iAttr < nAttrCount; iAttr++ )
    {
        int     nAttrId = poRecord->GetIntSubfield("NATF",0,"ATTL",iAttr);
        const char *pszAcronym;

        if( nAttrId < 1 || nAttrId >= poRegistrar->GetMaxAttrIndex()
            || (pszAcronym = poRegistrar->GetAttrAcronym(nAttrId)) == NULL )
        {
//            poRecord->Dump(stdout);
//            int     xnAttrId = poRecord->GetIntSubfield("NATF",0,"ATTL",iAttr);
            static int bAttrWarningIssued = FALSE;

            if( !bAttrWarningIssued )
            {
                bAttrWarningIssued = TRUE;
                CPLError( CE_Warning, CPLE_AppDefined,
                        "Illegal feature attribute id (NATF:ATTL[%d]) of %d\n"
                        "on feature FIDN=%d, FIDS=%d.\n"
                        "Skipping attribute, no more warnings will be issued.",
                          iAttr, nAttrId,
                          poFeature->GetFieldAsInteger( "FIDN" ),
                          poFeature->GetFieldAsInteger( "FIDS" ) );
            }

            continue;
        }

        const char *pszValue = poRecord->GetStringSubfield("NATF",0,"ATVL",iAttr);
        if( pszValue != NULL )
        {
            
        //      If National Language strings are encoded as UCS-2 (a.k.a UTF-16)
        //      then we capture and duplicate the attribute string directly,
        //      in order to avoid truncation that would happen if it were considered a simple char *
        
            if(Nall==2) { //national string encoded in UCS-2, determined from DSID record

        //      Compute the data size
                DDFField    *poField;
                int nLength = 0;
                poField = poRecord->FindField( "NATF", 0 );
                if( poField ) {
                    DDFSubfieldDefn     *poSFDefn;
            
                    poSFDefn = poField->GetFieldDefn()->FindSubfieldDefn( "ATVL" );
                    if( poSFDefn ) {
                        int max_length = 0;
                        const char *pachData = poField->GetSubfieldData(poSFDefn, &max_length, iAttr);
                        nLength = poSFDefn->GetDataLength( pachData, max_length, NULL);
                    }
                }
 
                if( nLength ) {
                    //  Make the new length a multiple of 2, so that
                    //  later stages will count chars correctly
                    //  Also, be sure that the string ends with 00 00
                    int new_len = ((nLength / 2) + 2)*2;
                    char *aa = (char *)calloc(new_len, 1);
                    memcpy(aa, pszValue, nLength);
                    
                    int index = poFeature->GetFieldIndex(pszAcronym);
                    OGRField *field = poFeature->GetRawFieldRef( index );
                    field->String =  aa;
                }
            }
            else {      //  encoded as ISO8859_1, pass it along
                poFeature->SetField(pszAcronym,pszValue);
            }
        }
                
    }
}

/************************************************************************/
/*                        generatelnamandrefs()                         */
/************************************************************************/

void S57Reader::GenerateLNAMAndRefs( DDFRecord * poRecord,
                                     OGRFeature * poFeature )

{
    char        szLNAM[32];

/* -------------------------------------------------------------------- */
/*      Apply the LNAM to the object.                                   */
/* -------------------------------------------------------------------- */
    sprintf( szLNAM, "%04X%08X%04X",
             poFeature->GetFieldAsInteger( "AGEN" ),
             poFeature->GetFieldAsInteger( "FIDN" ),
             poFeature->GetFieldAsInteger( "FIDS" ) );
    poFeature->SetField( "LNAM", szLNAM );

/* -------------------------------------------------------------------- */
/*      Do we have references to other features.                        */
/* -------------------------------------------------------------------- */
    DDFField    *poFFPT;

    poFFPT = poRecord->FindField( "FFPT" );

    if( poFFPT == NULL )
        return;

/* -------------------------------------------------------------------- */
/*      Apply references.                                               */
/* -------------------------------------------------------------------- */
    int         nRefCount = poFFPT->GetRepeatCount();
    DDFSubfieldDefn *poLNAM;
    char        **papszRefs = NULL;
    int         *panRIND = (int *) CPLMalloc(sizeof(int) * nRefCount);

    poLNAM = poFFPT->GetFieldDefn()->FindSubfieldDefn( "LNAM" );
    if( poLNAM == NULL )
        return;

    for( int iRef = 0; iRef < nRefCount; iRef++ )
    {
        unsigned char *pabyData;

        pabyData = (unsigned char *)
            poFFPT->GetSubfieldData( poLNAM, NULL, iRef );

        sprintf( szLNAM, "%02X%02X%02X%02X%02X%02X%02X%02X",
                 pabyData[1], pabyData[0], /* AGEN */
                 pabyData[5], pabyData[4], pabyData[3], pabyData[2], /* FIDN */
                 pabyData[7], pabyData[6] );

        papszRefs = CSLAddString( papszRefs, szLNAM );

        panRIND[iRef] = pabyData[8];
    }

    poFeature->SetField( "LNAM_REFS", papszRefs );
    CSLDestroy( papszRefs );

    poFeature->SetField( "FFPT_RIND", nRefCount, panRIND );
    CPLFree( panRIND );
}

/************************************************************************/
/*                       GenerateFSPTAttributes()                       */
/************************************************************************/

void S57Reader::GenerateFSPTAttributes( DDFRecord * poRecord,
                                        OGRFeature * poFeature )

{
/* -------------------------------------------------------------------- */
/*      Feature the spatial record containing the point.                */
/* -------------------------------------------------------------------- */
    DDFField    *poFSPT;
    int         nCount, i;

    poFSPT = poRecord->FindField( "FSPT" );
    if( poFSPT == NULL )
        return;

    nCount = poFSPT->GetRepeatCount();

/* -------------------------------------------------------------------- */
/*      Allocate working lists of the attributes.                       */
/* -------------------------------------------------------------------- */
    int *panORNT, *panUSAG, *panMASK, *panRCNM, *panRCID;

    panORNT = (int *) CPLMalloc( sizeof(int) * nCount );
    panUSAG = (int *) CPLMalloc( sizeof(int) * nCount );
    panMASK = (int *) CPLMalloc( sizeof(int) * nCount );
    panRCNM = (int *) CPLMalloc( sizeof(int) * nCount );
    panRCID = (int *) CPLMalloc( sizeof(int) * nCount );

/* -------------------------------------------------------------------- */
/*      loop over all entries, decoding them.                           */
/* -------------------------------------------------------------------- */
    for( i = 0; i < nCount; i++ )
    {
        panRCID[i] = ParseName( poFSPT, i, panRCNM + i );
        panORNT[i] = poRecord->GetIntSubfield( "FSPT", 0, "ORNT",i);
        panUSAG[i] = poRecord->GetIntSubfield( "FSPT", 0, "USAG",i);
        panMASK[i] = poRecord->GetIntSubfield( "FSPT", 0, "MASK",i);
    }

/* -------------------------------------------------------------------- */
/*      Assign to feature.                                              */
/* -------------------------------------------------------------------- */
    poFeature->SetField( "NAME_RCNM", nCount, panRCNM );
    poFeature->SetField( "NAME_RCID", nCount, panRCID );
    poFeature->SetField( "ORNT", nCount, panORNT );
    poFeature->SetField( "USAG", nCount, panUSAG );
    poFeature->SetField( "MASK", nCount, panMASK );

/* -------------------------------------------------------------------- */
/*      Cleanup.                                                        */
/* -------------------------------------------------------------------- */
    CPLFree( panRCNM );
    CPLFree( panRCID );
    CPLFree( panORNT );
    CPLFree( panUSAG );
    CPLFree( panMASK );
}

/************************************************************************/
/*                             ReadVector()                             */
/*                                                                      */
/*      Read a vector primitive objects based on the type (RCNM_)       */
/*      and index within the related index.                             */
/************************************************************************/

OGRFeature *S57Reader::ReadVector( int nFeatureId, int nRCNM )

{
    DDFRecordIndex *poIndex;
    const char *pszFDName = NULL;

/* -------------------------------------------------------------------- */
/*      What type of vector are we fetching.                            */
/* -------------------------------------------------------------------- */
    switch( nRCNM )
    {
      case RCNM_VI:
        poIndex = &oVI_Index;
        pszFDName = OGRN_VI;
        break;

      case RCNM_VC:
        poIndex = &oVC_Index;
        pszFDName = OGRN_VC;
        break;

      case RCNM_VE:
        poIndex = &oVE_Index;
        pszFDName = OGRN_VE;
        break;

      case RCNM_VF:
        poIndex = &oVF_Index;
        pszFDName = OGRN_VF;
        break;

      default:
        CPLAssert( FALSE );
        return NULL;
    }

    if( nFeatureId < 0 || nFeatureId >= poIndex->GetCount() )
        return NULL;

    DDFRecord *poRecord = poIndex->GetByIndex( nFeatureId );

/* -------------------------------------------------------------------- */
/*      Find the feature definition to use.                             */
/* -------------------------------------------------------------------- */
    OGRFeatureDefn *poFDefn = NULL;

    for( int i = 0; i < nFDefnCount; i++ )
    {
        if( EQUAL(papoFDefnList[i]->GetName(),pszFDName) )
        {
            poFDefn = papoFDefnList[i];
            break;
        }
    }

    if( poFDefn == NULL )
    {
        CPLAssert( FALSE );
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Create feature, and assign standard fields.                     */
/* -------------------------------------------------------------------- */
    OGRFeature *poFeature = new OGRFeature( poFDefn );

    poFeature->SetFID( nFeatureId );

    poFeature->SetField( "RCNM",
                         poRecord->GetIntSubfield( "VRID", 0, "RCNM",0) );
    poFeature->SetField( "RCID",
                         poRecord->GetIntSubfield( "VRID", 0, "RCID",0) );
    poFeature->SetField( "RVER",
                         poRecord->GetIntSubfield( "VRID", 0, "RVER",0) );
    poFeature->SetField( "RUIN",
                         poRecord->GetIntSubfield( "VRID", 0, "RUIN",0) );

/* -------------------------------------------------------------------- */
/*      Collect point geometries.                                       */
/* -------------------------------------------------------------------- */
    if( nRCNM == RCNM_VI || nRCNM == RCNM_VC )
    {
        double dfX=0.0, dfY=0.0, dfZ=0.0;

        if( poRecord->FindField( "SG2D" ) != NULL )
        {
            dfX = poRecord->GetIntSubfield("SG2D",0,"XCOO",0) / (double)nCOMF;
            dfY = poRecord->GetIntSubfield("SG2D",0,"YCOO",0) / (double)nCOMF;
            poFeature->SetGeometryDirectly( new OGRPoint( dfX, dfY ) );
        }

        else if( poRecord->FindField( "SG3D" ) != NULL ) /* presume sounding*/
        {
            int i, nVCount = poRecord->FindField("SG3D")->GetRepeatCount();
            if( nVCount == 1 )
            {
                dfX =poRecord->GetIntSubfield("SG3D",0,"XCOO",0)/(double)nCOMF;
                dfY =poRecord->GetIntSubfield("SG3D",0,"YCOO",0)/(double)nCOMF;
                dfZ =poRecord->GetIntSubfield("SG3D",0,"VE3D",0)/(double)nSOMF;
                poFeature->SetGeometryDirectly( new OGRPoint( dfX, dfY, dfZ ));
            }
            else
            {
                OGRMultiPoint *poMP = new OGRMultiPoint();

                for( i = 0; i < nVCount; i++ )
                {
                    dfX = poRecord->GetIntSubfield("SG3D",0,"XCOO",i)
                        / (double)nCOMF;
                    dfY = poRecord->GetIntSubfield("SG3D",0,"YCOO",i)
                        / (double)nCOMF;
                    dfZ = poRecord->GetIntSubfield("SG3D",0,"VE3D",i)
                        / (double)nSOMF;

                    poMP->addGeometryDirectly( new OGRPoint( dfX, dfY, dfZ ) );
                }

                poFeature->SetGeometryDirectly( poMP );
            }
        }

    }

/* -------------------------------------------------------------------- */
/*      Collect an edge geometry.                                       */
/* -------------------------------------------------------------------- */
    else if( nRCNM == RCNM_VE && poRecord->FindField( "SG2D" ) != NULL )
    {
          int i, nVCount = poRecord->FindField("SG2D")->GetRepeatCount();

          if(nVCount == 1)
          {
//              poRecord->Dump(stdout);
                OGRLineString *poLine = new OGRLineString();

                int jpt = 0;
                while(poRecord->FindField( "SG2D", jpt ) != NULL)
                {
                      poLine->setPoint(
                                       jpt,
                                       poRecord->GetIntSubfield("SG2D",jpt,"XCOO",0) / (double)nCOMF,
                                       poRecord->GetIntSubfield("SG2D",jpt,"YCOO",0) / (double)nCOMF );
                      jpt++;
                }

                poLine->setNumPoints( jpt );

                poFeature->SetGeometryDirectly( poLine );

          }
          else
          {

                  OGRLineString *poLine = new OGRLineString();

//        if(nVCount != 1)
//        {
//              poRecord->Dump(stdout);
//              nVCount = poRecord->FindField("SG2D")->GetRepeatCount();
//        }

                  poLine->setNumPoints( nVCount );

                  for( i = 0; i < nVCount; i++ )
                  {
                        poLine->setPoint(
                        i,
                        poRecord->GetIntSubfield("SG2D",0,"XCOO",i) / (double)nCOMF,
                        poRecord->GetIntSubfield("SG2D",0,"YCOO",i) / (double)nCOMF );
                  }
                  poFeature->SetGeometryDirectly( poLine );
          }
    }

/* -------------------------------------------------------------------- */
/*      Special edge fields.                                            */
/* -------------------------------------------------------------------- */
    DDFField *poVRPT;

    if( nRCNM == RCNM_VE
        && (poVRPT = poRecord->FindField( "VRPT" )) != NULL )
    {
        poFeature->SetField( "NAME_RCNM_0", RCNM_VC );
        poFeature->SetField( "NAME_RCID_0", ParseName( poVRPT, 0 ) );
        poFeature->SetField( "ORNT_0",
                             poRecord->GetIntSubfield("VRPT",0,"ORNT",0) );
        poFeature->SetField( "USAG_0",
                             poRecord->GetIntSubfield("VRPT",0,"USAG",0) );
        poFeature->SetField( "TOPI_0",
                             poRecord->GetIntSubfield("VRPT",0,"TOPI",0) );
        poFeature->SetField( "MASK_0",
                             poRecord->GetIntSubfield("VRPT",0,"MASK",0) );


        if(poVRPT->GetRepeatCount() > 1)
        {
            poFeature->SetField( "NAME_RCNM_1", RCNM_VC );
            poFeature->SetField( "NAME_RCID_1", ParseName( poVRPT, 1 ) );
            poFeature->SetField( "ORNT_1",
                              poRecord->GetIntSubfield("VRPT",0,"ORNT",1) );
            poFeature->SetField( "USAG_1",
                              poRecord->GetIntSubfield("VRPT",0,"USAG",1) );
            poFeature->SetField( "TOPI_1",
                              poRecord->GetIntSubfield("VRPT",0,"TOPI",1) );
            poFeature->SetField( "MASK_1",
                              poRecord->GetIntSubfield("VRPT",0,"MASK",1) );
        }
        else
        {
              DDFField *poVRPTEnd = poRecord->FindField( "VRPT", 1 );

              if(poVRPTEnd)
              {

                  poFeature->SetField( "NAME_RCNM_1", RCNM_VC );
                  poFeature->SetField( "NAME_RCID_1", ParseName( poVRPTEnd, 0 ) );
                  poFeature->SetField( "ORNT_1",
                                    poRecord->GetIntSubfield("VRPT",1,"ORNT",0) );
                  poFeature->SetField( "USAG_1",
                                    poRecord->GetIntSubfield("VRPT",1,"USAG",0) );
                  poFeature->SetField( "TOPI_1",
                                    poRecord->GetIntSubfield("VRPT",1,"TOPI",0) );
                  poFeature->SetField( "MASK_1",
                                    poRecord->GetIntSubfield("VRPT",1,"MASK",0) );
              }
              else
                    CPLDebug( "S57","Vector End Point not found, edge omitted." );
        }
    }

    return poFeature;
}

/************************************************************************/
/*                             FetchPoint()                             */
/*                                                                      */
/*      Fetch the location and quality of a spatial point object.       */
/************************************************************************/

int S57Reader::FetchPoint( int nRCNM, int nRCID,
                           double * pdfX, double * pdfY, double * pdfZ, int * pnquality )

{
    DDFRecord   *poSRecord;

    if( nRCNM == RCNM_VI )
        poSRecord = oVI_Index.FindRecord( nRCID );
    else
        poSRecord = oVC_Index.FindRecord( nRCID );

    if( poSRecord == NULL )
        return FALSE;

    //      Fetch the quality information
    if(NULL != pnquality)
    {
          DDFField *f;
          if( (f = poSRecord->FindField( "ATTV" )) != NULL )
          {
                DDFSubfieldDefn *sfd = (f->GetFieldDefn())->FindSubfieldDefn( "ATVL");
                if(NULL != sfd)
                {
                        int nSuccess;
                        char *s = (char *)poSRecord->GetStringSubfield("ATTV",0,"ATVL",0, &nSuccess) ;
                        if(nSuccess)
                        {
                              *pnquality = atoi(s);
                        }
                }
          }
    }

    double      dfX = 0.0, dfY = 0.0, dfZ = 0.0;

    if( poSRecord->FindField( "SG2D" ) != NULL )
    {
        dfX = poSRecord->GetIntSubfield("SG2D",0,"XCOO",0) / (double)nCOMF;
        dfY = poSRecord->GetIntSubfield("SG2D",0,"YCOO",0) / (double)nCOMF;
    }
    else if( poSRecord->FindField( "SG3D" ) != NULL )
    {
        dfX = poSRecord->GetIntSubfield("SG3D",0,"XCOO",0) / (double)nCOMF;
        dfY = poSRecord->GetIntSubfield("SG3D",0,"YCOO",0) / (double)nCOMF;
        dfZ = poSRecord->GetIntSubfield("SG3D",0,"VE3D",0) / (double)nSOMF;
    }
    else
        return FALSE;

    if( pdfX != NULL )
        *pdfX = dfX;
    if( pdfY != NULL )
        *pdfY = dfY;
    if( pdfZ != NULL )
        *pdfZ = dfZ;

    return TRUE;
}

/************************************************************************/
/*                       AssemblePointGeometry()                        */
/************************************************************************/

void S57Reader::AssemblePointGeometry( DDFRecord * poFRecord,
                                       OGRFeature * poFeature )

{
    DDFField    *poFSPT;
    int         nRCNM, nRCID;

/* -------------------------------------------------------------------- */
/*      Feature the spatial record containing the point.                */
/* -------------------------------------------------------------------- */
    poFSPT = poFRecord->FindField( "FSPT" );
    if( poFSPT == NULL )
        return;

    if( poFSPT->GetRepeatCount() != 1 )
    {
#ifdef DEBUG
        fprintf( stderr,
                 "Point features with other than one spatial linkage.\n" );
        poFRecord->Dump( stderr );
#endif
        CPLDebug( "S57",
           "Point feature encountered with other than one spatial linkage." );
    }

    nRCID = ParseName( poFSPT, 0, &nRCNM );

    double      dfX = 0.0, dfY = 0.0, dfZ = 0.0;

    int nquality = 10;                          // default is "precisely known"
    if( !FetchPoint( nRCNM, nRCID, &dfX, &dfY, &dfZ, &nquality ) )
    {
        CPLAssert( FALSE );
        return;
    }

    poFeature->SetGeometryDirectly( new OGRPoint( dfX, dfY, dfZ ) );
    OGRPoint * pp = (OGRPoint *)poFeature->GetGeometryRef();
    pp->setnQual(nquality);
}

/************************************************************************/
/*                      AssembleSoundingGeometry()                      */
/************************************************************************/

void S57Reader::AssembleSoundingGeometry( DDFRecord * poFRecord,
                                          OGRFeature * poFeature )

{
    DDFField    *poFSPT;
    int         nRCNM, nRCID;
    DDFRecord   *poSRecord;


/* -------------------------------------------------------------------- */
/*      Feature the spatial record containing the point.                */
/* -------------------------------------------------------------------- */
    poFSPT = poFRecord->FindField( "FSPT" );
    if( poFSPT == NULL )
        return;

    CPLAssert( poFSPT->GetRepeatCount() == 1 );

    nRCID = ParseName( poFSPT, 0, &nRCNM );

    if( nRCNM == RCNM_VI )
        poSRecord = oVI_Index.FindRecord( nRCID );
    else
        poSRecord = oVC_Index.FindRecord( nRCID );

    if( poSRecord == NULL )
        return;

/* -------------------------------------------------------------------- */
/*      Extract vertices.                                               */
/* -------------------------------------------------------------------- */
    OGRMultiPoint       *poMP = new OGRMultiPoint();
    DDFField            *poField;
    int                 nPointCount, i, nBytesLeft;
    DDFSubfieldDefn    *poXCOO, *poYCOO, *poVE3D;
    const char         *pachData;

    poField = poSRecord->FindField( "SG2D" );
    if( poField == NULL )
        poField = poSRecord->FindField( "SG3D" );
    if( poField == NULL )
        return;

    poXCOO = poField->GetFieldDefn()->FindSubfieldDefn( "XCOO" );
    poYCOO = poField->GetFieldDefn()->FindSubfieldDefn( "YCOO" );
    poVE3D = poField->GetFieldDefn()->FindSubfieldDefn( "VE3D" );

    nPointCount = poField->GetRepeatCount();

    pachData = poField->GetData();
    nBytesLeft = poField->GetDataSize();

    for( i = 0; i < nPointCount; i++ )
    {
        double          dfX, dfY, dfZ = 0.0;
        int             nBytesConsumed;

        dfY = poYCOO->ExtractIntData( pachData, nBytesLeft,
                                      &nBytesConsumed ) / (double) nCOMF;
        nBytesLeft -= nBytesConsumed;
        pachData += nBytesConsumed;

        dfX = poXCOO->ExtractIntData( pachData, nBytesLeft,
                                      &nBytesConsumed ) / (double) nCOMF;
        nBytesLeft -= nBytesConsumed;
        pachData += nBytesConsumed;

        if( poVE3D != NULL )
        {
            dfZ = poYCOO->ExtractIntData( pachData, nBytesLeft,
                                          &nBytesConsumed ) / (double) nSOMF;
            nBytesLeft -= nBytesConsumed;
            pachData += nBytesConsumed;
        }

        poMP->addGeometryDirectly( new OGRPoint( dfX, dfY, dfZ ) );
    }

    poFeature->SetGeometryDirectly( poMP );
}

/************************************************************************/
/*                        AssembleLineGeometry()                        */
/************************************************************************/

void S57Reader::AssembleLineGeometry( DDFRecord * poFRecord,
                                      OGRFeature * poFeature )

{
    DDFField    *poFSPT;
    int         nEdgeCount;
    OGRLineString *poLine = new OGRLineString();

/* -------------------------------------------------------------------- */
/*      Find the FSPT field.                                            */
/* -------------------------------------------------------------------- */
    poFSPT = poFRecord->FindField( "FSPT" );
    if( poFSPT == NULL )
        return;

    nEdgeCount = poFSPT->GetRepeatCount();

/* ==================================================================== */
/*      Loop collecting edges.                                          */
/* ==================================================================== */
    for( int iEdge = 0; iEdge < nEdgeCount; iEdge++ )
    {
        DDFRecord       *poSRecord;
        int             nRCID;

/* -------------------------------------------------------------------- */
/*      Find the spatial record for this edge.                          */
/* -------------------------------------------------------------------- */
        nRCID = ParseName( poFSPT, iEdge );

        poSRecord = oVE_Index.FindRecord( nRCID );
        if( poSRecord == NULL )
        {
            CPLError( CE_Warning, CPLE_AppDefined,
                      "Couldn't find spatial record %d.\n"
                      "Feature OBJL=%s, RCID=%d may have corrupt or"
                      "missing geometry.",
                      nRCID,
                      poFeature->GetDefnRef()->GetName(),
                      poFRecord->GetIntSubfield( "FRID", 0, "RCID", 0 ) );
            continue;
        }

/* -------------------------------------------------------------------- */
/*      Establish the number of vertices, and whether we need to        */
/*      reverse or not.                                                 */
/* -------------------------------------------------------------------- */
        int             nVCount;
        int             nStart, nEnd, nInc;
        DDFField        *poSG2D = poSRecord->FindField( "SG2D" );
        DDFSubfieldDefn *poXCOO=NULL, *poYCOO=NULL;

        if( poSG2D != NULL )
        {
            poXCOO = poSG2D->GetFieldDefn()->FindSubfieldDefn("XCOO");
            poYCOO = poSG2D->GetFieldDefn()->FindSubfieldDefn("YCOO");

            nVCount = poSG2D->GetRepeatCount();
        }
        else
            nVCount = 0;

        DDFField * poField = poSRecord->FindField( "VRPT" );
        int nVC_RCID0 = 0;
        int nVC_RCID1 = 0;
        int nVC_RCIDStart, nVC_RCIDEnd;

        if( poField == NULL )
        {
            CPLError( CE_Warning, CPLE_AppDefined,
                      "Couldn't find field VRPT in spatial record %d.\n"
                      "Feature OBJL=%s, RCID=%d may have corrupt or"
                      "missing geometry.",
                      nRCID,
                      poFeature->GetDefnRef()->GetName(),
                      poFRecord->GetIntSubfield( "FRID", 0, "RCID", 0 ) );
            continue;
        }
        
        if(poField->GetRepeatCount() > 1)
        {
              nVC_RCID0 = ParseName( poField, 0 );
              nVC_RCID1 = ParseName( poField, 1 );
        }
        else
        {
              nVC_RCID0 = ParseName( poField, 0 );
              DDFField * poFieldEnd = poSRecord->FindField( "VRPT", 1 );
              if(poFieldEnd)
                    nVC_RCID1 = ParseName( poFieldEnd, 0 );
        }


        if( poFRecord->GetIntSubfield( "FSPT", 0, "ORNT", iEdge ) == 2 )
        {
            nStart = nVCount-1;
            nEnd = 0;
            nInc = -1;
            nVC_RCIDStart = nVC_RCID1;          // reversed
            nVC_RCIDEnd =   nVC_RCID0;
        }
        else
        {
            nStart = 0;
            nEnd = nVCount-1;
            nInc = 1;
            nVC_RCIDStart = nVC_RCID0;
            nVC_RCIDEnd =   nVC_RCID1;
        }

/* -------------------------------------------------------------------- */
/*      Add the start node, if this is the first edge.                  */
/* -------------------------------------------------------------------- */
        if( iEdge == 0 )
        {
            int         nVC_RCID = 0;
            double      dfX, dfY;
/*
            if(poField)
            {
                  if( nInc == 1 )
                        nVC_RCID = ParseName( poField, 0 );
                  else
                        nVC_RCID = ParseName( poField, 1 );
            }
*/

            if( FetchPoint( RCNM_VC, nVC_RCIDStart, &dfX, &dfY ) )
                poLine->addPoint( dfX, dfY );
            else
                CPLError( CE_Warning, CPLE_AppDefined,
                          "Unable to fetch start node RCID%d.\n"
                          "Feature OBJL=%s, RCID=%d may have corrupt or"
                          " missing geometry.",
                          nVC_RCID,
                          poFeature->GetDefnRef()->GetName(),
                          poFRecord->GetIntSubfield( "FRID", 0, "RCID", 0 ) );
        }

/* -------------------------------------------------------------------- */
/*      Collect the vertices.                                           */
/* -------------------------------------------------------------------- */
        int             nVBase = poLine->getNumPoints();

        if(nVCount == 1)
        {
              int jpt = 0;
              while(poSRecord->FindField( "SG2D", jpt ))
              {
                    poLine->addPoint(
                                     poSRecord->GetIntSubfield("SG2D",jpt,"XCOO",0) / (double)nCOMF,
                                poSRecord->GetIntSubfield("SG2D",jpt,"YCOO",0) / (double)nCOMF );
                    jpt++;

              }
        }
        else
        {

            poLine->setNumPoints( nVCount+nVBase );

            for( int i = nStart; i != nEnd+nInc; i += nInc )
            {
                  double      dfX, dfY;
                  const char  *pachData;
                  int         nBytesRemaining;

                  pachData = poSG2D->GetSubfieldData(poXCOO,&nBytesRemaining,i);

                  dfX = poXCOO->ExtractIntData(pachData,nBytesRemaining,NULL)
                  / (double) nCOMF;

                  pachData = poSG2D->GetSubfieldData(poYCOO,&nBytesRemaining,i);

                  dfY = poXCOO->ExtractIntData(pachData,nBytesRemaining,NULL)
                  / (double) nCOMF;

                  poLine->setPoint( nVBase++, dfX, dfY );
            }
        }

/* -------------------------------------------------------------------- */
/*      Add the end node.                                               */
/* -------------------------------------------------------------------- */
        {
            int         nVC_RCID = 0;
            double      dfX, dfY;
/*
            if(poField)
            {
                  if( nInc == 1 )
                        nVC_RCID = ParseName( poField, 1 );
                  else
                        nVC_RCID = ParseName( poField, 0 );
            }
*/
            if( FetchPoint( RCNM_VC, nVC_RCIDEnd, &dfX, &dfY ) )
                poLine->addPoint( dfX, dfY );
            else
                CPLError( CE_Warning, CPLE_AppDefined,
                          "Unable to fetch end node RCID=%d.\n"
                          "Feature OBJL=%s, RCID=%d may have corrupt or"
                          " missing geometry.",
                          nVC_RCID,
                          poFeature->GetDefnRef()->GetName(),
                          poFRecord->GetIntSubfield( "FRID", 0, "RCID", 0 ) );
        }
    }

    if( poLine->getNumPoints() >= 2 )
        poFeature->SetGeometryDirectly( poLine );
    else
        delete poLine;
}

/************************************************************************/
/*                        AssembleAreaGeometry()                        */
/************************************************************************/

void S57Reader::AssembleAreaGeometry( DDFRecord * poFRecord,
                                         OGRFeature * poFeature )

{
    DDFField    *poFSPT;
    OGRGeometryCollection * poLines = new OGRGeometryCollection();
//    poFRecord->Dump(stdout);
/* -------------------------------------------------------------------- */
/*      Find the FSPT fields.                                           */
/* -------------------------------------------------------------------- */
    for( int iFSPT = 0;
         (poFSPT = poFRecord->FindField( "FSPT", iFSPT )) != NULL;
         iFSPT++ )
    {
        int         nEdgeCount;

        nEdgeCount = poFSPT->GetRepeatCount();

/* ==================================================================== */
/*      Loop collecting edges.                                          */
/* ==================================================================== */
        for( int iEdge = 0; iEdge < nEdgeCount; iEdge++ )
        {
            DDFRecord       *poSRecord;
            int             nRCID;

/* -------------------------------------------------------------------- */
/*      Find the spatial record for this edge.                          */
/* -------------------------------------------------------------------- */
            nRCID = ParseName( poFSPT, iEdge );

            poSRecord = oVE_Index.FindRecord( nRCID );
            if( poSRecord == NULL )
            {
                CPLError( CE_Warning, CPLE_AppDefined,
                          "Couldn't find spatial record %d.", nRCID );
                continue;
            }

/* -------------------------------------------------------------------- */
/*      Establish the number of vertices, and whether we need to        */
/*      reverse or not.                                                 */
/* -------------------------------------------------------------------- */
//            poSRecord->Dump(stdout);

            OGRLineString *poLine = new OGRLineString();

            int             nVCount;
            int             nStart, nEnd, nInc;
            DDFField        *poSG2D = poSRecord->FindField( "SG2D" );
            DDFSubfieldDefn *poXCOO=NULL, *poYCOO=NULL;

            if( poSG2D != NULL )
            {
                poXCOO = poSG2D->GetFieldDefn()->FindSubfieldDefn("XCOO");
                poYCOO = poSG2D->GetFieldDefn()->FindSubfieldDefn("YCOO");

                nVCount = poSG2D->GetRepeatCount();
            }
            else
                nVCount = 0;

            DDFField * poField = poSRecord->FindField( "VRPT" );
            int nVC_RCID0 = 0;
            int nVC_RCID1 = 0;
            int nVC_RCIDStart, nVC_RCIDEnd;

            if(poField && poField->GetRepeatCount() > 1)
            {
                  nVC_RCID0 = ParseName( poField, 0 );
                  nVC_RCID1 = ParseName( poField, 1 );
            }
            else
            {
                  if (poField)
                        nVC_RCID0 = ParseName( poField, 0 );
                  DDFField * poFieldEnd = poSRecord->FindField( "VRPT", 1 );
                  if(poFieldEnd)
                        nVC_RCID1 = ParseName( poFieldEnd, 0 );
            }


            if( poFRecord->GetIntSubfield( "FSPT", 0, "ORNT", iEdge ) == 2 )
            {
                  nStart = nVCount-1;
                  nEnd = 0;
                  nInc = -1;
                  nVC_RCIDStart = nVC_RCID1;          // reversed
                  nVC_RCIDEnd =   nVC_RCID0;
            }
            else
            {
                  nStart = 0;
                  nEnd = nVCount-1;
                  nInc = 1;
                  nVC_RCIDStart = nVC_RCID0;
                  nVC_RCIDEnd =   nVC_RCID1;
            }


/* -------------------------------------------------------------------- */
/*      Add the start node.                                             */
/* -------------------------------------------------------------------- */
            {
                double      dfX, dfY;

                if( FetchPoint( RCNM_VC, nVC_RCIDStart, &dfX, &dfY ) )
                    poLine->addPoint( dfX, dfY );
            }

/* -------------------------------------------------------------------- */
/*      Collect the vertices.                                           */
/* -------------------------------------------------------------------- */
            int             nVBase = poLine->getNumPoints();

            if(nVCount == 1)
            {
                  int jpt = 0;
                  while(poSRecord->FindField( "SG2D", jpt ))
                  {
                        poLine->addPoint(
                                          poSRecord->GetIntSubfield("SG2D",jpt,"XCOO",0) / (double)nCOMF,
                                          poSRecord->GetIntSubfield("SG2D",jpt,"YCOO",0) / (double)nCOMF );
                        jpt++;

                  }
            }
            else
            {
                  poLine->setNumPoints( nVCount+nVBase );

                  for( int i = nStart; i != nEnd+nInc; i += nInc )
                  {
                        double      dfX, dfY;
                        const char  *pachData;
                        int         nBytesRemaining;

                        pachData = poSG2D->GetSubfieldData(poXCOO,&nBytesRemaining,i);

                        dfX = poXCOO->ExtractIntData(pachData,nBytesRemaining,NULL) / (double) nCOMF;

                        pachData = poSG2D->GetSubfieldData(poYCOO,&nBytesRemaining,i);

                        dfY = poXCOO->ExtractIntData(pachData,nBytesRemaining,NULL) / (double) nCOMF;

                        poLine->setPoint( nVBase++, dfX, dfY );
                  }

            }
/* -------------------------------------------------------------------- */
/*      Add the end node.                                               */
/* -------------------------------------------------------------------- */
            {
                double      dfX, dfY;


                if( FetchPoint( RCNM_VC, nVC_RCIDEnd, &dfX, &dfY ) )
                    poLine->addPoint( dfX, dfY );
            }

            poLines->addGeometryDirectly( poLine );
        }
    }

/* -------------------------------------------------------------------- */
/*      Build lines into a polygon.                                     */
/* -------------------------------------------------------------------- */
    OGRPolygon  *poPolygon;
    OGRErr      eErr;

    poPolygon = (OGRPolygon *)
        OGRBuildPolygonFromEdges( (OGRGeometryH) poLines,
                                  TRUE, FALSE, 0.0, &eErr );
    if( eErr != OGRERR_NONE )
    {
        CPLError( CE_Warning, CPLE_AppDefined,
                  "Polygon assembly has failed for feature FIDN=%d,FIDS=%d.\n"
                  "Geometry may be missing or incomplete.",
                  poFeature->GetFieldAsInteger( "FIDN" ),
                  poFeature->GetFieldAsInteger( "FIDS" ) );
    }

    delete poLines;

    if( poPolygon != NULL )
        poFeature->SetGeometryDirectly( poPolygon );
}

/************************************************************************/
/*                             FindFDefn()                              */
/*                                                                      */
/*      Find the OGRFeatureDefn corresponding to the passed feature     */
/*      record.  It will search based on geometry class, or object      */
/*      class depending on the bClassBased setting.                     */
/************************************************************************/

OGRFeatureDefn * S57Reader::FindFDefn( DDFRecord * poRecord )

{
    if( poRegistrar != NULL )
    {
        int     nOBJL = poRecord->GetIntSubfield( "FRID", 0, "OBJL", 0 );

        if( !poRegistrar->SelectClass( nOBJL ) )
        {
            for( int i = 0; i < nFDefnCount; i++ )
            {
                if( EQUAL(papoFDefnList[i]->GetName(),"Generic") )
                    return papoFDefnList[i];
            }
            return NULL;
        }

        for( int i = 0; i < nFDefnCount; i++ )
        {
            if( EQUAL(papoFDefnList[i]->GetName(),
                      poRegistrar->GetAcronym()) )
                return papoFDefnList[i];
        }

        return NULL;
    }
    else
    {
        int     nPRIM = poRecord->GetIntSubfield( "FRID", 0, "PRIM", 0 );
        OGRwkbGeometryType eGType;

        if( nPRIM == PRIM_P )
            eGType = wkbPoint;
        else if( nPRIM == PRIM_L )
            eGType = wkbLineString;
        else if( nPRIM == PRIM_A )
            eGType = wkbPolygon;
        else
            eGType = wkbNone;

        for( int i = 0; i < nFDefnCount; i++ )
        {
            if( papoFDefnList[i]->GetGeomType() == eGType )
                return papoFDefnList[i];
        }
    }

    return NULL;
}

/************************************************************************/
/*                             ParseName()                              */
/*                                                                      */
/*      Pull the RCNM and RCID values from a NAME field.  The RCID      */
/*      is returned and the RCNM can be gotten via the pnRCNM argument. */
/************************************************************************/

int S57Reader::ParseName( DDFField * poField, int nIndex, int * pnRCNM )

{
    unsigned char       *pabyData;

    pabyData = (unsigned char *)
        poField->GetSubfieldData(
            poField->GetFieldDefn()->FindSubfieldDefn( "NAME" ),
            NULL, nIndex );

    if( pnRCNM != NULL )
        *pnRCNM = pabyData[0];

    return pabyData[1]
         + pabyData[2] * 256
         + pabyData[3] * 256 * 256
         + pabyData[4] * 256 * 256 * 256;
}

/************************************************************************/
/*                           AddFeatureDefn()                           */
/************************************************************************/

void S57Reader::AddFeatureDefn( OGRFeatureDefn * poFDefn )

{
    nFDefnCount++;
    papoFDefnList = (OGRFeatureDefn **)
        CPLRealloc(papoFDefnList, sizeof(OGRFeatureDefn*)*nFDefnCount );

    papoFDefnList[nFDefnCount-1] = poFDefn;
}

/************************************************************************/
/*                          CollectClassList()                          */
/*                                                                      */
/*      Establish the list of classes (unique OBJL values) that         */
/*      occur in this dataset.                                          */
/************************************************************************/

int S57Reader::CollectClassList(int *panClassCount, int nMaxClass )

{
    int         bSuccess = TRUE;

    if( !bFileIngested )
        Ingest();

    for( int iFEIndex = 0; iFEIndex < oFE_Index.GetCount(); iFEIndex++ )
    {
        DDFRecord *poRecord = oFE_Index.GetByIndex( iFEIndex );
        int     nOBJL = poRecord->GetIntSubfield( "FRID", 0, "OBJL", 0 );

        if( nOBJL >= nMaxClass )
            bSuccess = FALSE;
        else
            panClassCount[nOBJL]++;

    }

    return bSuccess;
}

/************************************************************************/
/*                         ApplyRecordUpdate()                          */
/*                                                                      */
/*      Update one target record based on an S-57 update record         */
/*      (RUIN=3).                                                       */
/************************************************************************/

int S57Reader::ApplyRecordUpdate( DDFRecord *poTarget, DDFRecord *poUpdate )

{
    const char *pszKey = poUpdate->GetField(1)->GetFieldDefn()->GetName();

/* -------------------------------------------------------------------- */
/*      Validate versioning.                                            */
/* -------------------------------------------------------------------- */
    if( poTarget->GetIntSubfield( pszKey, 0, "RVER", 0 ) + 1
        != poUpdate->GetIntSubfield( pszKey, 0, "RVER", 0 )  )
    {
          CPLError( CE_Warning, CPLE_AppDefined,
                  "On RecordUpdate, mismatched RVER value for RCNM=%d,RCID=%d...update RVER is %d, target RVER is %d.",
                  poTarget->GetIntSubfield( pszKey, 0, "RCNM", 0 ),
                  poTarget->GetIntSubfield( pszKey, 0, "RCID", 0 ),
                  poUpdate->GetIntSubfield( pszKey, 0, "RVER", 0 ),
                  poTarget->GetIntSubfield( pszKey, 0, "RVER", 0 ) );

        CPLAssert( FALSE );
        return FALSE;
    }

    //    More checks for validity
    if( poUpdate->FindField( "FRID" ) != NULL )
    {
/*
          int up_FIDN = poUpdate->GetIntSubfield( "FOID", 0, "FIDN", 0 );
          int up_FIDS = poUpdate->GetIntSubfield( "FOID", 0, "FIDS", 0 );
          int tar_FIDN = poTarget->GetIntSubfield( "FOID", 0, "FIDN", 0 );
          int tar_FIDS = poTarget->GetIntSubfield( "FOID", 0, "FIDS", 0 );
          if((up_FIDN != tar_FIDN) || (up_FIDS != tar_FIDS))
          {
          CPLError( CE_Warning, CPLE_AppDefined,
          "On RecordUpdate, mismatched FIDN/FIDS.... target FIDN=%d, target FIDS=%d   update FIDN=%d, update FIDS=%d.",
          tar_FIDN, tar_FIDS, up_FIDN, up_FIDS);

          return FALSE;
    }
*/
          int up_PRIM = poUpdate->GetIntSubfield( "FRID", 0, "PRIM", 0 );
          int tar_PRIM = poTarget->GetIntSubfield( "FRID", 0, "PRIM", 0 );
          if(up_PRIM != tar_PRIM)
          {
                CPLError( CE_Warning, CPLE_AppDefined,
                          "On RecordUpdate, mismatched PRIM.... target PRIM=%d, update PRIM=%d",
                          tar_PRIM, up_PRIM);
                return FALSE;
          }
    }


/* -------------------------------------------------------------------- */
/*      Update the target version.                                      */
/* -------------------------------------------------------------------- */
    unsigned char       *pnRVER;
    DDFField    *poKey = poTarget->FindField( pszKey );
    DDFSubfieldDefn *poRVER_SFD;

    if( poKey == NULL )
    {
        CPLAssert( FALSE );
        return FALSE;
    }

    poRVER_SFD = poKey->GetFieldDefn()->FindSubfieldDefn( "RVER" );
    if( poRVER_SFD == NULL )
        return FALSE;

    pnRVER = (unsigned char *) poKey->GetSubfieldData( poRVER_SFD, NULL, 0 );

    *pnRVER += 1;

/* -------------------------------------------------------------------- */
/*      Check for, and apply record record to spatial record pointer    */
/*      updates.                                                        */
/* -------------------------------------------------------------------- */
    if( poUpdate->FindField( "FSPC" ) != NULL )
    {
        int     nFSUI = poUpdate->GetIntSubfield( "FSPC", 0, "FSUI", 0 );
        int     nFSIX = poUpdate->GetIntSubfield( "FSPC", 0, "FSIX", 0 );
        int     nNSPT = poUpdate->GetIntSubfield( "FSPC", 0, "NSPT", 0 );
        DDFField *poSrcFSPT = poUpdate->FindField( "FSPT" );
        DDFField *poDstFSPT = poTarget->FindField( "FSPT" );
        int     nPtrSize;

        if( (poSrcFSPT == NULL && nFSUI != 2) || poDstFSPT == NULL )
        {
            CPLAssert( FALSE );
            return FALSE;
        }

        nPtrSize = poDstFSPT->GetFieldDefn()->GetFixedWidth();

        if( nFSUI == 1 ) /* INSERT */
        {
            char        *pachInsertion;
            int         nInsertionBytes = nPtrSize * nNSPT;

            pachInsertion = (char *) CPLMalloc(nInsertionBytes + nPtrSize);
            memcpy( pachInsertion, poSrcFSPT->GetData(), nInsertionBytes );

            /*
            ** If we are inserting before an instance that already
            ** exists, we must add it to the end of the data being
            ** inserted.
            */
            if( nFSIX <= poDstFSPT->GetRepeatCount() )
            {
                memcpy( pachInsertion + nInsertionBytes,
                        poDstFSPT->GetData() + nPtrSize * (nFSIX-1),
                        nPtrSize );
                nInsertionBytes += nPtrSize;
            }

            poTarget->SetFieldRaw( poDstFSPT, nFSIX - 1,
                                   pachInsertion, nInsertionBytes );
            CPLFree( pachInsertion );
        }
        else if( nFSUI == 2 ) /* DELETE */
        {
            /* Wipe each deleted coordinate */
            for( int i = nNSPT-1; i >= 0; i-- )
            {
                poTarget->SetFieldRaw( poDstFSPT, i + nFSIX - 1, NULL, 0 );
            }
        }
        else if( nFSUI == 3 ) /* MODIFY */
        {
            /* copy over each ptr */
            for( int i = 0; i < nNSPT; i++ )
            {
                const char *pachRawData;

                pachRawData = poSrcFSPT->GetData() + nPtrSize * i;

                poTarget->SetFieldRaw( poDstFSPT, i + nFSIX - 1,
                                       pachRawData, nPtrSize );
            }
        }
    }

/* -------------------------------------------------------------------- */
/*      Check for, and apply vector record to vector record pointer     */
/*      updates.                                                        */
/* -------------------------------------------------------------------- */
    if( poUpdate->FindField( "VRPC" ) != NULL )
    {
        int     nVPUI = poUpdate->GetIntSubfield( "VRPC", 0, "VPUI", 0 );
        int     nVPIX = poUpdate->GetIntSubfield( "VRPC", 0, "VPIX", 0 );
        int     nNVPT = poUpdate->GetIntSubfield( "VRPC", 0, "NVPT", 0 );
        DDFField *poSrcVRPT = poUpdate->FindField( "VRPT" );
        DDFField *poDstVRPT = poTarget->FindField( "VRPT" );
        int     nPtrSize;

        if( (poSrcVRPT == NULL && nVPUI != 2) || poDstVRPT == NULL )
        {
            CPLAssert( FALSE );
            return FALSE;
        }

        nPtrSize = poDstVRPT->GetFieldDefn()->GetFixedWidth();

        if( nVPUI == 1 ) /* INSERT */
        {
            char        *pachInsertion;
            int         nInsertionBytes = nPtrSize * nNVPT;

            pachInsertion = (char *) CPLMalloc(nInsertionBytes + nPtrSize);
            memcpy( pachInsertion, poSrcVRPT->GetData(), nInsertionBytes );

            /*
            ** If we are inserting before an instance that already
            ** exists, we must add it to the end of the data being
            ** inserted.
            */
            if( nVPIX <= poDstVRPT->GetRepeatCount() )
            {
                memcpy( pachInsertion + nInsertionBytes,
                        poDstVRPT->GetData() + nPtrSize * (nVPIX-1),
                        nPtrSize );
                nInsertionBytes += nPtrSize;
            }

            poTarget->SetFieldRaw( poDstVRPT, nVPIX - 1,
                                   pachInsertion, nInsertionBytes );
            CPLFree( pachInsertion );
        }
        else if( nVPUI == 2 ) /* DELETE */
        {
            /* Wipe each deleted coordinate */
            for( int i = nNVPT-1; i >= 0; i-- )
            {
                poTarget->SetFieldRaw( poDstVRPT, i + nVPIX - 1, NULL, 0 );
            }
        }
        else if( nVPUI == 3 ) /* MODIFY */
        {
            /* copy over each ptr */
            for( int i = 0; i < nNVPT; i++ )
            {
                const char *pachRawData;

                pachRawData = poSrcVRPT->GetData() + nPtrSize * i;

                poTarget->SetFieldRaw( poDstVRPT, i + nVPIX - 1,
                                       pachRawData, nPtrSize );
            }
        }
    }

/* -------------------------------------------------------------------- */
/*      Check for, and apply record update to coordinates.              */
/* -------------------------------------------------------------------- */
    if( poUpdate->FindField( "SGCC" ) != NULL )
    {
        int     nCCUI = poUpdate->GetIntSubfield( "SGCC", 0, "CCUI", 0 );
        int     nCCIX = poUpdate->GetIntSubfield( "SGCC", 0, "CCIX", 0 );
        int     nCCNC = poUpdate->GetIntSubfield( "SGCC", 0, "CCNC", 0 );
        DDFField *poSrcSG2D = poUpdate->FindField( "SG2D" );
        DDFField *poDstSG2D = poTarget->FindField( "SG2D" );
        int     nCoordSize;

        /* If we don't have SG2D, check for SG3D */
        if( poDstSG2D == NULL )
        {
            poSrcSG2D = poUpdate->FindField( "SG3D" );
            poDstSG2D = poTarget->FindField( "SG3D" );
        }

        if( (poSrcSG2D == NULL && nCCUI != 2) || poDstSG2D == NULL )
        {
            CPLAssert( FALSE );
            return FALSE;
        }

        nCoordSize = poDstSG2D->GetFieldDefn()->GetFixedWidth();

        if( nCCUI == 1 ) /* INSERT */
        {
            char        *pachInsertion;
            int         nInsertionBytes = nCoordSize * nCCNC;

            pachInsertion = (char *) CPLMalloc(nInsertionBytes + nCoordSize);
            memcpy( pachInsertion, poSrcSG2D->GetData(), nInsertionBytes );

            /*
            ** If we are inserting before an instance that already
            ** exists, we must add it to the end of the data being
            ** inserted.
            */
            if( nCCIX <= poDstSG2D->GetRepeatCount() )
            {
                memcpy( pachInsertion + nInsertionBytes,
                        poDstSG2D->GetData() + nCoordSize * (nCCIX-1),
                        nCoordSize );
                nInsertionBytes += nCoordSize;
            }

            poTarget->SetFieldRaw( poDstSG2D, nCCIX - 1,
                                   pachInsertion, nInsertionBytes );
            CPLFree( pachInsertion );

        }
        else if( nCCUI == 2 ) /* DELETE */
        {
            /* Wipe each deleted coordinate */
            for( int i = nCCNC-1; i >= 0; i-- )
            {
                poTarget->SetFieldRaw( poDstSG2D, i + nCCIX - 1, NULL, 0 );
            }
        }
        else if( nCCUI == 3 ) /* MODIFY */
        {
            /* copy over each ptr */
            for( int i = 0; i < nCCNC; i++ )
            {
                const char *pachRawData;

                pachRawData = poSrcSG2D->GetData() + nCoordSize * i;

                poTarget->SetFieldRaw( poDstSG2D, i + nCCIX - 1,
                                       pachRawData, nCoordSize );
            }
        }
    }

/* -------------------------------------------------------------------- */
/*      We don't currently handle FFPC (feature to feature linkage)     */
/*      issues, but we will at least report them when debugging.        */
/* -------------------------------------------------------------------- */
/*
    if( poUpdate->FindField( "FFPC" ) != NULL )
    {
        CPLDebug( "S57", "Found FFPC, but not applying it." );
    }
*/
/* -------------------------------------------------------------------- */
/*      Check for and apply changes to attribute lists.                 */
/* -------------------------------------------------------------------- */
    if( poUpdate->FindField( "ATTF" ) != NULL )
    {
        DDFSubfieldDefn *poSrcATVLDefn;
        DDFField *poSrcATTF = poUpdate->FindField( "ATTF" );
        DDFField *poDstATTF = poTarget->FindField( "ATTF" );

        if(NULL == poDstATTF)
        {
              //  This probably means that the update applies to an attribute that doesn't (yet) exist
              //  To fix, we need to add an attribute, then update it.
              CPLDebug( "S57","Could not find target ATTF field for attribute update");
              return FALSE;
        }

        int     nRepeatCount = poSrcATTF->GetRepeatCount();

        poSrcATVLDefn = poSrcATTF->GetFieldDefn()->FindSubfieldDefn( "ATVL" );

        for( int iAtt = 0; iAtt < nRepeatCount; iAtt++ )
        {
            int nATTL = poUpdate->GetIntSubfield( "ATTF", 0, "ATTL", iAtt );
            int iTAtt, nDataBytes;
            const char *pszRawData;

            for( iTAtt = poDstATTF->GetRepeatCount()-1; iTAtt >= 0; iTAtt-- )
            {
                if( poTarget->GetIntSubfield( "ATTF", 0, "ATTL", iTAtt )
                    == nATTL )
                    break;
            }
            if( iTAtt == -1 )
                iTAtt = poDstATTF->GetRepeatCount();

            pszRawData = poSrcATTF->GetInstanceData( iAtt, &nDataBytes );
            poTarget->SetFieldRaw( poDstATTF, iTAtt, pszRawData, nDataBytes );
        }
    }

    if( poUpdate->FindField( "NATF" ) != NULL )
    {
        bool b_newField = false;
        DDFSubfieldDefn *poSrcATVLDefn;
        DDFField *poSrcATTF = poUpdate->FindField( "NATF" );
        DDFField *poDstATTF = poTarget->FindField( "NATF" );
 
//        int up_FIDN = poUpdate->GetIntSubfield( "FOID", 0, "FIDN", 0 );
//        if(up_FIDN == 1103712044 /*1225530334*/){
//            poTarget->Dump(stdout);
//        }
        
        if(NULL == poDstATTF)
        {
            //  This probably means that the update applies to an attribute that doesn't (yet) exist
            //  To fix, we need to add an attribute, then update it.
            
            DDFFieldDefn *poNATF = poTarget->GetModule()->FindFieldDefn( "NATF" );
            poTarget->AddField(poNATF);
            poDstATTF = poTarget->FindField( "NATF" );
            b_newField = true;
            
//            poTarget->Dump(stdout);
            
//            CPLDebug( "S57","Could not find target ATTF field for attribute update");
//           return FALSE;
        }
        
        int     nRepeatCount = poSrcATTF->GetRepeatCount();
        
        poSrcATVLDefn = poSrcATTF->GetFieldDefn()->FindSubfieldDefn( "ATVL" );
        
        for( int iAtt = 0; iAtt < nRepeatCount; iAtt++ )
        {
            int nATTL = poUpdate->GetIntSubfield( "NATF", 0, "ATTL", iAtt );
            int iTAtt, nDataBytes;
            const char *pszRawData;
            
            for( iTAtt = poDstATTF->GetRepeatCount()-1; iTAtt >= 0; iTAtt-- )
            {
                if( poTarget->GetIntSubfield( "NATF", 0, "ATTL", iTAtt ) == nATTL )
                    break;
            }
            if( iTAtt == -1 )
                iTAtt = poDstATTF->GetRepeatCount();

            //  If we just added a new field above, then the first attribute will be 0.
            //   We should replace this one    
            if(b_newField){
                if( poTarget->GetIntSubfield( "NATF", 0, "ATTL", 0 ) == 0){
                    iTAtt = 0;
                    b_newField = false;
                }
            }
            
                
            pszRawData = poSrcATTF->GetInstanceData( iAtt, &nDataBytes );
            
//            poTarget->Dump(stdout);
            poTarget->SetFieldRaw( poDstATTF, iTAtt, pszRawData, nDataBytes ); ///dsr
//            poTarget->Dump(stdout);
            
        }
    }
    
    return TRUE;
}


/************************************************************************/
/*                            ApplyUpdates()                            */
/*                                                                      */
/*      Read records from an update file, and apply them to the         */
/*      currently loaded index of features.                             */
/************************************************************************/

int S57Reader::ApplyUpdates( DDFModule *poUpdateModule, int iUpdate )

{
    DDFRecord   *poRecord;

    int ret_code = 0;

/* -------------------------------------------------------------------- */
/*      Ensure base file is loaded.                                     */
/* -------------------------------------------------------------------- */
    Ingest();

/* -------------------------------------------------------------------- */
/*      Read records, and apply as updates.                             */
/* -------------------------------------------------------------------- */
    while( (poRecord = poUpdateModule->ReadRecord()) != NULL )
    {
        DDFField        *poKeyField = poRecord->GetField(1);
        const char      *pszKey = poKeyField->GetFieldDefn()->GetName();

        if( EQUAL(pszKey,"VRID") || EQUAL(pszKey,"FRID"))
        {
            int         nRCNM = poRecord->GetIntSubfield( pszKey,0, "RCNM",0 );
            int         nRCID = poRecord->GetIntSubfield( pszKey,0, "RCID",0 );
            int         nRVER = poRecord->GetIntSubfield( pszKey,0, "RVER",0 );
            int         nRUIN = poRecord->GetIntSubfield( pszKey,0, "RUIN",0 );
            DDFRecordIndex *poIndex = NULL;

            if( EQUAL(poKeyField->GetFieldDefn()->GetName(),"VRID") )
            {
                switch( nRCNM )
                {
                  case RCNM_VI:
                    poIndex = &oVI_Index;
                    break;

                  case RCNM_VC:
                    poIndex = &oVC_Index;
                    break;

                  case RCNM_VE:
                    poIndex = &oVE_Index;
                    break;

                  case RCNM_VF:
                    poIndex = &oVF_Index;
                    break;

                  default:
                    CPLAssert( FALSE );
                    break;
                }
            }
            else
            {
                poIndex = &oFE_Index;
            }

            if( poIndex != NULL )
            {
                if( nRUIN == 1 )  /* insert */
                {
//                      CPLDebug( "S57","Insert Record, RCID=%d", nRCID);
                      poIndex->AddRecord( nRCID, poRecord->CloneOn(poModule) );
                }
                else if( nRUIN == 2 ) /* delete */
                {
//                      CPLDebug( "S57","Remove Record, RCID=%d", nRCID);
                      DDFRecord   *poTarget;

                    poTarget = poIndex->FindRecord( nRCID );
                    if( poTarget == NULL )
                    {
                        CPLError( CE_Warning, CPLE_AppDefined,
                                  "While applying update %d, Can't find RCNM=%d,RCID=%d for delete.",
                                  iUpdate, nRCNM, nRCID );
                        ret_code = BAD_UPDATE;
                    }
                    else if( poTarget->GetIntSubfield( pszKey, 0, "RVER", 0 )
                             != nRVER - 1 )
                    {
                        CPLError( CE_Warning, CPLE_AppDefined,
                                  "While applying update %d, On RecordRemove, mismatched RVER value for RCNM=%d,RCID=%d...update RVER is %d, target RVER is %d.",
                                  iUpdate, nRCNM, nRCID, nRVER, poTarget->GetIntSubfield( pszKey, 0, "RVER", 0 ) );
                        CPLError( CE_Warning, CPLE_AppDefined,
                                  "While applying update %d, Removal of RCNM=%d,RCID=%d failed.",
                                  iUpdate, nRCNM, nRCID );
                        ret_code = BAD_UPDATE;

                    }
                    else
                    {
                          poIndex->RemoveRecord( nRCID );
                    }
                }

                else if( nRUIN == 3 ) /* modify in place */
                {
//                    CPLDebug( "S57","Update Record, RCID=%d", nRCID);
                    DDFRecord   *poTarget;

                    poTarget = poIndex->FindRecord( nRCID );
                    if( poTarget == NULL )
                    {
                        CPLError( CE_Warning, CPLE_AppDefined,
                                  "While applying update %d, Can't find RCNM=%d,RCID=%d for update.",
                                  iUpdate, nRCNM, nRCID );
                        ret_code = BAD_UPDATE;

                    }
                    else
                    {
                        if( !ApplyRecordUpdate( poTarget, poRecord ) )
                        {
                            CPLError( CE_Warning, CPLE_AppDefined,
                                      "While applying update %d, an update to RCNM=%d,RCID=%d failed.",
                                      iUpdate, nRCNM, nRCID );
                            ret_code = BAD_UPDATE;
                        }
                    }
                }
            }
        }

        else if( EQUAL(pszKey,"DSID") )
        {
            /* ignore */;
        }

        else
        {
            CPLDebug( "S57",
                      "While applying update %d, Skipping %s record in S57Reader::ApplyUpdates().",
                      iUpdate, pszKey );
            ret_code = BAD_UPDATE;

        }
    }

    return ret_code;
}

/************************************************************************/
/*                        FindAndApplyUpdates()                         */
/*                                                                      */
/*      Find all update files that would appear to apply to this        */
/*      base file.                                                      */
/************************************************************************/

int S57Reader::FindAndApplyUpdates( const char * pszPath )

{
    int         iUpdate;
    int         bSuccess = TRUE;
    int         ret_code = 0;

    if( pszPath == NULL )
        pszPath = pszModuleName;

    if( !EQUAL(CPLGetExtension(pszPath),"000") )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Can't apply updates to a base file with a different\n"
                  "extension than .000." );
        return BAD_UPDATE;
    }

    for( iUpdate = 1; bSuccess; iUpdate++ )
    {
        char    szExtension[4];
        char    *pszUpdateFilename;
        DDFModule oUpdateModule;

        sprintf( szExtension, "%03d", iUpdate );

        pszUpdateFilename = CPLStrdup(CPLResetExtension(pszPath,szExtension));

        bSuccess = oUpdateModule.Open( pszUpdateFilename, TRUE );

        if( bSuccess )
            CPLDebug( "S57", "Applying feature updates from %s.",
                      pszUpdateFilename );
        CPLFree( pszUpdateFilename );

        if( bSuccess )
        {
              int update_ret = ApplyUpdates( &oUpdateModule, iUpdate );
              if(update_ret)
                    ret_code = update_ret;
        }
    }

    return ret_code;
}

/************************************************************************/
/*                             GetExtent()                              */
/*                                                                      */
/*      Scan all the cached records collecting spatial bounds as        */
/*      efficiently as possible for this transfer.                      */
/************************************************************************/

OGRErr S57Reader::GetExtent( OGREnvelope *psExtent, int bForce )

{
#define INDEX_COUNT     4

    DDFRecordIndex      *apoIndex[INDEX_COUNT];

/* -------------------------------------------------------------------- */
/*      If we aren't forced to get the extent say no if we haven't      */
/*      already indexed the iso8211 records.                            */
/* -------------------------------------------------------------------- */
    if( !bForce && !bFileIngested )
        return OGRERR_FAILURE;

    Ingest();

/* -------------------------------------------------------------------- */
/*      We will scan all the low level vector elements for extents      */
/*      coordinates.                                                    */
/* -------------------------------------------------------------------- */
    int         bGotExtents = FALSE;
    int         nXMin=0, nXMax=0, nYMin=0, nYMax=0;

    apoIndex[0] = &oVI_Index;
    apoIndex[1] = &oVC_Index;
    apoIndex[2] = &oVE_Index;
    apoIndex[3] = &oVF_Index;

    for( int iIndex = 0; iIndex < INDEX_COUNT; iIndex++ )
    {
        DDFRecordIndex  *poIndex = apoIndex[iIndex];

        for( int iVIndex = 0; iVIndex < poIndex->GetCount(); iVIndex++ )
        {
            DDFRecord *poRecord = poIndex->GetByIndex( iVIndex );
            DDFField    *poSG3D = poRecord->FindField( "SG3D" );
            DDFField    *poSG2D = poRecord->FindField( "SG2D" );

            if( poSG3D != NULL )
            {
                int     i, nVCount = poSG3D->GetRepeatCount();
                GInt32  *panData, nX, nY;

                panData = (GInt32 *) poSG3D->GetData();
                for( i = 0; i < nVCount; i++ )
                {
                    nX = CPL_LSBWORD32(panData[i*3+1]);
                    nY = CPL_LSBWORD32(panData[i*3+0]);

                    if( bGotExtents )
                    {
                        nXMin = MIN(nXMin,nX);
                        nXMax = MAX(nXMax,nX);
                        nYMin = MIN(nYMin,nY);
                        nYMax = MAX(nYMax,nY);
                    }
                    else
                    {
                        nXMin = nXMax = nX;
                        nYMin = nYMax = nY;
                        bGotExtents = TRUE;
                    }
                }
            }
            else if( poSG2D != NULL )
            {
                int     i, nVCount = poSG2D->GetRepeatCount();
                GInt32  *panData, nX, nY;

                panData = (GInt32 *) poSG2D->GetData();
                for( i = 0; i < nVCount; i++ )
                {
                    nX = CPL_LSBWORD32(panData[i*2+1]);
                    nY = CPL_LSBWORD32(panData[i*2+0]);

                    if( bGotExtents )
                    {
                        nXMin = MIN(nXMin,nX);
                        nXMax = MAX(nXMax,nX);
                        nYMin = MIN(nYMin,nY);
                        nYMax = MAX(nYMax,nY);
                    }
                    else
                    {
                        nXMin = nXMax = nX;
                        nYMin = nYMax = nY;
                        bGotExtents = TRUE;
                    }
                }
            }
        }
    }

    if( !bGotExtents )
        return OGRERR_FAILURE;
    else
    {
        psExtent->MinX = nXMin / (double) nCOMF;
        psExtent->MaxX = nXMax / (double) nCOMF;
        psExtent->MinY = nYMin / (double) nCOMF;
        psExtent->MaxY = nYMax / (double) nCOMF;

        return OGRERR_NONE;
    }
}

