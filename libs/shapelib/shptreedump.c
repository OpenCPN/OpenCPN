/******************************************************************************
 *
 * Project:  Shapelib
 * Purpose:  Mainline for creating and dumping an ASCII representation of
 *           a quadtree.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1999, Frank Warmerdam
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.0-or-later
 ******************************************************************************
 *
 */

#include "shapefil.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void SHPTreeNodeDump(SHPTree *, SHPTreeNode *, const char *, int);
static void SHPTreeNodeSearchAndDump(SHPTree *, double *, double *);

/************************************************************************/
/*                               Usage()                                */
/************************************************************************/

static void Usage()

{
    printf("shptreedump [-maxdepth n] [-search xmin ymin xmax ymax]\n"
           "            [-v] [-o indexfilename] [-i indexfilename]\n"
           "            shp_file\n");
    exit(1);
}

/************************************************************************/
/*                                main()                                */
/************************************************************************/
int main(int argc, char **argv)

{
    int nExpandShapes = 0;
    int nMaxDepth = 0;
    bool bDoSearch = false;
    double adfSearchMin[4];
    double adfSearchMax[4];
    const char *pszOutputIndexFilename = NULL;
    const char *pszInputIndexFilename = NULL;
    const char *pszTargetFile = NULL;

    /* -------------------------------------------------------------------- */
    /*	Consume flags.							*/
    /* -------------------------------------------------------------------- */
    while (argc > 1)
    {
        if (strcmp(argv[1], "-v") == 0)
        {
            nExpandShapes = 1;
            argv++;
            argc--;
        }
        else if (strcmp(argv[1], "-maxdepth") == 0 && argc > 2)
        {
            nMaxDepth = atoi(argv[2]);
            argv += 2;
            argc -= 2;
        }
        else if (strcmp(argv[1], "-o") == 0 && argc > 2)
        {
            pszOutputIndexFilename = argv[2];
            argv += 2;
            argc -= 2;
        }
        else if (strcmp(argv[1], "-i") == 0 && argc > 2)
        {
            pszInputIndexFilename = argv[2];
            argv += 2;
            argc -= 2;
        }
        else if (strcmp(argv[1], "-search") == 0 && argc > 5)
        {
            bDoSearch = true;

            adfSearchMin[0] = atof(argv[2]);
            adfSearchMin[1] = atof(argv[3]);
            adfSearchMax[0] = atof(argv[4]);
            adfSearchMax[1] = atof(argv[5]);

            adfSearchMin[2] = adfSearchMax[2] = 0.0;
            adfSearchMin[3] = adfSearchMax[3] = 0.0;

            if (adfSearchMin[0] > adfSearchMax[0] ||
                adfSearchMin[1] > adfSearchMax[1])
            {
                printf("Min greater than max in search criteria.\n");
                Usage();
            }

            argv += 5;
            argc -= 5;
        }
        else if (pszTargetFile == NULL)
        {
            pszTargetFile = argv[1];
            argv++;
            argc--;
        }
        else
        {
            printf("Unrecognised argument: %s\n", argv[1]);
            Usage();
        }
    }

    /* -------------------------------------------------------------------- */
    /*      Do a search with an existing index file?                        */
    /* -------------------------------------------------------------------- */
    if (bDoSearch && pszInputIndexFilename != NULL)
    {
        FILE *fp = fopen(pszInputIndexFilename, "rb");

        if (fp == NULL)
        {
            perror(pszInputIndexFilename);
            exit(1);
        }

        int nResultCount = 0;
        int *panResult =
            SHPSearchDiskTree(fp, adfSearchMin, adfSearchMax, &nResultCount);

        printf("Result: ");
        for (int iResult = 0; iResult < nResultCount; iResult++)
            printf("%d ", panResult[iResult]);
        printf("\n");
        free(panResult);

        fclose(fp);

        exit(0);
    }

    /* -------------------------------------------------------------------- */
    /*      Display a usage message.                                        */
    /* -------------------------------------------------------------------- */
    if (pszTargetFile == NULL)
    {
        Usage();
    }

    /* -------------------------------------------------------------------- */
    /*      Open the passed shapefile.                                      */
    /* -------------------------------------------------------------------- */
    SHPHandle hSHP = SHPOpen(pszTargetFile, "rb");

    if (hSHP == NULL)
    {
        printf("Unable to open:%s\n", pszTargetFile);
        exit(1);
    }

    /* -------------------------------------------------------------------- */
    /*      Build a quadtree structure for this file.                       */
    /* -------------------------------------------------------------------- */
    SHPTree *psTree = SHPCreateTree(hSHP, 2, nMaxDepth, NULL, NULL);

    /* -------------------------------------------------------------------- */
    /*      Trim unused nodes from the tree.                                */
    /* -------------------------------------------------------------------- */
    SHPTreeTrimExtraNodes(psTree);

    /* -------------------------------------------------------------------- */
    /*      Dump tree to .qix file.                                         */
    /* -------------------------------------------------------------------- */
    if (pszOutputIndexFilename != NULL)
    {
        SHPWriteTree(psTree, pszOutputIndexFilename);
    }

    /* -------------------------------------------------------------------- */
    /*      Dump tree by recursive descent.                                 */
    /* -------------------------------------------------------------------- */
    else if (!bDoSearch)
        SHPTreeNodeDump(psTree, psTree->psRoot, "", nExpandShapes);

    /* -------------------------------------------------------------------- */
    /*      or do a search instead.                                         */
    /* -------------------------------------------------------------------- */
    else
        SHPTreeNodeSearchAndDump(psTree, adfSearchMin, adfSearchMax);

    /* -------------------------------------------------------------------- */
    /*      cleanup                                                         */
    /* -------------------------------------------------------------------- */
    SHPDestroyTree(psTree);

    SHPClose(hSHP);

#ifdef USE_DBMALLOC
    malloc_dump(2);
#endif

    exit(0);
}

