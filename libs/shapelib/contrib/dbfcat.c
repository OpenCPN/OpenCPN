/*
 * Copyright (c) 1995 Frank Warmerdam
 *
 * This code is in the public domain.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shapefil.h"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("dbfcat [-v] [-f] from_DBFfile to_DBFfile\n");
        exit(1);
    }

    int force = 0;
    int verbose = 0;
    int shift = 0;
    if (strcmp("-v", argv[1]) == 0)
    {
        shift = 1;
        verbose = 1;
    }
    if (strcmp("-f", argv[1 + shift]) == 0)
    {
        shift++;
        force = 1;
    }
    if (strcmp("-v", argv[1 + shift]) == 0)
    {
        shift++;
        verbose = 1;
    }

    char tfile[160];
    strcpy(tfile, argv[1 + shift]);
    strcat(tfile, ".dbf");

    DBFHandle hDBF = DBFOpen(tfile, "rb");
    if (hDBF == NULL)
    {
        printf("DBFOpen(%s.dbf,\"r\") failed for From_DBF.\n", tfile);
        exit(2);
    }

    strcpy(tfile, argv[2 + shift]);
    strcat(tfile, ".dbf");

    DBFHandle cDBF = DBFOpen(tfile, "rb+");
    if (cDBF == NULL)
    {
        printf("DBFOpen(%s.dbf,\"rb+\") failed for To_DBF.\n", tfile);
        exit(2);
    }

    if (DBFGetFieldCount(hDBF) == 0)
    {
        printf("There are no fields in this table!\n");
        exit(3);
    }

    const int hflds = DBFGetFieldCount(hDBF);
    const int cflds = DBFGetFieldCount(cDBF);

    int matches = 0;
    int mismatch = 0;

    DBFFieldType hType;
    int nWidth;
    int nDecimals;
    char fld_m[256];
    int cnWidth;
    int cnDecimals;
    const char type_names[4][15] = {"integer", "string", "double", "double"};
    char nTitle[32];
    char cTitle[32];

    for (int i = 0; i < hflds; i++)
    {
        char szTitle[18];
        hType = DBFGetFieldInfo(hDBF, i, szTitle, &nWidth, &nDecimals);

        char cname[18];
        fld_m[i] = -1;
        for (int j = 0; j < cflds; j++)
        {
            const DBFFieldType cType =
                DBFGetFieldInfo(cDBF, j, cname, &cnWidth, &cnDecimals);
            if (strcmp(cname, szTitle) == 0)
            {
                if (hType != cType)
                {
                    printf("Incompatible fields %s(%s) != %s(%s),\n",
                           type_names[hType], nTitle, type_names[cType],
                           cTitle);
                    mismatch = 1;
                }
                fld_m[i] = j;
                if (verbose)
                {
                    printf("%s  %s(%d,%d) <- %s  %s(%d,%d)\n", cname,
                           type_names[cType], cnWidth, cnDecimals, szTitle,
                           type_names[hType], nWidth, nDecimals);
                }
                j = cflds;
                matches = 1;
            }
        }
    }

    if ((matches == 0) && !force)
    {
        printf("ERROR: No field names match for tables, cannot proceed\n   use "
               "-f to force processing using blank records\n");
        exit(-1);
    }
    if (mismatch && !force)
    {
        printf("ERROR: field type mismatch cannot proceed\n    use -f to force "
               "processing using attempted conversions\n");
        exit(-1);
    }

    int iRecord = 0;
    for (; iRecord < DBFGetRecordCount(hDBF); iRecord++)
    {
        const int ciRecord = DBFGetRecordCount(cDBF);
        for (int i = 0; i < hflds; i++)
        {
            const int ci = fld_m[i];
            if (ci != -1)
            {
                const DBFFieldType cType =
                    DBFGetFieldInfo(cDBF, ci, cTitle, &cnWidth, &cnDecimals);
                hType = DBFGetFieldInfo(hDBF, i, nTitle, &nWidth, &nDecimals);

                switch (cType)
                {
                    case FTString:
                    case FTLogical:
                    case FTDate:
                        DBFWriteStringAttribute(
                            cDBF, ciRecord, ci,
                            (char *)DBFReadStringAttribute(hDBF, iRecord, i));
                        break;

                    case FTInteger:
                        DBFWriteIntegerAttribute(
                            cDBF, ciRecord, ci,
                            (int)DBFReadIntegerAttribute(hDBF, iRecord, i));
                        break;

                    case FTDouble:
                        /*	        cf = DBFReadDoubleAttribute( hDBF, iRecord, i );
	        printf ("%s <-  %s (%f)\n", cTitle, nTitle, cf);
*/
                        DBFWriteDoubleAttribute(
                            cDBF, ciRecord, ci,
                            (double)DBFReadDoubleAttribute(hDBF, iRecord, i));
                        break;

                    case FTInvalid:
                        break;
                }
            }
        } /* fields names match */
    }

    if (verbose)
    {
        printf(" %d records appended \n\n", iRecord);
    }
    DBFClose(hDBF);
    DBFClose(cDBF);

    return (0);
}
