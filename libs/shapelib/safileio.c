/******************************************************************************
 *
 * Project:  Shapelib
 * Purpose:  Default implementation of file io based on stdio.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2007, Frank Warmerdam
 *
 * SPDX-License-Identifier: MIT OR LGPL-2.0-or-later
 ******************************************************************************
 *
 */

#include "shapefil.h"

#include <assert.h>
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SHPAPI_UTF8_HOOKS
#ifdef SHPAPI_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#pragma comment(lib, "kernel32.lib")
#endif
#endif

static SAFile SADFOpen(const char *pszFilename, const char *pszAccess,
                       void *pvUserData)
{
    (void)pvUserData;
    return (SAFile)fopen(pszFilename, pszAccess);
}

static SAOffset SADFRead(void *p, SAOffset size, SAOffset nmemb, SAFile file)
{
    return (SAOffset)fread(p, (size_t)size, (size_t)nmemb, (FILE *)file);
}

static SAOffset SADFWrite(const void *p, SAOffset size, SAOffset nmemb,
                          SAFile file)
{
    return (SAOffset)fwrite(p, (size_t)size, (size_t)nmemb, (FILE *)file);
}

static SAOffset SADFSeek(SAFile file, SAOffset offset, int whence)
{
#if defined(_MSC_VER) && _MSC_VER >= 1400
    return (SAOffset)_fseeki64((FILE *)file, (__int64)offset, whence);
#else
    return (SAOffset)fseek((FILE *)file, (long)offset, whence);
#endif
}

static SAOffset SADFTell(SAFile file)
{
#if defined(_MSC_VER) && _MSC_VER >= 1400
    return (SAOffset)_ftelli64((FILE *)file);
#else
    return (SAOffset)ftell((FILE *)file);
#endif
}

static int SADFFlush(SAFile file)
{
    return fflush((FILE *)file);
}

static int SADFClose(SAFile file)
{
    return fclose((FILE *)file);
}

static int SADRemove(const char *filename, void *pvUserData)
{
    (void)pvUserData;
    return remove(filename);
}

static void SADError(const char *message)
{
    fprintf(stderr, "%s\n", message);
}

void SASetupDefaultHooks(SAHooks *psHooks)
{
    psHooks->FOpen = SADFOpen;
    psHooks->FRead = SADFRead;
    psHooks->FWrite = SADFWrite;
    psHooks->FSeek = SADFSeek;
    psHooks->FTell = SADFTell;
    psHooks->FFlush = SADFFlush;
    psHooks->FClose = SADFClose;
    psHooks->Remove = SADRemove;

    psHooks->Error = SADError;
    psHooks->Atof = atof;
    psHooks->pvUserData = NULL;
}

#ifdef SHPAPI_WINDOWS

static wchar_t *Utf8ToWideChar(const char *pszFilename)
{
    const int nMulti = (int)strlen(pszFilename) + 1;
    const int nWide =
        MultiByteToWideChar(CP_UTF8, 0, pszFilename, nMulti, 0, 0);
    if (nWide == 0)
    {
        return NULL;
    }
    wchar_t *pwszFileName = (wchar_t *)malloc(nWide * sizeof(wchar_t));
    if (pwszFileName == NULL)
    {
        return NULL;
    }
    if (MultiByteToWideChar(CP_UTF8, 0, pszFilename, nMulti, pwszFileName,
                            nWide) == 0)
    {
        free(pwszFileName);
        return NULL;
    }
    return pwszFileName;
}

/************************************************************************/
/*                           SAUtf8WFOpen                               */
/************************************************************************/

SAFile SAUtf8WFOpen(const char *pszFilename, const char *pszAccess)
{
    SAFile file = NULL;
    wchar_t *pwszFileName = Utf8ToWideChar(pszFilename);
    wchar_t *pwszAccess = Utf8ToWideChar(pszAccess);
    if (pwszFileName != NULL && pwszAccess != NULL)
    {
        file = (SAFile)_wfopen(pwszFileName, pwszAccess);
    }
    free(pwszFileName);
    free(pwszAccess);
    return file;
}

int SAUtf8WRemove(const char *pszFilename)
{
    wchar_t *pwszFileName = Utf8ToWideChar(pszFilename);
    int rc = -1;
    if (pwszFileName != NULL)
    {
        rc = _wremove(pwszFileName);
    }
    free(pwszFileName);
    return rc;
}

#endif

#ifdef SHPAPI_UTF8_HOOKS
#ifndef SHPAPI_WINDOWS
#error "no implementations of UTF-8 hooks available for this platform"
#endif

void SASetupUtf8Hooks(SAHooks *psHooks)
{
    psHooks->FOpen = SAUtf8WFOpen;
    psHooks->Remove = SAUtf8WRemove;
    psHooks->FRead = SADFRead;
    psHooks->FWrite = SADFWrite;
    psHooks->FSeek = SADFSeek;
    psHooks->FTell = SADFTell;
    psHooks->FFlush = SADFFlush;
    psHooks->FClose = SADFClose;

    psHooks->Error = SADError;
    psHooks->Atof = atof;
}
#endif
