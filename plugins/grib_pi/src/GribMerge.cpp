/***************************************************************************
 *   Copyright (C) 2026 by the OpenCPN Authors                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "GribMerge.h"

#include <wx/file.h>
#include <wx/filefn.h>
#include <wx/filename.h>

#include <algorithm>
#include <limits>

namespace {

const wxFileOffset MAX_GRIB_MERGE_FILE_SIZE = 512LL * 1024LL * 1024LL;

uint32_t ReadUint24(const std::vector<unsigned char> &data, size_t pos) {
  return ((uint32_t)data[pos] << 16) | ((uint32_t)data[pos + 1] << 8) |
         (uint32_t)data[pos + 2];
}

uint32_t ReadUint16(const std::vector<unsigned char> &data, size_t pos) {
  return ((uint32_t)data[pos] << 8) | (uint32_t)data[pos + 1];
}

uint32_t ReadUint32(const std::vector<unsigned char> &data, size_t pos) {
  return ((uint32_t)data[pos] << 24) | ((uint32_t)data[pos + 1] << 16) |
         ((uint32_t)data[pos + 2] << 8) | (uint32_t)data[pos + 3];
}

uint64_t ReadUint64(const std::vector<unsigned char> &data, size_t pos) {
  uint64_t value = 0;
  for (int i = 0; i < 8; i++) value = (value << 8) | data[pos + i];
  return value;
}

int32_t ReadInt32(const std::vector<unsigned char> &data, size_t pos) {
  return (int32_t)ReadUint32(data, pos);
}

int32_t ReadInt24Grib1(const std::vector<unsigned char> &data, size_t pos) {
  uint32_t value = ReadUint24(data, pos);
  if (value & 0x800000) return -(int32_t)(value & 0x7fffff);
  return (int32_t)value;
}

double NormaliseLon(double lon) {
  while (lon > 180.0) lon -= 360.0;
  while (lon < -180.0) lon += 360.0;
  return lon;
}

bool IsGribAt(const std::vector<unsigned char> &data, size_t pos) {
  return pos + 4 <= data.size() && data[pos] == 'G' && data[pos + 1] == 'R' &&
         data[pos + 2] == 'I' && data[pos + 3] == 'B';
}

bool EndsWith7777(const std::vector<unsigned char> &data, size_t end) {
  return end >= 4 && data[end - 4] == '7' && data[end - 3] == '7' &&
         data[end - 2] == '7' && data[end - 1] == '7';
}

bool AddTimeDelta(wxDateTime &time, int unit, uint32_t value) {
  switch (unit) {
    case 0:
      time += wxTimeSpan::Minutes(value);
      return true;
    case 1:
      time += wxTimeSpan::Hours(value);
      return true;
    case 2:
      time += wxTimeSpan::Days(value);
      return true;
    case 10:
      time += wxTimeSpan::Hours(3 * value);
      return true;
    case 11:
      time += wxTimeSpan::Hours(6 * value);
      return true;
    case 12:
      time += wxTimeSpan::Hours(12 * value);
      return true;
    case 13:
      time += wxTimeSpan::Seconds(value);
      return true;
    default:
      return false;
  }
}

bool SetUtcTime(wxDateTime &time, int year, int month, int day, int hour,
                int minute, int second) {
  if (month < 1 || month > 12) return false;

  time.Set((wxDateTime::wxDateTime_t)day, (wxDateTime::Month)(month - 1),
           year, hour, minute, second);
  if (!time.IsValid()) return false;

  time.MakeFromTimezone(wxDateTime::UTC);
  return true;
}

void AddValidTime(GribMergeSummary &summary, const wxDateTime &time) {
  if (!time.IsValid()) return;
  if (!summary.hasValidTimes) {
    summary.firstValidTime = time;
    summary.lastValidTime = time;
    summary.hasValidTimes = true;
    return;
  }
  if (time < summary.firstValidTime) summary.firstValidTime = time;
  if (time > summary.lastValidTime) summary.lastValidTime = time;
}

void AddCoverage(GribMergeSummary &summary, double lat1, double lon1,
                 double lat2, double lon2) {
  double latMin = std::min(lat1, lat2);
  double latMax = std::max(lat1, lat2);
  double lonMin = std::min(lon1, lon2);
  double lonMax = std::max(lon1, lon2);

  if (!summary.hasCoverage) {
    summary.latMin = latMin;
    summary.latMax = latMax;
    summary.lonMin = lonMin;
    summary.lonMax = lonMax;
    summary.hasCoverage = true;
    return;
  }

  summary.latMin = std::min(summary.latMin, latMin);
  summary.latMax = std::max(summary.latMax, latMax);
  summary.lonMin = std::min(summary.lonMin, lonMin);
  summary.lonMax = std::max(summary.lonMax, lonMax);
}

void ParseGrib1Meta(const std::vector<unsigned char> &data,
                    const GribMergeMessage &msg, GribMergeSummary &summary) {
  size_t pdsStart = msg.start + 8;
  if (pdsStart + 3 > msg.start + msg.length) return;

  uint32_t pdsLength = ReadUint24(data, pdsStart);
  if (pdsLength < 25 || pdsStart + pdsLength > msg.start + msg.length) return;

  unsigned char flags = data[pdsStart + 7];
  int year = data[pdsStart + 12];
  int month = data[pdsStart + 13];
  int day = data[pdsStart + 14];
  int hour = data[pdsStart + 15];
  int minute = data[pdsStart + 16];
  int timeUnit = data[pdsStart + 17];
  int p1 = data[pdsStart + 18];
  int p2 = data[pdsStart + 19];
  int timeRange = data[pdsStart + 20];
  int century = data[pdsStart + 24];
  int fullYear = (century - 1) * 100 + year;

  wxDateTime validTime;
  if (SetUtcTime(validTime, fullYear, month, day, hour, minute, 0)) {
    uint32_t step = ((timeRange >= 2 && timeRange <= 5) && p2) ? p2 : p1;
    AddTimeDelta(validTime, timeUnit, step);
    AddValidTime(summary, validTime);
  }

  if (!(flags & 0x80)) return;

  size_t gdsStart = pdsStart + pdsLength;
  if (gdsStart + 28 > msg.start + msg.length) return;

  uint32_t gdsLength = ReadUint24(data, gdsStart);
  if (gdsLength < 28 || gdsStart + gdsLength > msg.start + msg.length) return;

  if (data[gdsStart + 5] == 0) {
    double lat1 = ReadInt24Grib1(data, gdsStart + 10) / 1000.0;
    double lon1 = NormaliseLon(ReadInt24Grib1(data, gdsStart + 13) / 1000.0);
    double lat2 = ReadInt24Grib1(data, gdsStart + 17) / 1000.0;
    double lon2 = NormaliseLon(ReadInt24Grib1(data, gdsStart + 20) / 1000.0);
    AddCoverage(summary, lat1, lon1, lat2, lon2);
  }
}

void ParseGrib2Meta(const std::vector<unsigned char> &data,
                    const GribMergeMessage &msg, GribMergeSummary &summary) {
  size_t end = msg.start + msg.length;
  size_t pos = msg.start + 16;
  wxDateTime refTime;

  while (pos + 5 <= end - 4) {
    uint32_t sectionLength = ReadUint32(data, pos);
    if (sectionLength < 5 || pos + sectionLength > end) return;

    int sectionNumber = data[pos + 4];
    if (sectionNumber == 1 && sectionLength >= 21) {
      int year = ReadUint16(data, pos + 12);
      int month = data[pos + 14];
      int day = data[pos + 15];
      int hour = data[pos + 16];
      int minute = data[pos + 17];
      int second = data[pos + 18];
      SetUtcTime(refTime, year, month, day, hour, minute, second);
    } else if (sectionNumber == 3 && sectionLength >= 72) {
      uint32_t gridTemplate = ReadUint16(data, pos + 12);
      if (gridTemplate == 0) {
        uint32_t basicAngle = ReadUint32(data, pos + 38);
        uint32_t subdivisions = ReadUint32(data, pos + 42);
        double scale =
            (basicAngle == 0 || subdivisions == 0)
                ? 1e-6
                : (double)basicAngle / (double)subdivisions;
        double lat1 = ReadInt32(data, pos + 46) * scale;
        double lon1 = NormaliseLon(ReadInt32(data, pos + 50) * scale);
        double lat2 = ReadInt32(data, pos + 55) * scale;
        double lon2 = NormaliseLon(ReadInt32(data, pos + 59) * scale);
        AddCoverage(summary, lat1, lon1, lat2, lon2);
      }
    } else if (sectionNumber == 4 && sectionLength >= 22 &&
               refTime.IsValid()) {
      wxDateTime validTime = refTime;
      int timeUnit = data[pos + 17];
      uint32_t forecastTime = ReadUint32(data, pos + 18);
      if (AddTimeDelta(validTime, timeUnit, forecastTime))
        AddValidTime(summary, validTime);
    }

    pos += sectionLength;
  }
}

bool ScanMessages(const wxString &path, const std::vector<unsigned char> &data,
                  std::vector<GribMergeMessage> &messages, wxString &error) {
  size_t pos = 0;

  while (pos < data.size()) {
    size_t start = data.size();
    for (size_t i = pos; i + 4 <= data.size(); i++) {
      if (IsGribAt(data, i)) {
        start = i;
        break;
      }
    }

    if (start == data.size()) break;

    if (start + 8 > data.size()) {
      error.Printf(_("%s: truncated GRIB header at byte %llu"),
                   wxFileName(path).GetFullName(), (unsigned long long)start);
      return false;
    }

    int edition = data[start + 7];
    uint64_t length = 0;
    if (edition == 1) {
      length = ReadUint24(data, start + 4);
    } else if (edition == 2) {
      if (start + 16 > data.size()) {
        error.Printf(_("%s: truncated GRIB2 header at byte %llu"),
                     wxFileName(path).GetFullName(),
                     (unsigned long long)start);
        return false;
      }
      length = ReadUint64(data, start + 8);
    } else {
      error.Printf(_("%s: unsupported GRIB edition %d at byte %llu"),
                   wxFileName(path).GetFullName(), edition,
                   (unsigned long long)start);
      return false;
    }

    if (length == 0 || length > std::numeric_limits<size_t>::max()) {
      error.Printf(_("%s: invalid GRIB message length at byte %llu"),
                   wxFileName(path).GetFullName(), (unsigned long long)start);
      return false;
    }

    size_t end = start + (size_t)length;
    if (end < start || end > data.size()) {
      error.Printf(_("%s: GRIB message at byte %llu is truncated"),
                   wxFileName(path).GetFullName(), (unsigned long long)start);
      return false;
    }

    if (!EndsWith7777(data, end)) {
      error.Printf(_("%s: GRIB message at byte %llu does not end with 7777"),
                   wxFileName(path).GetFullName(), (unsigned long long)start);
      return false;
    }

    GribMergeMessage msg;
    msg.start = start;
    msg.length = (size_t)length;
    msg.edition = edition;
    messages.push_back(msg);
    pos = end;
  }

  if (messages.empty()) {
    error.Printf(_("%s: no GRIB messages were found"),
                 wxFileName(path).GetFullName());
    return false;
  }

  return true;
}

wxString FormatTime(const wxDateTime &time) {
  return time.Format("%Y-%m-%d %H:%M", wxDateTime::UTC) + " UTC";
}

}  // namespace

bool GribMerge::ReadAndValidate(const wxString &path, GribMergeFile &file,
                                wxString &error) {
  wxFile input(path);
  if (!input.IsOpened()) {
    error.Printf(_("Unable to open %s"), path);
    return false;
  }

  wxFileOffset length = input.Length();
  if (length <= 0) {
    error = _("The selected file is empty.");
    return false;
  }

  if (length > MAX_GRIB_MERGE_FILE_SIZE) {
    error = _("The selected file is too large to process.");
    return false;
  }

  file.path = path;
  file.data.assign((size_t)length, 0);
  if (input.Read(file.data.data(), (size_t)length) != length) {
    error.Printf(_("Unable to read %s"), path);
    return false;
  }

  file.messages.clear();
  if (!ScanMessages(path, file.data, file.messages, error)) return false;

  GribMergeSummary summary;
  summary.fileName = wxFileName(path).GetFullName();
  summary.fileSize = file.data.size();
  summary.messageCount = file.messages.size();
  summary.grib1Count = 0;
  summary.grib2Count = 0;
  summary.hasValidTimes = false;
  summary.hasCoverage = false;
  summary.latMin = summary.latMax = summary.lonMin = summary.lonMax = 0.0;

  for (size_t i = 0; i < file.messages.size(); i++) {
    const GribMergeMessage &msg = file.messages[i];
    if (msg.edition == 1) {
      summary.grib1Count++;
      ParseGrib1Meta(file.data, msg, summary);
    } else if (msg.edition == 2) {
      summary.grib2Count++;
      ParseGrib2Meta(file.data, msg, summary);
    }
  }

  file.summary = summary;
  return true;
}

bool GribMerge::WriteMerged(const GribMergeFile &first,
                            const GribMergeFile &second,
                            const wxString &outputPath, wxString &error) {
  wxFileName outputName(outputPath);
  wxString tempPrefix = outputName.GetPathWithSep() + "." +
                        outputName.GetFullName() + ".tmp";
  wxString tempPath = wxFileName::CreateTempFileName(tempPrefix);

  if (tempPath.IsEmpty()) {
    error.Printf(_("Unable to create a temporary file for %s"), outputPath);
    return false;
  }

  wxFile output(tempPath, wxFile::write);
  if (!output.IsOpened()) {
    wxRemoveFile(tempPath);
    error.Printf(_("Unable to create a temporary file for %s"), outputPath);
    return false;
  }

  const GribMergeFile *files[2] = {&first, &second};
  for (int fileIndex = 0; fileIndex < 2; fileIndex++) {
    const GribMergeFile &file = *files[fileIndex];
    for (size_t i = 0; i < file.messages.size(); i++) {
      const GribMergeMessage &msg = file.messages[i];
      const unsigned char *buffer = file.data.data() + msg.start;
      if (output.Write(buffer, msg.length) != msg.length) {
        output.Close();
        wxRemoveFile(tempPath);
        error.Printf(_("Unable to write %s"), outputPath);
        return false;
      }
    }
  }

  if (!output.Close()) {
    wxRemoveFile(tempPath);
    error.Printf(_("Unable to write %s"), outputPath);
    return false;
  }

  if (!wxRenameFile(tempPath, outputPath, true)) {
    wxRemoveFile(tempPath);
    error.Printf(_("Unable to replace %s"), outputPath);
    return false;
  }

  return true;
}

wxString GribMerge::FormatSummary(const wxString &label,
                                  const GribMergeSummary &summary) {
  wxString text;
  text << label << "\n";
  text << wxString::Format(_("  File: %s\n"), summary.fileName);
  text << wxString::Format(_("  Messages: %llu\n"),
                           (unsigned long long)summary.messageCount);
  text << wxString::Format(_("  Editions: GRIB1: %llu, GRIB2: %llu\n"),
                           (unsigned long long)summary.grib1Count,
                           (unsigned long long)summary.grib2Count);

  if (summary.hasValidTimes) {
    text << wxString::Format(_("  Valid time: %s to %s\n"),
                             FormatTime(summary.firstValidTime),
                             FormatTime(summary.lastValidTime));
  } else {
    text << _("  Valid time: unavailable\n");
  }

  if (summary.hasCoverage) {
    text << wxString::Format(_("  Coverage: lat %.4f to %.4f, lon %.4f to %.4f\n"),
                             summary.latMin, summary.latMax, summary.lonMin,
                             summary.lonMax);
  } else {
    text << _("  Coverage: unavailable\n");
  }

  return text;
}

wxString GribMerge::FormatOverlap(const GribMergeSummary &current,
                                  const GribMergeSummary &weather) {
  wxString text;
  text << _("Wind and ocean current data overlap:") << "\n";

  if (!current.hasValidTimes || !weather.hasValidTimes) {
    text << _("unavailable") << "\n";
    text << _("Outside this overlap, the merged file may contain weather data "
              "without ocean current data.");
    return text;
  }

  wxDateTime start =
      current.firstValidTime > weather.firstValidTime ? current.firstValidTime
                                                      : weather.firstValidTime;
  wxDateTime end =
      current.lastValidTime < weather.lastValidTime ? current.lastValidTime
                                                    : weather.lastValidTime;

  if (start <= end) {
    text << wxString::Format(_("%s to %s"), FormatTime(start),
                             FormatTime(end));
  } else {
    text << _("none");
  }

  text << "\n";
  text << _("Outside this overlap, the merged file may contain weather data "
            "without ocean current data.");
  return text;
}
