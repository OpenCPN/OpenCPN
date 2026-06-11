/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_pi.h"

#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <memory>

#ifdef DLDR_USE_LIBARCHIVE
#include <archive.h>
#include <archive_entry.h>
#ifdef CHARTDLDR_RAR_UNARR
#include "unarr.h"
#endif
#else
#include "unarr.h"
#endif

#ifdef __ANDROID__
#include "androidSupport.h"
#endif

#ifdef _UNIX
#include <clocale>
#endif

namespace {

static bool IsPathInsideDir(const wxString &targetDir,
                            const wxString &entryName, wxString &outFullPath) {
  // Construct the full path
  wxString combinedPath = targetDir;
  if (!combinedPath.EndsWith(wxFileName::GetPathSeparator())) {
    combinedPath += wxFileName::GetPathSeparator();
  }
  combinedPath += entryName;

  // Normalize the combined path to resolve any ".." components
  wxFileName fn(combinedPath);
  fn.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG);
  outFullPath = fn.GetFullPath();

  // Normalize target dir for comparison
  wxFileName targetFn(targetDir);
  targetFn.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_ABSOLUTE |
                     wxPATH_NORM_LONG);
  wxString normalizedTarget = targetFn.GetFullPath();

  // Ensure target ends with separator for proper prefix matching
  if (!normalizedTarget.EndsWith(wxFileName::GetPathSeparator())) {
    normalizedTarget += wxFileName::GetPathSeparator();
  }

  // Check if the normalized path starts with the target directory
  // This catches all path traversal attempts including "../", absolute paths,
  // etc.
  if (outFullPath.StartsWith(normalizedTarget)) {
    return true;
  }

  // Also allow if it's exactly the target directory (for directory entries)
  if (outFullPath == targetFn.GetFullPath()) {
    return true;
  }

  return false;
}

#ifdef DLDR_USE_LIBARCHIVE
#ifndef __ANDROID__
static int copy_data(struct archive *ar, struct archive *aw) {
  int r;
  const void *buff;
  size_t size;
  __LA_INT64_T offset;

  for (;;) {
    r = archive_read_data_block(ar, &buff, &size, &offset);
    if (r == ARCHIVE_EOF) return (ARCHIVE_OK);
    if (r < ARCHIVE_OK) return (r);
    r = archive_write_data_block(aw, buff, size, offset);
    if (r < ARCHIVE_OK) {
      // fprintf(stderr, "%s\n", archive_error_string(aw));
      wxLogError(wxString::Format("Chartdldr_pi: LibArchive error: %s",
                                  archive_error_string(aw)));
      return (r);
    }
  }
}
#endif

}  // namespace

static void ChartDldrLogExtractStart(const wxString &archive_file,
                                     bool verbose_extract_log) {
  if (verbose_extract_log) {
    wxLogMessage(_T("chartdldr_pi: Going to extract '") + archive_file +
                 _T("'."));
  }
}

