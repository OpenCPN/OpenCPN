/***************************************************************************
 *   Copyright (C) 2026 by the OpenCPN Authors                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef __GRIBMERGE_H__
#define __GRIBMERGE_H__

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <stdint.h>

#include <vector>

struct GribMergeMessage {
  size_t start;
  size_t length;
  int edition;
};

struct GribMergeSummary {
  wxString fileName;
  size_t fileSize;
  size_t messageCount;
  size_t grib1Count;
  size_t grib2Count;
  bool hasValidTimes;
  wxDateTime firstValidTime;
  wxDateTime lastValidTime;
  bool hasCoverage;
  double latMin;
  double latMax;
  double lonMin;
  double lonMax;
};

struct GribMergeFile {
  wxString path;
  std::vector<unsigned char> data;
  std::vector<GribMergeMessage> messages;
  GribMergeSummary summary;
};

class GribMerge {
public:
  static bool ReadAndValidate(const wxString &path, GribMergeFile &file,
                              wxString &error);
  static bool WriteMerged(const GribMergeFile &first,
                          const GribMergeFile &second,
                          const wxString &outputPath, wxString &error);
  static wxString FormatSummary(const wxString &label,
                                const GribMergeSummary &summary);
  static wxString FormatOverlap(const GribMergeSummary &current,
                                const GribMergeSummary &weather);
};

#endif
