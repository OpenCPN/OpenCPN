/**********************************************************************
zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://www.zygrib.org

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#ifndef ZU_FILE_H
#define ZU_FILE_H

#ifdef __cplusplus
extern "C" {
#endif

// zuFile : uniform interface for READING uncompressed, gziped and bziped files
// (with fseek ftell not so bugged as in ... library)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <zlib.h>
#include <bzlib.h>

#define ZU_COMPRESS_AUTO  -1
#define ZU_COMPRESS_NONE   0
#define ZU_COMPRESS_GZIP   1
#define ZU_COMPRESS_BZIP   2

#define ZU_BUFREADSIZE   256000


typedef struct
{
    int   type;
    int   ok;
    char  *fname;
    long  pos;

    void *zfile;   // exact file type depends of compress type

    FILE *faux;   // auxiliary file for bzip
} ZUFILE;


ZUFILE * zu_open(const char *fname, const char *mode, int type=ZU_COMPRESS_AUTO);
int    zu_close(ZUFILE *f);

int    zu_can_read_file(const char *fname);

int    zu_read(ZUFILE *f, void *buf, long len);

long   zu_tell(ZUFILE *f);

int    zu_seek(ZUFILE *f, long offset, int whence);        // TODO: whence=SEEK_END

void   zu_rewind(ZUFILE *f);

long   zu_filesize(ZUFILE *f);

// for internal use :
int zu_bzSeekForward(ZUFILE *f, unsigned long nbytes);


#ifdef __cplusplus
}
#endif

#endif
