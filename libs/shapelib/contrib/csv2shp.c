// TODO(schwehr): Untested.

/*
csv2shp - converts a character delimited file to a ESRI shapefile
Copyright (C) 2005 Springs Rescue Mission

LICENSE
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

The GNU General Public License is also available from the web
site <http://www.gnu.org>.

GRATITUDE
Like this program?  Donate at <http://springsrescuemission.org>.

USAGE NOTES
This program operates on single points only (not polygons or lines).

The input file may be a .csv file (comma separated values) or tab-separated
values, or it may be separated by any other character.  The first row must
contain column names.  There must be each a column named longitude and
latitude in the input file.

The .csv parser does not understand text delimiters (e.g. quotation mark).
It parses fields only by the given field delimiter (e.g. comma or tab).
The program has not been tested with null values, and in this case, the
behavior is undefined.  The program will not accept lines with a trailing
delimiter character.

All columns (including longitude and latitude) in the input file are exported
to the .dbf file.

The program attempts to find the best type (integer, decimal, string) and
smallest size of the fields necessary for the .dbf file.


SUPPORT
Springs Rescue Mission does not offer any support for this program.


CONTACT INFORMATION
Springs Rescue Mission
5 West Las Vegas St
PO Box 2108
Colorado Springs CO 80901
Web: <http://springsrescuemission.org>
Email: <http://springsrescuemission.org/email.php?recipient=webmaster>

*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shapefil.h"

#define MAX_COLUMNS 30

#if defined(_MSC_VER)
#define STRCASECMP(a, b) (_stricmp(a, b))
#elif defined(WIN32) || defined(_WIN32)
#define STRCASECMP(a, b) (stricmp(a, b))
#else
#include <strings.h>
#define STRCASECMP(a, b) (strcasecmp(a, b))
#endif

typedef struct column_t
{
    DBFFieldType eType;
    int nWidth;
    int nDecimals;
} column;

/* counts the number of occurrences of the character in the string */
int strnchr(const char *s, char c)
{
    int n = 0;

    for (size_t x = 0; x < strlen(s); x++)
    {
        if (c == s[x])
        {
            n++;
        }
    }

    return n;
}

/* Returns a field given by column n (0-based) in a character-
   delimited string s */
char *delimited_column(char *s, char delim, int n)
{
    if (strnchr(s, delim) < n)
    {
        fprintf(stderr, "delimited_column: n is too large\n");
        return NULL;
    }

    char szbuffer[4096]; /* a copy of s */
    strcpy(szbuffer, s);

    char szdelimiter[2]; /* delim converted to string */
    szdelimiter[0] = delim;
    szdelimiter[1] = '\0';

    int x = 0;
    const char *pchar = strtok(szbuffer, szdelimiter);
    while (x < n)
    {
        pchar = strtok(NULL, szdelimiter);
        x++;
    }

    if (NULL == pchar)
    {
        return NULL;
    }

    static char szreturn[4096];
    strcpy(szreturn, pchar);
    return szreturn;
}

/* returns the number of decimals in a real number given as a string s */
int str_to_ndecimals(const char *s)
{
    if (s == NULL)
    {
        return -1;
    }

    /* Check for float: ^-?[0-9]+\.[0-9]+$ */
    if (!isdigit(s[0]) && s[0] != '-')
    {
        return -1;
    }

    size_t len = strlen(s);
    if (!isdigit(s[len - 1]))
    {
        return -1;
    }

    const char *decimalPoint = strchr(s, '.');
    if ((decimalPoint == NULL) || (strchr(decimalPoint + 1, '.') != NULL))
    {
        return -1;
    }

    for (size_t x = 1; x < len - 1; x++)
    {
        if (!isdigit(s[x]) && s[x] != '.')
        {
            return -1;
        }
    }

    return (int)strlen(decimalPoint + 1);
}

/* Determines the most specific column type.
   The most specific types from most to least are integer, float, string.  */
DBFFieldType str_to_fieldtype(const char *s)
{
    size_t len = strlen(s);

    /* Check for integer: ^[0-9]+$ */
    int isInteger = 1;
    for (size_t x = 0; x < len; x++)
    {
        if (!isdigit(s[x]))
        {
            isInteger = 0;
            break;
        }
    }
    if (isInteger)
    {
        return FTInteger;
    }

    return str_to_ndecimals(s) > 0 ? FTDouble : FTString;
}

/* returns the field width */
int str_to_nwidth(const char *s, DBFFieldType eType)
{
    switch (eType)
    {
        case FTString:
        case FTInteger:
        case FTDouble:
            return (int)strlen(s);

        default:
            fprintf(stderr, "str_to_nwidth: unexpected type\n");
            exit(EXIT_FAILURE);
    }
}

/* returns true if f1 is more general than f2, otherwise false */
int more_general_field_type(DBFFieldType t1, DBFFieldType t2)
{
    if (FTInteger == t2 && t1 != FTInteger)
    {
        return 1;
    }

    if (FTDouble == t2 && FTString == t1)
    {
        return 1;
    }

    return 0;
}