bool chartdldr_pi::ExtractLibArchiveFiles(const wxString &aArchiveFile,
                                          const wxString &aTargetDir,
                                          bool aStripPath, wxDateTime aMTime,
                                          bool aRemoveArchive,
                                          bool verbose_extract_log) {
#ifndef __ANDROID__
  ChartDldrLogExtractStart(aArchiveFile, verbose_extract_log);
  struct archive *a = NULL;
  struct archive *ext = NULL;
  bool ok = false;

  int flags = ARCHIVE_EXTRACT_TIME;
#ifdef ARCHIVE_EXTRACT_SECURE_NODOTDOT
  flags |= ARCHIVE_EXTRACT_SECURE_NODOTDOT;
#endif
#ifdef ARCHIVE_EXTRACT_SECURE_SYMLINKS
  flags |= ARCHIVE_EXTRACT_SECURE_SYMLINKS;
#endif

  a = archive_read_new();
  ext = archive_write_disk_new();

  if (!a || !ext) {
    wxLogError(_T("Chartdldr_pi: Failed to create libarchive objects."));
    goto cleanup;
  }

  archive_read_support_format_all(a);
  archive_read_support_filter_all(a);
#if !defined(__clang__)
  archive_read_support_compression_all(a);
#endif

  archive_write_disk_set_options(ext, flags);
  archive_write_disk_set_standard_lookup(ext);

#ifdef _WIN32
  if (archive_read_open_filename_w(a, aArchiveFile.wc_str(), 10240) !=
      ARCHIVE_OK) {
    wxLogError(wxString::Format("Chartdldr_pi: LibArchive open error: %s",
                                archive_error_string(a)));
    goto cleanup;
  }
#else
  {
    if (archive_read_open_filename(a, aArchiveFile.mb_str().data(), 10240) !=
        ARCHIVE_OK) {
      wxLogError(wxString::Format("Chartdldr_pi: LibArchive open error: %s",
                                  archive_error_string(a)));
      goto cleanup;
    }
  }
#endif

  for (;;) {
    struct archive_entry *entry = NULL;
    int r = archive_read_next_header(a, &entry);

    if (r == ARCHIVE_EOF) {
      break;
    }

    if (r < ARCHIVE_OK) {
      wxLogError(wxString::Format("Chartdldr_pi: LibArchive error: %s",
                                  archive_error_string(a)));
    }
    if (r < ARCHIVE_WARN) {
      goto cleanup;
    }

    wxString entryName;
#ifdef _WIN32
    const char *rawUtf8 = archive_entry_pathname_utf8(entry);
    if (rawUtf8 && *rawUtf8) {
      entryName = wxString::FromUTF8(rawUtf8);
    } else {
      const wchar_t *rawWide = archive_entry_pathname_w(entry);
      if (rawWide && *rawWide) entryName = wxString(rawWide);
    }
#else
    const char *rawPath = archive_entry_pathname(entry);
    if (rawPath && *rawPath) {
      entryName = wxString::FromUTF8(rawPath);
      if (entryName.IsEmpty()) {
        entryName = wxString::From8BitData(rawPath);
      }
    }
#endif

    if (entryName.IsEmpty()) {
      wxLogWarning(_T("Skipping archive entry with empty pathname."));
      continue;
    }

    if (aStripPath) {
      wxFileName stripped(entryName);
      entryName = stripped.GetFullName();

      if (entryName.IsEmpty()) {
        continue;
      }
    }

    wxString outputPath = entryName;
    if (aTargetDir != wxEmptyString) {
      if (!IsPathInsideDir(aTargetDir, entryName, outputPath)) {
        wxLogWarning(
            _T("Skipping archive entry with path traversal attempt: ") +
            entryName);
        continue;
      }
    }

#ifdef _WIN32
    archive_entry_copy_pathname_w(entry, outputPath.wc_str());
#else
    archive_entry_copy_pathname(entry, outputPath.fn_str().data());
#endif

    if (aMTime.IsValid()) {
      archive_entry_set_mtime(entry, static_cast<time_t>(aMTime.GetTicks()), 0);
    }

    r = archive_write_header(ext, entry);
    if (r < ARCHIVE_OK) {
      wxLogError(wxString::Format("Chartdldr_pi: LibArchive error: %s",
                                  archive_error_string(ext)));
    }
    if (r < ARCHIVE_WARN) {
      goto cleanup;
    }

    if (archive_entry_size(entry) > 0) {
      r = copy_data(a, ext);
      if (r < ARCHIVE_OK) {
        wxLogError(wxString::Format("Chartdldr_pi: LibArchive error: %s",
                                    archive_error_string(ext)));
      }
      if (r < ARCHIVE_WARN) {
        goto cleanup;
      }
    }

    r = archive_write_finish_entry(ext);
    if (r < ARCHIVE_OK) {
      wxLogError(wxString::Format("Chartdldr_pi: LibArchive error: %s",
                                  archive_error_string(ext)));
    }
    if (r < ARCHIVE_WARN) {
      goto cleanup;
    }
  }

  ok = true;

cleanup:
  if (a) {
    archive_read_close(a);
    archive_read_free(a);
  }
  if (ext) {
    archive_write_close(ext);
    archive_write_free(ext);
  }

  if (ok && aRemoveArchive) wxRemoveFile(aArchiveFile);
  return ok;

#else
  wxUnusedVar(aArchiveFile);
  wxUnusedVar(aTargetDir);
  wxUnusedVar(aStripPath);
  wxUnusedVar(aMTime);
  wxUnusedVar(aRemoveArchive);
  wxUnusedVar(verbose_extract_log);
  return false;
#endif
}
#endif  // DLDR_USE_LIBARCHIVE