/************************************************************************/
/*                           EmitCoordinate()                           */
/************************************************************************/

static void EmitCoordinate(double *padfCoord, int nDimension)

{
    const char *pszFormat;

    if (fabs(padfCoord[0]) < 180 && fabs(padfCoord[1]) < 180)
        pszFormat = "%.9f";
    else
        pszFormat = "%.2f";

    printf(pszFormat, padfCoord[0]);
    printf(",");
    printf(pszFormat, padfCoord[1]);

    if (nDimension > 2)
    {
        printf(",");
        printf(pszFormat, padfCoord[2]);
    }
    if (nDimension > 3)
    {
        printf(",");
        printf(pszFormat, padfCoord[3]);
    }
}

/************************************************************************/
/*                             EmitShape()                              */
/************************************************************************/

static void EmitShape(SHPObject *psObject, const char *pszPrefix,
                      int nDimension)

{
    printf("%s( Shape\n", pszPrefix);
    printf("%s  ShapeId = %d\n", pszPrefix, psObject->nShapeId);

    printf("%s  Min = (", pszPrefix);
    EmitCoordinate(&psObject->dfXMin, nDimension);
    printf(")\n");

    printf("%s  Max = (", pszPrefix);
    EmitCoordinate(&psObject->dfXMax, nDimension);
    printf(")\n");

    for (int i = 0; i < psObject->nVertices; i++)
    {
        double adfVertex[4];

        printf("%s  Vertex[%d] = (", pszPrefix, i);

        adfVertex[0] = psObject->padfX[i];
        adfVertex[1] = psObject->padfY[i];
        adfVertex[2] = psObject->padfZ[i];
        adfVertex[3] = psObject->padfM[i];

        EmitCoordinate(adfVertex, nDimension);
        printf(")\n");
    }
    printf("%s)\n", pszPrefix);
}

/************************************************************************/
/*                          SHPTreeNodeDump()                           */
/*                                                                      */
/*      Dump a tree node in a readable form.                            */
/************************************************************************/

static void SHPTreeNodeDump(SHPTree *psTree, SHPTreeNode *psTreeNode,
                            const char *pszPrefix, int nExpandShapes)

