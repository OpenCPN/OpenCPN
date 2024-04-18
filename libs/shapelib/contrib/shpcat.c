/******************************************************************************
 * Copyright (c) 1999, Carl Anderson
 *
 * This code is based in part on the earlier work of Frank Warmerdam
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
 *	shpcat
 *
 *  Utility program to concatenate two shapefiles
 *  Must be used in concert with dbfcat
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shapefil.h"

int main(int argc, char **argv)
{
    /* -------------------------------------------------------------------- */
    /*      Display a usage message.                                        */
    /* -------------------------------------------------------------------- */
    if (argc != 3)
    {
        printf("shpcat from_shpfile to_shpfile\n");
        return 1;
    }

    /* -------------------------------------------------------------------- */
    /*      Open the passed shapefile.                                      */
    /* -------------------------------------------------------------------- */
    SHPHandle hSHP = SHPOpen(argv[1], "rb");
    if (hSHP == NULL)
    {
        printf("Unable to open:%s\n", argv[1]);
        return 1;
    }

    int nEntities;
    int nShapeType;
    SHPGetInfo(hSHP, &nEntities, &nShapeType, NULL, NULL);
    fprintf(stderr, "Opened From File %s, with %d shapes\n", argv[1],
            nEntities);

    /* -------------------------------------------------------------------- */
    /*      Open the passed shapefile.                                      */
    /* -------------------------------------------------------------------- */
    SHPHandle cSHP = SHPOpen(argv[2], "rb+");
    if (cSHP == NULL)
    {
        printf("Unable to open:%s\n", argv[2]);
        SHPClose(hSHP);
        return 1;
    }

    int nShpInFile;
    SHPGetInfo(cSHP, &nShpInFile, NULL, NULL, NULL);
    fprintf(stderr, "Opened to file %s with %d shapes, ready to add %d\n",
            argv[2], nShpInFile, nEntities);

    /* -------------------------------------------------------------------- */
    /*	Skim over the list of shapes, printing all the vertices.	*/
    /* -------------------------------------------------------------------- */
    for (int i = 0; i < nEntities; i++)
    {
        SHPObject *shape = SHPReadObject(hSHP, i);
        SHPWriteObject(cSHP, -1, shape);
        SHPDestroyObject(shape);
    }

    SHPClose(hSHP);
    SHPClose(cSHP);

    return 0;
}