namespace {

#if defined(CHARTDLDR_RAR_UNARR) || !defined(DLDR_USE_LIBARCHIVE)
ar_archive *ar_open_any_archive(ar_stream *stream, const char *fileext) {
  ar_archive *ar = ar_open_rar_archive(stream);
  if (!ar)
    ar =
        ar_open_zip_archive(stream, fileext && (strcmp(fileext, ".xps") == 0 ||
                                                strcmp(fileext, ".epub") == 0));
  if (!ar) ar = ar_open_7z_archive(stream);
  if (!ar) ar = ar_open_tar_archive(stream);
  return ar;
}
#endif

}  // namespace

#if defined(CHARTDLDR_RAR_UNARR) || !defined(DLDR_USE_LIBARCHIVE)
bool chartdldr_pi::ExtractUnarrFiles(const wxString &aRarFile,
                                     const wxString &aTargetDir,
                                     bool aStripPath, wxDateTime aMTime,
                                     bool aRemoveRar, bool verbose_extract_log) {
  ChartDldrLogExtractStart(aRarFile, verbose_extract_log);
  ar_stream *stream = NULL;
  ar_archive *ar = NULL;
  int entry_count = 1;
  int entry_skips = 0;
  int error_step = 1;
  bool ret = true;

  stream = ar_open_file(aRarFile.c_str());
  if (!stream) {
    wxLogError(_T("Can not open file '") + aRarFile + _T("'."));
    ar_close_archive(ar);
    ar_close(stream);
    return false;
  }
  ar = ar_open_any_archive(stream, strrchr(aRarFile.c_str(), '.'));
  if (!ar) {
    wxLogError(_T("Can not open archive '") + aRarFile + _T("'."));
    ar_close_archive(ar);
    ar_close(stream);
    return false;
  }
  while (ar_parse_entry(ar)) {
    size_t size = ar_entry_get_size(ar);
    wxString name = ar_entry_get_name(ar);
    wxString originalName = name;  // Save for logging
    if (aStripPath) {
      wxFileName fn(name);
      /* We can completly replace the entry path */
      // fn.SetPath(aTargetDir);
      // name = fn.GetFullPath();
      /* Or only remove the first dir (eg. ENC_ROOT) */
      if (fn.GetDirCount() > 0) {
        fn.RemoveDir(0);
        name = fn.GetFullPath();
      }
    }

    // Path traversal protection: validate path stays inside target directory
    wxString fullPath;
    if (!IsPathInsideDir(aTargetDir, name, fullPath)) {
      wxLogWarning(_T("Skipping archive entry with path traversal attempt: ") +
                   originalName);
      continue;
    }
    name = fullPath;

    wxFileName fn(name);
    if (!fn.DirExists()) {
      if (!wxFileName::Mkdir(fn.GetPath())) {
        wxLogError(_T("Can not create directory '") + fn.GetPath() + _T("'."));
        ret = false;
        break;
      }
    }
    wxFileOutputStream file(name);
    if (!file) {
      wxLogError(_T("Can not create file '") + name + _T("'."));
      ret = false;
      break;
    }
    while (size > 0) {
      unsigned char buffer[1024];
      size_t count = size < sizeof(buffer) ? size : sizeof(buffer);
      if (!ar_entry_uncompress(ar, buffer, count)) break;
      file.Write(buffer, count);
      size -= count;
    }
    file.Close();
    fn.SetTimes(&aMTime, &aMTime, &aMTime);
    if (size > 0) {
      wxLogError("Warning: Failed to uncompress... skipping");
      entry_skips++;
      ret = false;
    }
  }
  if (!ar_at_eof(ar)) {
    wxLogError("Error: Failed to parse entry %d!", entry_count);
    ret = false;
  }
  ar_close_archive(ar);
  ar_close(stream);

  if (aRemoveRar) wxRemoveFile(aRarFile);

#ifdef _UNIX
  // reset LC_NUMERIC locale, some locales use a comma for decimal point
  // and it corrupts navobj.xml file
  setlocale(LC_NUMERIC, "C");
#endif

  return ret;
}
#endif  // CHARTDLDR_RAR_UNARR || !DLDR_USE_LIBARCHIVE

