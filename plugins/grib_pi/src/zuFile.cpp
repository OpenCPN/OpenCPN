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

#include "zuFile.h"

//----------------------------------------------------
int    zu_can_read_file(const char *fname)
{
    ZUFILE *f;
    f = zu_open(fname, "rb");
    if (f == NULL) {
        return 0;
    }
    else {
        zu_close(f);
        return 1;
    }
}

//----------------------------------------------------
ZUFILE * zu_open(const char *fname, const char *mode, int type)
{
    ZUFILE *f;
    char buf[16];
    if (!fname || strlen(fname)==0) {
        return NULL;
    }
    f = (ZUFILE *) malloc(sizeof(ZUFILE));
    if (!f) {
        return NULL;
    }

    f->ok = 1;
    f->pos = 0;
    f->fname = strdup(fname);

	if (type == ZU_COMPRESS_AUTO)
	{
		char *p = strrchr(f->fname, '.');
		int  i=0;
		while (p!=NULL && *p !='\0' && i<4) {
			buf[i] = tolower(*p);
			i++;
			p++;
		}
		buf[i] = '\0';
		if (!strcmp(buf, ".gz")) {
			f->type = ZU_COMPRESS_GZIP;
			//fprintf(stderr,"ZU_COMPRESS_GZIP\n");
		}
		else if (!strcmp(buf, ".bz2") || !strcmp(buf, ".bz")) {
			f->type = ZU_COMPRESS_BZIP;
			//fprintf(stderr,"ZU_COMPRESS_BZIP\n");
		}
		else {
			f->type = ZU_COMPRESS_NONE;
			//fprintf(stderr,"ZU_COMPRESS_NONE\n");
		}
	}
	else
	{
		f->type = type;
	}
	
    switch(f->type) {
        case ZU_COMPRESS_NONE :
            f->zfile = (void *) fopen(f->fname, mode);
            break;
        case ZU_COMPRESS_GZIP :
            f->zfile = (void *) gzopen(f->fname, mode);
            break;
        case ZU_COMPRESS_BZIP :
            f->faux = fopen(f->fname, mode);
            if (f->faux) {
                int bzerror=BZ_OK;
                f->zfile = (void *) BZ2_bzReadOpen(&bzerror,f->faux,0,0,NULL,0);
                if (bzerror != BZ_OK) {
                    BZ2_bzReadClose (&bzerror,(BZFILE*)(f->zfile));
                    fclose(f->faux);
                    f->zfile = NULL;
                }
            } else {
                f->zfile = NULL;
            }
            break;
        default :
            f->zfile = NULL;
    }

    if (f->zfile == NULL) {
        free(f);
        f = NULL;
    }

    return f;
}
//----------------------------------------------------
int  zu_read(ZUFILE *f, void *buf, long len)
{
    int nb = 0;
    int bzerror=BZ_OK;
    switch(f->type) {
        case ZU_COMPRESS_NONE :
            nb = fread(buf, 1, len, (FILE*)(f->zfile));
            break;
        case ZU_COMPRESS_GZIP :
            nb = gzread((gzFile)(f->zfile), buf, len);
            break;
        case ZU_COMPRESS_BZIP :
            nb = BZ2_bzRead(&bzerror,(BZFILE*)(f->zfile), buf, len);
            break;
    }
    f->pos += nb;
    return nb;
}

//----------------------------------------------------
int zu_close(ZUFILE *f)
{
    int bzerror=BZ_OK;
    if (f) {
        f->ok = 0;
        f->pos = 0;
        free(f->fname);
        if (f->zfile) {
            switch(f->type) {
                case ZU_COMPRESS_NONE :
                    fclose((FILE*)(f->zfile));
                    break;
                case ZU_COMPRESS_GZIP :
                    gzclose((gzFile)(f->zfile));
                    break;
                case ZU_COMPRESS_BZIP :
                    BZ2_bzReadClose (&bzerror,(BZFILE*)(f->zfile));
                    if (f->faux) {
                        fclose(f->faux);
                    }
                    break;
            }
        }
    }
    return 0;
}



//----------------------------------------------------
long   zu_tell(ZUFILE *f)
{
    return f->pos;
}

//----------------------------------------------------
long   zu_filesize(ZUFILE *f)
{
    long res = 0;
    FILE *ftmp = fopen(f->fname, "rb");
    if (ftmp)
    {
        fseek(ftmp, 0, SEEK_END);
        res = ftell(ftmp);        
        fclose(ftmp);
    }
    return res;
}

//----------------------------------------------------
int zu_seek(ZUFILE *f, long offset, int whence)
{
    int res = 0;
    int bzerror=BZ_OK;
    if (whence == SEEK_END) {
        return -1;              // TODO
    }
    
    switch(f->type) {         //SEEK_SET, SEEK_CUR
        case ZU_COMPRESS_NONE :
            res = fseek((FILE*)(f->zfile), offset, whence);
            f->pos = ftell((FILE*)(f->zfile));
            break;
        case ZU_COMPRESS_GZIP :
            if (whence == SEEK_SET) {
                res = gzseek((gzFile)(f->zfile), offset, whence);
            }
            else {     // !!! BUG with SEEK_CUR in ZLIB !!!
                int p1 = gztell((gzFile)(f->zfile));
                res = gzseek((gzFile)(f->zfile), p1+offset, SEEK_SET);
            }
            f->pos = gztell((gzFile)(f->zfile));
            if (res >= 0)
                res = 0;
            break;
        case ZU_COMPRESS_BZIP :
            if (whence==SEEK_SET  &&  offset >= f->pos) {
                res = zu_bzSeekForward(f, offset-f->pos);
            }
            else if (whence==SEEK_CUR) {
                res = zu_bzSeekForward(f, offset);
            }
            else {    // BAD : reopen file
                BZ2_bzReadClose (&bzerror,(BZFILE*)(f->zfile));
                bzerror=BZ_OK;
                rewind(f->faux);
                f->pos = 0;
                f->zfile = (void *) BZ2_bzReadOpen(&bzerror,f->faux,0,0,NULL,0);
                if (bzerror != BZ_OK) {
                    BZ2_bzReadClose (&bzerror,(BZFILE*)(f->zfile));
                    fclose(f->faux);
                    f->zfile = NULL;
                    f->ok = 0;
                }
                res = zu_bzSeekForward(f, offset);
            }
            break;
    }
    return res;
}

//-----------------------------------------------------------------
int  zu_bzSeekForward(ZUFILE *f, unsigned long nbytes_)
// for internal use
{
    unsigned long nbytes = nbytes_;
    char buf[ZU_BUFREADSIZE];
    unsigned long nbread = 0;
    int nb;
    int bzerror=BZ_OK;
    while (bzerror==BZ_OK  &&  nbytes>=ZU_BUFREADSIZE) {
        nb = BZ2_bzRead(&bzerror,(BZFILE*)(f->zfile), buf, ZU_BUFREADSIZE);
        nbytes -= nb;
        nbread += nb;
    }
    if (bzerror==BZ_OK  &&  nbytes>0) {
        nb = BZ2_bzRead(&bzerror,(BZFILE*)(f->zfile), buf, nbytes);
        nbread += nb;
    }
    f->pos += nbread;

    return nbread==nbytes_ ? 0 : -1;
}

//-----------------------------------------------------------------
void   zu_rewind(ZUFILE *f)
{
    zu_seek(f, 0, SEEK_SET);
}


