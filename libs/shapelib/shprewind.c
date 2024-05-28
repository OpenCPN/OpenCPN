/******************************************************************************
 *
 * Project:  Shapelib
 * Purpose:  Utility to validate and reset the winding order of rings in
 *           polygon geometries to match the ordering required by spec.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2002, Frank Warmerdam
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.0-or-later
 ******************************************************************************
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "shapefil.h"

int main(int argc, char **argv)
{
    /* -------------------------------------------------------------------- */
    /*      Display a usage message.                                        */
    /* -------------------------------------------------------------------- */
    if (argc != 3)
    {
        printf("shprewind in_shp_file out_shp_file\n");
        exit(1);
    }

    /* -------------------------------------------------------------------- */
    /*      Open the passed shapefile.                                      */
    /* -------------------------------------------------------------------- */
    SHPHandle hSHP = SHPOpen(argv[1], "rb");

    if (hSHP == NULL)
    {
        printf("Unable to open:%s\n", argv[1]);
        exit(1);
    }

    int nShapeType;
    int nEntities;
    double adfMinBound[4];
    double adfMaxBound[4];
    SHPGetInfo(hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound);

    /* -------------------------------------------------------------------- */
    /*      Create output shapefile.                                        */
    /* -------------------------------------------------------------------- */
    SHPHandle hSHPOut = SHPCreate(argv[2], nShapeType);

    if (hSHPOut == NULL)
    {
        printf("Unable to create:%s\n", argv[2]);
        exit(1);
    }

    /* -------------------------------------------------------------------- */
    /*	Skim over the list of shapes, printing all the vertices.	*/
    /* -------------------------------------------------------------------- */
    int nInvalidCount = 0;

    for (int i = 0; i < nEntities; i++)
    {
        SHPObject *psShape = SHPReadObject(hSHP, i);
        if (SHPRewindObject(hSHP, psShape))
            nInvalidCount++;
        SHPWriteObject(hSHPOut, -1, psShape);
        SHPDestroyObject(psShape);
    }

    SHPClose(hSHP);
    SHPClose(hSHPOut);

    printf("%d objects rewound.\n", nInvalidCount);

    exit(0);
}