bool chartdldr_pi::ExtractZipFiles(const wxString &aZipFile,
                                   const wxString &aTargetDir, bool aStripPath,
                                   wxDateTime aMTime, bool aRemoveZip,
                                   bool verbose_extract_log) {
  bool ret = true;

#ifdef __ANDROID__
  int nStrip = 0;
  if (aStripPath) nStrip = 1;

  ret = AndroidUnzip(aZipFile, aTargetDir, nStrip, true);
#else
  std::unique_ptr<wxZipEntry> entry(new wxZipEntry());

  do {
    ChartDldrLogExtractStart(aZipFile, verbose_extract_log);
    wxFileInputStream in(aZipFile);

    if (!in) {
      wxLogMessage(_T("Can not open file '") + aZipFile + _T("'."));
      ret = false;
      break;
    }
    wxZipInputStream zip(in);
    ret = false;

    while (entry.reset(zip.GetNextEntry()), entry.get() != NULL) {
      // access meta-data
      wxString name = entry->GetName();
      wxString fullPath;
      if (aStripPath) {
        wxFileName fn(name);
        /* We can completly replace the entry path */
        // fn.SetPath(aTargetDir);
        // name = fn.GetFullPath();
        /* Or only remove the first dir (eg. ENC_ROOT) */
        if (fn.GetDirCount() > 0) fn.RemoveDir(0);
        name = fn.GetFullPath();
      }

      // Path traversal protection: validate path stays inside target directory
      if (!IsPathInsideDir(aTargetDir, name, fullPath)) {
        wxLogWarning(_T("Skipping zip entry with path traversal attempt: ") +
                     entry->GetName());
        continue;
      }
      name = fullPath;

      // read 'zip' to access the entry's data
      if (entry->IsDir()) {
        int perm = entry->GetMode();
        if (!wxFileName::Mkdir(name, perm, wxPATH_MKDIR_FULL)) {
          wxLogMessage(_T("Can not create directory '") + name + _T("'."));
          ret = false;
          break;
        }
      } else {
        if (!zip.OpenEntry(*entry.get())) {
          wxLogMessage(_T("Can not open zip entry '") + entry->GetName() +
                       _T("'."));
          ret = false;
          break;
        }
        if (!zip.CanRead()) {
          wxLogMessage(_T("Can not read zip entry '") + entry->GetName() +
                       _T("'."));
          ret = false;
          break;
        }

        wxFileName fn(name);
        if (!fn.DirExists()) {
          if (!wxFileName::Mkdir(fn.GetPath())) {
            wxLogMessage(_T("Can not create directory '") + fn.GetPath() +
                         _T("'."));
            ret = false;
            break;
          }
        }

        wxFileOutputStream file(name);

        if (!file) {
          wxLogMessage(_T("Can not create file '") + name + _T("'."));
          ret = false;
          break;
        }
        zip.Read(file);
        fn.SetTimes(&aMTime, &aMTime, &aMTime);
        ret = true;
      }
    }

  } while (false);

  if (aRemoveZip) wxRemoveFile(aZipFile);
#endif  // Android

  return ret;
}

