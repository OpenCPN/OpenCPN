/******************************************************************************
 *
 * Project:  Shapelib
 * Purpose:  Sample application for adding a record to an existing .dbf file.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1999, Frank Warmerdam
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.0-or-later
 ******************************************************************************
 */

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "shapefil.h"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("dbfadd xbase_file field_values\n");
        return EXIT_FAILURE;
    }

    DBFHandle hDBF = DBFOpen(argv[1], "r+b");
    if (hDBF == NULL)
    {
        printf("DBFOpen(%s,\"rb+\") failed.\n", argv[1]);
        return 2;
    }

    // Do we have the correct number of arguments?
    if (DBFGetFieldCount(hDBF) != argc - 2)
    {
        printf("Got %d fields, but require %d\n", argc - 2,
               DBFGetFieldCount(hDBF));
        DBFClose(hDBF);
        return 3;
    }

    const int iRecord = DBFGetRecordCount(hDBF);

    // Loop assigning the new field values.
    for (int i = 0; i < DBFGetFieldCount(hDBF); i++)
    {
        if (strcmp(argv[i + 2], "") == 0)
            DBFWriteNULLAttribute(hDBF, iRecord, i);
        else if (DBFGetFieldInfo(hDBF, i, NULL, NULL, NULL) == FTString)
            DBFWriteStringAttribute(hDBF, iRecord, i, argv[i + 2]);
        else
            DBFWriteDoubleAttribute(hDBF, iRecord, i, atof(argv[i + 2]));
    }

    DBFClose(hDBF);

    return 0;
}
