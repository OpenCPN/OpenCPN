/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "chartdldr_archive_classify.h"
#include "chartdldr_pi.h"
#include "chartdldr_extract_txn.h"

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
      wxLogError(wxString::Format("Chartdldr_pi: LibArchive error: %s",
                                  archive_error_string(aw)));
      return (r);
    }
  }
}
#endif
#endif

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

static void ChartDldrLogExtractStart(const wxString &archive_file,
                                     bool verbose_extract_log) {
  if (verbose_extract_log) {
    wxLogMessage(_T("chartdldr_pi: Going to extract '") + archive_file +
                 _T("'."));
  }
}

/** On success remove the archive; on failure log. Shared ProcessFile epilogue.
 */
static bool ChartDldrFinishExtractedArchive(bool ok, const wxString &archive) {
  if (ok) {
    wxRemoveFile(archive);
  } else {
    wxLogError(_T("chartdldr_pi: Unable to extract: ") + archive);
  }
  return ok;
}

#ifdef DLDR_USE_LIBARCHIVE
bool chartdldr_pi::ExtractLibArchiveFiles(const wxString &aArchiveFile,
                                          const wxString &aTargetDir,
                                          bool aStripPath, wxDateTime aMTime,
                                          bool aRemoveArchive,
                                          bool verbose_extract_log) {
#ifndef __ANDROID__
  ChartDldrLogExtractStart(aArchiveFile, verbose_extract_log);
  ChartDldrExtractTxn txn;
  if (!txn.Begin(aTargetDir)) {
    return false;
  }

  struct archive *a = NULL;
  struct archive *ext = NULL;
  bool ok = false;

  int flags = 0;
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
      ok = true;
      break;
    }

    if (r < ARCHIVE_OK) {
      wxLogError(wxString::Format("Chartdldr_pi: LibArchive error: %s",
                                  archive_error_string(a)));
    }
    if (r < ARCHIVE_WARN) {
      ok = false;
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

    entryName = ChartDldrArchiveEntryRelPath(entryName, aStripPath);
    if (entryName.IsEmpty()) {
      continue;
    }

    wxString stagePath;
    if (!txn.MapEntry(entryName, stagePath)) {
      wxLogWarning(_T("Skipping archive entry with path traversal attempt: ") +
                   entryName);
      continue;
    }

#ifdef _WIN32
    archive_entry_copy_pathname_w(entry, stagePath.wc_str());
#else
    archive_entry_copy_pathname(entry, stagePath.fn_str().data());
#endif

    r = archive_write_header(ext, entry);
    if (r < ARCHIVE_OK) {
      wxLogError(wxString::Format("Chartdldr_pi: LibArchive error: %s",
                                  archive_error_string(ext)));
    }
    if (r < ARCHIVE_WARN) {
      ok = false;
      goto cleanup;
    }

    const bool is_dir = archive_entry_filetype(entry) == AE_IFDIR;
    if (archive_entry_size(entry) > 0) {
      r = copy_data(a, ext);
      if (r < ARCHIVE_OK) {
        wxLogError(wxString::Format("Chartdldr_pi: LibArchive error: %s",
                                    archive_error_string(ext)));
      }
      if (r < ARCHIVE_WARN) {
        ok = false;
        goto cleanup;
      }
    }

    r = archive_write_finish_entry(ext);
    if (r < ARCHIVE_OK) {
      wxLogError(wxString::Format("Chartdldr_pi: LibArchive error: %s",
                                  archive_error_string(ext)));
    }
    if (r < ARCHIVE_WARN) {
      ok = false;
      goto cleanup;
    }

    if (!is_dir) {
      const wxFileOffset expected = archive_entry_size(entry);
      if (!txn.AcceptWrittenFile(stagePath, expected)) {
        ok = false;
        goto cleanup;
      }
    }
  }

cleanup:
  if (a) {
    archive_read_close(a);
    archive_read_free(a);
  }
  if (ext) {
    archive_write_close(ext);
    archive_write_free(ext);
  }

  ok = txn.Finalize(ok, aMTime);
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