{
    char szNextPrefix[150];

    strcpy(szNextPrefix, pszPrefix);
    if (strlen(pszPrefix) < sizeof(szNextPrefix) - 3)
        strcat(szNextPrefix, "  ");

    printf("%s( SHPTreeNode\n", pszPrefix);

    /* -------------------------------------------------------------------- */
    /*      Emit the bounds.                                                */
    /* -------------------------------------------------------------------- */
    printf("%s  Min = (", pszPrefix);
    EmitCoordinate(psTreeNode->adfBoundsMin, psTree->nDimension);
    printf(")\n");

    printf("%s  Max = (", pszPrefix);
    EmitCoordinate(psTreeNode->adfBoundsMax, psTree->nDimension);
    printf(")\n");

    /* -------------------------------------------------------------------- */
    /*      Emit the list of shapes on this node.                           */
    /* -------------------------------------------------------------------- */
    if (nExpandShapes)
    {
        printf("%s  Shapes(%d):\n", pszPrefix, psTreeNode->nShapeCount);
        for (int i = 0; i < psTreeNode->nShapeCount; i++)
        {
            SHPObject *psObject;

            psObject = SHPReadObject(psTree->hSHP, psTreeNode->panShapeIds[i]);
            assert(psObject != NULL);
            if (psObject != NULL)
            {
                EmitShape(psObject, szNextPrefix, psTree->nDimension);
            }

            SHPDestroyObject(psObject);
        }
    }
    else
    {
        printf("%s  Shapes(%d): ", pszPrefix, psTreeNode->nShapeCount);
        for (int i = 0; i < psTreeNode->nShapeCount; i++)
        {
            printf("%d ", psTreeNode->panShapeIds[i]);
        }
        printf("\n");
    }

    /* -------------------------------------------------------------------- */
    /*      Emit subnodes.                                                  */
    /* -------------------------------------------------------------------- */
    for (int i = 0; i < psTreeNode->nSubNodes; i++)
    {
        if (psTreeNode->apsSubNode[i] != NULL)
            SHPTreeNodeDump(psTree, psTreeNode->apsSubNode[i], szNextPrefix,
                            nExpandShapes);
    }

    printf("%s)\n", pszPrefix);

    return;
}

/************************************************************************/
/*                      SHPTreeNodeSearchAndDump()                      */
/************************************************************************/

static void SHPTreeNodeSearchAndDump(SHPTree *hTree, double *padfBoundsMin,
                                     double *padfBoundsMax)

{
    /* -------------------------------------------------------------------- */
    /*      Perform the search for likely candidates.  These are shapes     */
    /*      that fall into a tree node whose bounding box intersects our    */
    /*      area of interest.                                               */
    /* -------------------------------------------------------------------- */
    int nShapeCount;
    int *panHits = SHPTreeFindLikelyShapes(hTree, padfBoundsMin, padfBoundsMax,
                                           &nShapeCount);

    /* -------------------------------------------------------------------- */
    /*      Read all of these shapes, and establish whether the shape's     */
    /*      bounding box actually intersects the area of interest.  Note    */
    /*      that the bounding box could intersect the area of interest,     */
    /*      and the shape itself still not cross it but we don't try to     */
    /*      address that here.                                              */
    /* -------------------------------------------------------------------- */
    for (int i = 0; i < nShapeCount; i++)
    {
        SHPObject *psObject = SHPReadObject(hTree->hSHP, panHits[i]);
        if (psObject == NULL)
            continue;

        if (!SHPCheckBoundsOverlap(padfBoundsMin, padfBoundsMax,
                                   &psObject->dfXMin, &psObject->dfXMax,
                                   hTree->nDimension))
        {
            printf("Shape %d: not in area of interest, but fetched.\n",
                   panHits[i]);
        }
        else
        {
            printf("Shape %d: appears to be in area of interest.\n",
                   panHits[i]);
        }

        SHPDestroyObject(psObject);
    }

    if (nShapeCount == 0)
        printf("No shapes found in search.\n");
}