void strip_crlf(char *line)
{
    /* remove trailing CR/LF */

    if (strchr(line, 0x0D))
    {
        char *pszline = strchr(line, 0x0D);
        pszline[0] = '\0';
    }

    if (strchr(line, 0x0A))
    {
        char *pszline = strchr(line, 0x0A);
        pszline[0] = '\0';
    }
}

static void IGNORE_FGETS_RET_VAL(char* s)
{
    (void)s;
}

int main(int argc, char **argv)
{
    printf("csv2shp version 1, Copyright (C) 2005 Springs Rescue Mission\n");

    if (4 != argc)
    {
        fprintf(stderr,
                "csv2shp comes with ABSOLUTELY NO WARRANTY; for details\n");
        fprintf(stderr,
                "see csv2shp.c.  This is free software, and you are welcome\n");
        fprintf(stderr,
                "to redistribute it under certain conditions; see csv2shp.c\n");
        fprintf(stderr, "for details\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "USAGE\n");
        fprintf(stderr,
                "csv2shp csv_filename delimiter_character shp_filename\n");
        fprintf(stderr, "   csv_filename\n");
        fprintf(stderr,
                "     columns named longitude and latitude must exist\n");
        fprintf(stderr, "   delimiter_character\n");
        fprintf(stderr, "     one character only\n");
        fprintf(stderr, "   shp_filename\n");
        fprintf(stderr, "     base name, do not give the extension\n");
        return EXIT_FAILURE;
    }

    if (strlen(argv[2]) > 1)
    {
        fprintf(stderr, "delimiter must be one character in length\n");
        return EXIT_FAILURE;
    }

    const char delimiter = argv[2][0];

    FILE *csv_f = fopen(argv[1], "r");
    if (NULL == csv_f)
    {
        perror("could not open csv file");
        exit(EXIT_FAILURE);
    }

    char sbuffer[4096];
    IGNORE_FGETS_RET_VAL(fgets(sbuffer, 4000, csv_f));

    /* check first row */

    strip_crlf(sbuffer);

    if (delimiter == sbuffer[strlen(sbuffer) - 1])
    {
        fprintf(stderr, "lines must not end with the delimiter character\n");
        return EXIT_FAILURE;
    }

    /* count columns and verify consistency*/

    /* 1-based */
    int n_columns = strnchr(sbuffer, delimiter);
    if (n_columns > MAX_COLUMNS)
    {
        fprintf(stderr, "too many columns, maximum is %i\n", MAX_COLUMNS);
        return EXIT_FAILURE;
    }

    int n_line = 1;
    while (!feof(csv_f))
    {
        n_line++;
        IGNORE_FGETS_RET_VAL(fgets(sbuffer, 4000, csv_f));
        if (n_columns != strnchr(sbuffer, delimiter))
        {
            fprintf(stderr,
                    "Number of columns on row %i does not match number of "
                    "columns on row 1\n",
                    n_columns);
            return EXIT_FAILURE;
        }
    }

    /* identify longitude and latitude columns */

    fseek(csv_f, 0, SEEK_SET);
    IGNORE_FGETS_RET_VAL(fgets(sbuffer, 4000, csv_f));
    strip_crlf(sbuffer);

    int n_longitude = -1; /* column with x, 0 based */
    int n_latitude = -1;  /* column with y, 0 based */
    for (int x = 0; x <= n_columns; x++)
    {
        if (0 ==
            STRCASECMP("Longitude", delimited_column(sbuffer, delimiter, x)))
        {
            n_longitude = x;
        }
        else if (0 == STRCASECMP("Latitude",
                                 delimited_column(sbuffer, delimiter, x)))
        {
            n_latitude = x;
        }
    }

#ifdef DEBUG
    printf("debug lat/long = %i/%i\n", n_latitude, n_longitude);
#endif

    if (-1 == n_longitude || -1 == n_latitude)
    {
        fprintf(stderr, "The header row must define one each a column named "
                        "longitude and latitude\n");
        return EXIT_FAILURE;
    }

    /* determine best fit for each column */

    printf("Analyzing column types...\n");

#ifdef DEBUG
    printf("debug: string type = %i\n", FTString);
    printf("debug: int type = %i\n", FTInteger);
    printf("debug: double type = %i\n", FTDouble);
#endif

    column columns[MAX_COLUMNS + 1];
    for (int x = 0; x <= n_columns; x++)
    {
#ifdef DEBUG
        printf("debug: examining column %i\n", x);
#endif
        columns[x].eType = FTInteger;
        columns[x].nWidth = 2;
        columns[x].nDecimals = 0;

        fseek(csv_f, 0, SEEK_SET);
        IGNORE_FGETS_RET_VAL(fgets(sbuffer, 4000, csv_f));
        strip_crlf(sbuffer);

        while (!feof(csv_f))
        {
#ifdef DEBUG
            printf("column %i, type = %i, w = %i, d = %i\n", x,
                   columns[x].eType, columns[x].nWidth, columns[x].nDecimals);
#endif
            if (NULL == fgets(sbuffer, 4000, csv_f))
            {
                if (!feof(csv_f))
                {
                    fprintf(stderr, "error during fgets()\n");
                }
                continue;
            }
            strip_crlf(sbuffer);

            char szfield[4096];
            strcpy(szfield, delimited_column(sbuffer, delimiter, x));
            if (more_general_field_type(str_to_fieldtype(szfield),
                                        columns[x].eType))
            {
                columns[x].eType = str_to_fieldtype(szfield);
                columns[x].nWidth = 2;
                columns[x].nDecimals = 0;
                fseek(csv_f, 0, SEEK_SET);
                IGNORE_FGETS_RET_VAL(fgets(sbuffer, 4000, csv_f));
                strip_crlf(sbuffer);
                continue;
            }
            if (columns[x].nWidth < str_to_nwidth(szfield, columns[x].eType))
            {
                columns[x].nWidth = str_to_nwidth(szfield, columns[x].eType);
            }
            if (FTDouble == columns[x].eType &&
                columns[x].nDecimals < str_to_ndecimals(szfield))
            {
                columns[x].nDecimals = str_to_ndecimals(szfield);
            }
        }
    }

    printf("Initializing output files...\n");

    // TODO(schwehr): Close csv_f, shp_h, and dbf_h before EXIT_FAILURE
    SHPHandle shp_h = SHPCreate(argv[3], SHPT_POINT);
    DBFHandle dbf_h = DBFCreate(argv[3]);
    if (NULL == dbf_h)
    {
        fprintf(stderr, "DBFCreate failed\n");
        exit(EXIT_FAILURE);
    }

    fseek(csv_f, 0, SEEK_SET);
    IGNORE_FGETS_RET_VAL(fgets(sbuffer, 4000, csv_f));
    strip_crlf(sbuffer);

    for (int x = 0; x <= n_columns; x++)
    {
#ifdef DEBUG
        printf(
            "debug: final: column %i, type = %i, w = %i, d = %i, name=|%s|\n",
            x, columns[x].eType, columns[x].nWidth, columns[x].nDecimals,
            delimited_column(sbuffer, delimiter, x));
#endif
        if (-1 == DBFAddField(dbf_h, delimited_column(sbuffer, delimiter, x),
                              columns[x].eType, columns[x].nWidth,
                              columns[x].nDecimals))
        {
            fprintf(stderr, "DBFFieldAdd failed column %i\n", x + 1);
            exit(EXIT_FAILURE);
        }
    }

    printf("Writing data...\n");

    fseek(csv_f, 0, SEEK_SET);
    IGNORE_FGETS_RET_VAL(fgets(sbuffer, 4000, csv_f)); /* skip header */
    strip_crlf(sbuffer);

    n_columns = strnchr(sbuffer, delimiter);
    n_line = 1;

    while (!feof(csv_f))
    {
        n_line++;
        IGNORE_FGETS_RET_VAL(fgets(sbuffer, 4000, csv_f));
        strip_crlf(sbuffer);

        /* write to shape file */
        double x_pt = atof(delimited_column(sbuffer, delimiter, n_longitude));
        double y_pt = atof(delimited_column(sbuffer, delimiter, n_latitude));

#ifdef DEBUG
        printf("debug: sbuffer=%s", sbuffer);
        printf("debug: x,y = %f, %f\n", x_pt, y_pt);
#endif

        SHPObject *shp =
            SHPCreateSimpleObject(SHPT_POINT, 1, &x_pt, &y_pt, NULL);
        const int shp_i = SHPWriteObject(shp_h, -1, shp);
        SHPDestroyObject(shp);

        /* write to dbf */

        for (int x = 0; x <= n_columns; x++)
        {
            char szfield[4096];
            strcpy(szfield, delimited_column(sbuffer, delimiter, x));

            int b;
            switch (columns[x].eType)
            {
                case FTInteger:
                    b = DBFWriteIntegerAttribute(dbf_h, shp_i, x,
                                                 atoi(szfield));
                    break;
                case FTDouble:
                    b = DBFWriteDoubleAttribute(dbf_h, shp_i, x, atof(szfield));
                    break;
                case FTString:
                    b = DBFWriteStringAttribute(dbf_h, shp_i, x, szfield);
                    break;
                default:
                    fprintf(stderr, "unexpected column type %i in column %i\n",
                            columns[x].eType, x);
            }

            if (!b)
            {
                fprintf(stderr, "DBFWrite*Attribute failed\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    fclose(csv_f);
    SHPClose(shp_h);
    DBFClose(dbf_h);

    return EXIT_SUCCESS;
}