#if defined(CHARTDLDR_RAR_UNARR) || !defined(DLDR_USE_LIBARCHIVE)
bool chartdldr_pi::ExtractUnarrFiles(const wxString &aRarFile,
                                     const wxString &aTargetDir,
                                     bool aStripPath, wxDateTime aMTime,
                                     bool aRemoveRar,
                                     bool verbose_extract_log) {
  ChartDldrLogExtractStart(aRarFile, verbose_extract_log);
  ChartDldrExtractTxn txn;
  if (!txn.Begin(aTargetDir)) {
    return false;
  }

  ar_stream *stream = NULL;
  ar_archive *ar = NULL;
  bool ok = true;

  stream = ar_open_file(aRarFile.c_str());
  if (!stream) {
    wxLogError(_T("Can not open file '") + aRarFile + _T("'."));
    return txn.Finalize(false, aMTime);
  }
  ar = ar_open_any_archive(stream, strrchr(aRarFile.c_str(), '.'));
  if (!ar) {
    wxLogError(_T("Can not open archive '") + aRarFile + _T("'."));
    ar_close(stream);
    return txn.Finalize(false, aMTime);
  }
  while (ar_parse_entry(ar)) {
    size_t size = ar_entry_get_size(ar);
    const wxFileOffset expected_size = static_cast<wxFileOffset>(size);
    const wxString originalName = ar_entry_get_name(ar);
    const wxString name =
        ChartDldrArchiveEntryRelPath(originalName, aStripPath);

    wxString stagePath;
    if (!txn.MapEntry(name, stagePath)) {
      wxLogWarning(_T("Skipping archive entry with path traversal attempt: ") +
                   originalName);
      continue;
    }
    if (!txn.EnsureParentDirs(stagePath)) {
      wxLogError(_T("Can not create directory for '") + stagePath + _T("'."));
      ok = false;
      break;
    }

    wxFileOutputStream file(stagePath);
    if (!file) {
      wxLogError(_T("Can not create file '") + stagePath + _T("'."));
      ok = false;
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
    if (size > 0 || !txn.AcceptWrittenFile(stagePath, expected_size)) {
      wxLogError("Warning: Failed to uncompress... skipping");
      ok = false;
      break;
    }
  }
  if (ok && !ar_at_eof(ar)) {
    wxLogError("Error: Failed to parse archive entry!");
    ok = false;
  }
  ar_close_archive(ar);
  ar_close(stream);

  ok = txn.Finalize(ok, aMTime);
  if (ok && aRemoveRar) wxRemoveFile(aRarFile);

#ifdef _UNIX
  // reset LC_NUMERIC locale, some locales use a comma for decimal point
  // and it corrupts navobj.xml file
  setlocale(LC_NUMERIC, "C");
#endif

  return ok;
}
#endif  // CHARTDLDR_RAR_UNARR || !DLDR_USE_LIBARCHIVE

bool chartdldr_pi::ExtractZipFiles(const wxString &aZipFile,
                                   const wxString &aTargetDir, bool aStripPath,
                                   wxDateTime aMTime, bool aRemoveZip,
                                   bool verbose_extract_log) {
  ChartDldrLogExtractStart(aZipFile, verbose_extract_log);
  ChartDldrExtractTxn txn;
  if (!txn.Begin(aTargetDir)) {
    return false;
  }

#ifdef __ANDROID__
  int nStrip = 0;
  if (aStripPath) nStrip = 1;
  bool ok = AndroidUnzip(aZipFile, txn.StageRoot(), nStrip, false);
  if (ok) {
    ok = txn.AcceptAllStagedFiles();
  }
  ok = txn.Finalize(ok, aMTime);
  if (ok && aRemoveZip) wxRemoveFile(aZipFile);
  return ok;
#else
  bool ok = false;
  std::unique_ptr<wxZipEntry> entry(new wxZipEntry());
  wxFileInputStream in(aZipFile);
  if (!in) {
    wxLogMessage(_T("Can not open file '") + aZipFile + _T("'."));
    return txn.Finalize(false, aMTime);
  }
  wxZipInputStream zip(in);
  ok = true;

  while (entry.reset(zip.GetNextEntry()), entry.get() != NULL) {
    const wxString name =
        ChartDldrArchiveEntryRelPath(entry->GetName(), aStripPath);

    wxString stagePath;
    if (entry->IsDir()) {
      if (!txn.MapEntry(name, stagePath)) {
        wxLogWarning(_T("Skipping zip entry with path traversal attempt: ") +
                     entry->GetName());
        continue;
      }
      if (!wxFileName::Mkdir(stagePath, entry->GetMode(), wxPATH_MKDIR_FULL)) {
        wxLogMessage(_T("Can not create directory '") + stagePath + _T("'."));
        ok = false;
        break;
      }
      continue;
    }

    if (!txn.MapEntry(name, stagePath)) {
      wxLogWarning(_T("Skipping zip entry with path traversal attempt: ") +
                   entry->GetName());
      continue;
    }
    if (!txn.EnsureParentDirs(stagePath)) {
      wxLogMessage(_T("Can not create directory for '") + stagePath + _T("'."));
      ok = false;
      break;
    }

    if (!zip.OpenEntry(*entry.get())) {
      wxLogMessage(_T("Can not open zip entry '") + entry->GetName() +
                   _T("'."));
      ok = false;
      break;
    }
    if (!zip.CanRead()) {
      wxLogMessage(_T("Can not read zip entry '") + entry->GetName() +
                   _T("'."));
      ok = false;
      break;
    }

    wxFileOutputStream file(stagePath);
    if (!file) {
      wxLogMessage(_T("Can not create file '") + stagePath + _T("'."));
      ok = false;
      break;
    }
    zip.Read(file);
    file.Close();
    if (!file.IsOk() || !txn.AcceptWrittenFile(stagePath, entry->GetSize())) {
      wxLogMessage(_T("Failed to extract zip entry '") + entry->GetName() +
                   _T("'."));
      ok = false;
      break;
    }
  }

  ok = txn.Finalize(ok, aMTime);
  if (ok && aRemoveZip) wxRemoveFile(aZipFile);
  return ok;
#endif  // Android
}

bool chartdldr_pi::ProcessFile(const wxString &aFile,
                               const wxString &aTargetDir, bool aStripPath,
                               wxDateTime aMTime, bool verbose_extract_log) {
  switch (ChartDldrClassifyArchive(aFile)) {
    case ChartDldrArchiveKind::Zip:
      return ChartDldrFinishExtractedArchive(
          ExtractZipFiles(aFile, aTargetDir, aStripPath, aMTime, false,
                          verbose_extract_log),
          aFile);

    case ChartDldrArchiveKind::Rar:
#if defined(DLDR_USE_LIBARCHIVE) && !defined(CHARTDLDR_RAR_UNARR)
      return ChartDldrFinishExtractedArchive(
          ExtractLibArchiveFiles(aFile, aTargetDir, aStripPath, aMTime, false,
                                 verbose_extract_log),
          aFile);
#else
      // Android / non-libarchive, or libarchive+unarr RAR: never AndroidUnzip.
      return ChartDldrFinishExtractedArchive(
          ExtractUnarrFiles(aFile, aTargetDir, aStripPath, aMTime, false,
                            verbose_extract_log),
          aFile);
#endif

    case ChartDldrArchiveKind::MultiFormat:
#ifdef DLDR_USE_LIBARCHIVE
      return ChartDldrFinishExtractedArchive(
          ExtractLibArchiveFiles(aFile, aTargetDir, aStripPath, aMTime, false,
                                 verbose_extract_log),
          aFile);
#else
      // Non-ZIP containers on Android use unarr, not AndroidUnzip.
      return ChartDldrFinishExtractedArchive(
          ExtractUnarrFiles(aFile, aTargetDir, aStripPath, aMTime, false,
                            verbose_extract_log),
          aFile);
#endif

    case ChartDldrArchiveKind::Plain:
      break;
  }

  // Uncompressed single file: move into place and stamp.
  wxFileName fn(aFile);
  if (fn.GetPath() != aTargetDir) {
    if (!wxDirExists(aTargetDir)) {
      if (!wxFileName::Mkdir(aTargetDir, 0755, wxPATH_MKDIR_FULL)) {
        return false;
      }
    }
    wxFileName dest(aTargetDir, fn.GetFullName());
    if (!wxRenameFile(aFile, dest.GetFullPath())) {
      return false;
    }
    fn = dest;
  }
  fn.SetTimes(&aMTime, &aMTime, &aMTime);
  return true;
}