bool chartdldr_pi::ProcessFile(const wxString &aFile,
                               const wxString &aTargetDir, bool aStripPath,
                               wxDateTime aMTime, bool verbose_extract_log) {
  if (aFile.Lower().EndsWith(_T("zip")))  // Zip compressed
  {
    bool ret = ExtractZipFiles(aFile, aTargetDir, aStripPath, aMTime, false,
                               verbose_extract_log);
    if (ret)
      wxRemoveFile(aFile);
    else
      wxLogError(_T("chartdldr_pi: Unable to extract: ") + aFile);
    return ret;
  }
#ifdef DLDR_USE_LIBARCHIVE
  else if (aFile.Lower().EndsWith(_T("rar"))) {
#ifdef CHARTDLDR_RAR_UNARR
    bool ret = ExtractUnarrFiles(aFile, aTargetDir, aStripPath, aMTime, false,
                                 verbose_extract_log);
#else
    bool ret = ExtractLibArchiveFiles(aFile, aTargetDir, aStripPath, aMTime,
                                      false, verbose_extract_log);
#endif
    if (ret)
      wxRemoveFile(aFile);
    else
      wxLogError(_T("chartdldr_pi: Unable to extract: ") + aFile);
    return ret;
  } else if (aFile.Lower().EndsWith(_T("tar")) ||
             aFile.Lower().EndsWith(_T("gz")) ||
             aFile.Lower().EndsWith(_T("bz2")) ||
             aFile.Lower().EndsWith(_T("lzma")) ||
             aFile.Lower().EndsWith(_T("7z")) ||
             aFile.Lower().EndsWith(_T("xz"))) {
    bool ret = ExtractLibArchiveFiles(aFile, aTargetDir, aStripPath, aMTime,
                                      false, verbose_extract_log);
    if (ret)
      wxRemoveFile(aFile);
    else
      wxLogError(_T("chartdldr_pi: Unable to extract: ") + aFile);
    return ret;
  }
#else
  else if (aFile.Lower().EndsWith(_T("rar")) ||
           aFile.Lower().EndsWith(_T("tar"))
#ifdef HAVE_BZIP2
           || aFile.Lower().EndsWith(_T("bz2"))
#endif
#ifdef HAVE_ZLIB
           || aFile.Lower().EndsWith(_T("gz"))
#endif
#ifdef HAVE_7Z
           || aFile.Lower().EndsWith(
                  _T("7z"))  // TODO: Could it actually extract more formats the
                             // LZMA SDK supports?
#endif
  ) {
    bool ret = ExtractUnarrFiles(aFile, aTargetDir, aStripPath, aMTime, false,
                                 verbose_extract_log);
    if (ret)
      wxRemoveFile(aFile);
    else
      wxLogError(_T("chartdldr_pi: Unable to extract: ") + aFile);
    return ret;
  }
#endif

#ifdef __ANDROID__
  else if (aFile.Lower().EndsWith(_T("tar")) ||
           aFile.Lower().EndsWith(_T("gz")) ||
           aFile.Lower().EndsWith(_T("bz2")) ||
           aFile.Lower().EndsWith(_T("lzma")) ||
           aFile.Lower().EndsWith(_T("7z")) ||
           aFile.Lower().EndsWith(_T("xz"))) {
    int nStrip = 0;
    if (aStripPath) nStrip = 1;

    if (m_dldrpanel) m_dldrpanel->SetChartInfo(_("Installing charts."));

    androidShowBusyIcon();
    bool ret = AndroidUnzip(aFile, aTargetDir, nStrip, true);
    androidHideBusyIcon();

    return ret;
  }
#endif

  else  // Uncompressed
  {
    wxFileName fn(aFile);
    if (fn.GetPath() != aTargetDir)  // We have to move the file somewhere
    {
      if (!wxDirExists(aTargetDir)) {
        if (wxFileName::Mkdir(aTargetDir, 0755, wxPATH_MKDIR_FULL)) {
          if (!wxRenameFile(aFile, aTargetDir)) return false;
        } else
          return false;
      }
    }
    wxString name = fn.GetFullName();
    fn.Clear();
    fn.Assign(aTargetDir, name);
    fn.SetTimes(&aMTime, &aMTime, &aMTime);
  }
  return true;
}
