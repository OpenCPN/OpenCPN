/////////////////////////////////////////////////////////////////////////////
// Name:        base64.h
// Purpose:     Base64 encoding function for HTTP Authentication
//                  Code originated from PHP.net source
// Author:      Angelo Mandato
// Created:     2005/08/10
// RCS-ID:      $Id: base64.h,v 1.2 2005/08/12 03:58:08 amandato Exp $
// Copyright:   (c) 2005 Angelo Mandato (http://www.spaceblue.com)
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_BASE64_H_
#define _WX_BASE64_H_

// optimization for GCC
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "base64.h"
#endif

#include <wx/string.h>

static constexpr char base64_table[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '\0'};

static constexpr char base64_pad = '=';

static constexpr short base64_reverse_table[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, -1, -1, -1, -1, -1, -1, -1, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1,
    -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
    43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1};

//! Returns string base64 encoded
// Based on code from PHP library:
WXDLLIMPEXP_HTTPENGINE wxString wxBase64Encode(const wxString &str) {
  wxString szToReturn;
  int length = str.Length();
  int current = 0;

  while (length >
         2)  // keep going until we have less than 24 bits (each item is 8 bits)
  {
    szToReturn.Append(base64_table[str.GetChar(current) >> 2]);
    szToReturn.Append(base64_table[((str.GetChar(current) & 0x03) << 4) +
                                   (str.GetChar(current + 1) >> 4)]);
    szToReturn.Append(base64_table[((str.GetChar(current + 1) & 0x0f) << 2) +
                                   (str.GetChar(current + 2) >> 6)]);
    szToReturn.Append(base64_table[str.GetChar(current + 2) & 0x3f]);

    current += 3;
    length -= 3;  // we just handle 3 octets of data
  }

  // now deal with the tail end of things
  if (length != 0) {
    szToReturn.Append(base64_table[str.GetChar(current) >> 2]);

    //*p++ = base64_table[current[0] >> 2];
    if (length > 1) {
      szToReturn.Append(base64_table[((str.GetChar(current) & 0x03) << 4) +
                                     (str.GetChar(current + 1) >> 4)]);
      szToReturn.Append(base64_table[(str.GetChar(current + 1) & 0x0f) << 2]);
      szToReturn.Append(base64_pad);
    } else {
      szToReturn.Append(base64_table[(str.GetChar(current) & 0x03) << 4]);
      szToReturn.Append(base64_pad);
      szToReturn.Append(base64_pad);
    }
  }

  return szToReturn;
}

//! Returns string base64 decoded
// Based on code from PHP library:
WXDLLIMPEXP_HTTPENGINE wxString wxBase64Decode(const wxString &str) {
  wxString szToReturn;

  int length = str.Length();
  unsigned int current = 0;
  int i = 0, j = 0, k;
  wxChar ch = ' ';

  while (current != str.Length() && length-- > 0) {
    ch = str.GetChar(current++);

    if (ch == base64_pad) break;

    //   When Base64 gets POSTed, all pluses are interpreted as spaces.
    //   This line changes them back.  It's not exactly the Base64 spec,
    //   but it is completely compatible with it (the spec says that
    //   spaces are invalid).  This will also save many people considerable
    //   headache.  - Turadg Aleahmad <turadg@wise.berkeley.edu>

    if (ch == ' ') ch = '+';

    ch = base64_reverse_table[(int)ch];  // CHECK

    switch (i % 4) {
      case 0:

        szToReturn.Append(ch << 2);
        break;
      case 1:
        szToReturn.SetChar(j, szToReturn.GetChar(j) | ch >> 4);
        j++;
        szToReturn.Append((ch & 0x0f) << 4);
        break;
      case 2:
        szToReturn.SetChar(j, szToReturn.GetChar(j) | ch >> 2);
        j++;
        szToReturn.Append((ch & 0x03) << 6);
        break;
      case 3:
        szToReturn.SetChar(j, szToReturn.GetChar(j) | ch);
        j++;
        break;
    }
    i++;
  }

  k = j;
  // mop things up if we ended on a boundary
  if (ch == base64_pad) {
    switch (i % 4) {
      case 0:
      case 1:
        return wxEmptyString;
    }
  }

  return szToReturn;
}

#endif
