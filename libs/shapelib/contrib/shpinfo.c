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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shapefil.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("shpinfo shp_file\n");
        return EXIT_FAILURE;
    }

    SHPHandle hSHP = SHPOpen(argv[1], "rb");
    if (hSHP == NULL)
    {
        printf("Unable to open:%s\n", argv[1]);
        return EXIT_FAILURE;
    }

    int nEntities;
    int nShapeType;
    double adfBndsMin[4];
    double adfBndsMax[4];
    SHPGetInfo(hSHP, &nEntities, &nShapeType, adfBndsMin, adfBndsMax);
    SHPClose(hSHP);

    // TODO(schwehr): Make a function for all of shapelib.
    const char *sType = NULL;  // [15]= "";
    switch (nShapeType)
    {
        case SHPT_POINT:
            sType = "Point";
            break;
        case SHPT_ARC:
            sType = "Polyline";
            break;
        case SHPT_POLYGON:
            sType = "";
            break;
        case SHPT_MULTIPOINT:
            sType = "MultiPoint";
            break;
        default:
            // TODO(schwehr): Handle all of the SHPT types.
            sType = "UNKNOWN";
    }

    printf("Info for %s\n", argv[1]);
    printf("%s(%d), %d Records in file\n", sType, nShapeType, nEntities);

    // Print out the file bounds.
    // TODO(schwehr): Do a better job at formatting the results.
    printf("File Bounds: (%15.10lg,%15.10lg)\n", adfBndsMin[0], adfBndsMin[1]);
    printf("\t(%15.10lg,%15.10lg)\n", adfBndsMax[0], adfBndsMax[1]);

    return EXIT_SUCCESS;
}
