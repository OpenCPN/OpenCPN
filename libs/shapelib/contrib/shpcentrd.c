/******************************************************************************
 * Copyright (c) 1999, Carl Anderson
 *
 * this code is based in part on the earlier work of Frank Warmerdam
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
 * shpcentrd.c  - compute XY centroid for complex shapes
 *			and create a new SHPT_PT file of then
 * 			specifically undo compound objects but not complex ones
 */

/* the centroid is defined as
 *
 *      Cx = sum (x dArea ) / Total Area
 *  and
 *      Cy = sum (y dArea ) / Total Area
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shapefil.h"
#include "shpgeo.h"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("shpcentrd shp_file new_shp_file\n");
        return 1;
    }

    DBFHandle old_DBF = DBFOpen(argv[1], "rb");
    if (old_DBF == NULL)
    {
        printf("Unable to DBFOpen old files:%s\n", argv[1]);
        return 1;
    }

    SHPHandle old_SHP = SHPOpen(argv[1], "rb");
    if (old_SHP == NULL)
    {
        printf("Unable to SHPOpen old files:%s\n", argv[1]);
        DBFClose(old_DBF);
        return 1;
    }

    int nEntities;
    int nShapeType;
    SHPGetInfo(old_SHP, &nEntities, &nShapeType, NULL, NULL);

    DBFHandle new_DBF = DBFCloneEmpty(old_DBF, argv[2]);
    if (new_DBF == NULL)
    {
        printf("Unable to create dbf for new files:%s\n", argv[2]);
        DBFClose(old_DBF);
        SHPClose(old_SHP);
        return 1;
    }

    SHPHandle new_SHP = SHPCreate(argv[2], SHPT_POINT);
    if (new_SHP == NULL)
    {
        printf("Unable to create new files:%s\n", argv[2]);
        DBFClose(old_DBF);
        SHPClose(old_SHP);
        DBFClose(new_DBF);
        return 1;
    }

    char *DBFRow = (char *)malloc(old_DBF->nRecordLength + 15);

    int byRing = 1;
    for (int i = 0; i < nEntities; i++)
    {
        SHPObject *psCShape = SHPReadObject(old_SHP, i);

        if (byRing == 1)
        {
            for (int ring = 0; ring < psCShape->nParts; ring++)
            {
                SHPObject *psO = SHPClone(psCShape, ring, ring + 1);

                PT Centrd = SHPCentrd_2d(psO);

                SHPObject *cent_pt;
                cent_pt =
                    SHPCreateSimpleObject(SHPT_POINT, 1, (double *)&Centrd.x,
                                          (double *)&Centrd.y, NULL);

                SHPWriteObject(new_SHP, -1, cent_pt);

                memcpy(DBFRow, DBFReadTuple(old_DBF, i),
                       old_DBF->nRecordLength);
                DBFWriteTuple(new_DBF, new_DBF->nRecords, DBFRow);

                SHPDestroyObject(cent_pt);

                SHPDestroyObject(psO);
            }
        }
        else
        {
            PT Centrd = SHPCentrd_2d(psCShape);

            SHPObject *cent_pt =
                SHPCreateSimpleObject(SHPT_POINT, 1, (double *)&Centrd.x,
                                      (double *)&Centrd.y, NULL);

            SHPWriteObject(new_SHP, -1, cent_pt);

            memcpy(DBFRow, DBFReadTuple(old_DBF, i), old_DBF->nRecordLength);
            DBFWriteTuple(new_DBF, new_DBF->nRecords, DBFRow);

            SHPDestroyObject(cent_pt);
        }
    }

    printf("\n");

    DBFClose(old_DBF);
    SHPClose(old_SHP);

    DBFClose(new_DBF);
    SHPClose(new_SHP);

    return EXIT_SUCCESS;
}
