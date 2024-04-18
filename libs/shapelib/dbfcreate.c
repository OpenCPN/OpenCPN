/******************************************************************************
 *
 * Project:  Shapelib
 * Purpose:  Sample application for creating a new .dbf file.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1999, Frank Warmerdam
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.0-or-later
 ******************************************************************************
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shapefil.h"

int main(int argc, char **argv)
{
    // Display a usage message.
    if (argc < 2)
    {
        printf("dbfcreate xbase_file [[-s field_name width], "
               "[-n field_name width decimals]]...\n");
        return 1;
    }

    // Create the database.
    DBFHandle hDBF = DBFCreate(argv[1]);
    if (hDBF == NULL)
    {
        printf("DBFCreate(%s) failed.\n", argv[1]);
        return 2;
    }

    // Loop over the field definitions adding new fields.
    for (int i = 2; i < argc; i++)
    {
        if (i < argc - 2 && strcmp(argv[i], "-s") == 0)
        {
            const char *field = argv[i + 1];
            const int width = atoi(argv[i + 2]);
            const int decimals = 0;
            if (DBFAddField(hDBF, field, FTString, width, decimals) == -1)
            {
                printf("DBFAddField(%s,FTString,%d,0) failed.\n", field, width);
                DBFClose(hDBF);
                return 4;
            }
            i += 2;
        }
        else if (i < argc - 3 && strcmp(argv[i], "-n") == 0)
        {
            const char *field = argv[i + 1];
            const int width = atoi(argv[i + 2]);
            const int decimals = atoi(argv[i + 3]);
            if (DBFAddField(hDBF, field, FTDouble, width, decimals) == -1)
            {
                printf("DBFAddField(%s,FTDouble,%d,%d) failed.\n", field, width,
                       decimals);
                DBFClose(hDBF);
                return 4;
            }
            i += 3;
        }
        else
        {
            printf("Argument incomplete, or unrecognised: %s\n", argv[i]);
            DBFClose(hDBF);
            return 3;
        }
    }

    DBFClose(hDBF);
    return 0;
}
