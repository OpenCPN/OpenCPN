#include <gtest/gtest.h>

#include <wx/file.h>
#include <wx/filename.h>

#include "GribMerge.h"

#include <vector>

namespace {

wxString WriteTempFile(const std::vector<unsigned char> &data) {
  wxString path = wxFileName::CreateTempFileName("ocpn_grib_merge_test");
  wxFile file(path, wxFile::write);
  EXPECT_TRUE(file.IsOpened());
  if (!data.empty()) EXPECT_EQ(file.Write(data.data(), data.size()), data.size());
  file.Close();
  return path;
}

std::vector<unsigned char> Grib1Message() {
  return {'G', 'R', 'I', 'B', 0x00, 0x00, 0x0c, 0x01,
          '7', '7', '7', '7'};
}

std::vector<unsigned char> Grib2Message() {
  return {'G', 'R', 'I', 'B', 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x14, '7',  '7',  '7',  '7'};
}

}  // namespace

TEST(GribMerge, ValidatesMixedEditionMessages) {
  std::vector<unsigned char> data = Grib1Message();
  std::vector<unsigned char> grib2 = Grib2Message();
  data.insert(data.end(), grib2.begin(), grib2.end());

  wxString path = WriteTempFile(data);
  GribMergeFile file;
  wxString error;

  EXPECT_TRUE(GribMerge::ReadAndValidate(path, file, error)) << error;
  EXPECT_EQ(file.messages.size(), 2U);
  EXPECT_EQ(file.summary.messageCount, 2U);
  EXPECT_EQ(file.summary.grib1Count, 1U);
  EXPECT_EQ(file.summary.grib2Count, 1U);

  wxRemoveFile(path);
}

TEST(GribMerge, ValidatesGrib1Message) {
  wxString path = WriteTempFile(Grib1Message());
  GribMergeFile file;
  wxString error;

  EXPECT_TRUE(GribMerge::ReadAndValidate(path, file, error)) << error;
  EXPECT_EQ(file.messages.size(), 1U);
  EXPECT_EQ(file.summary.grib1Count, 1U);
  EXPECT_EQ(file.summary.grib2Count, 0U);

  wxRemoveFile(path);
}

TEST(GribMerge, ValidatesGrib2Message) {
  wxString path = WriteTempFile(Grib2Message());
  GribMergeFile file;
  wxString error;

  EXPECT_TRUE(GribMerge::ReadAndValidate(path, file, error)) << error;
  EXPECT_EQ(file.messages.size(), 1U);
  EXPECT_EQ(file.summary.grib1Count, 0U);
  EXPECT_EQ(file.summary.grib2Count, 1U);

  wxRemoveFile(path);
}

TEST(GribMerge, RejectsMessageWithout7777Terminator) {
  std::vector<unsigned char> data = Grib1Message();
  data[10] = '0';

  wxString path = WriteTempFile(data);
  GribMergeFile file;
  wxString error;

  EXPECT_FALSE(GribMerge::ReadAndValidate(path, file, error));
  EXPECT_TRUE(error.Contains("7777"));

  wxRemoveFile(path);
}

TEST(GribMerge, RejectsEmptyFile) {
  std::vector<unsigned char> data;
  wxString path = WriteTempFile(data);
  GribMergeFile file;
  wxString error;

  EXPECT_FALSE(GribMerge::ReadAndValidate(path, file, error));
  EXPECT_TRUE(error.Contains("empty"));

  wxRemoveFile(path);
}

TEST(GribMerge, WritesMixedEditionMerge) {
  wxString grib1Path = WriteTempFile(Grib1Message());
  wxString grib2Path = WriteTempFile(Grib2Message());
  wxString outputPath = wxFileName::CreateTempFileName("ocpn_grib_merge_out");

  GribMergeFile grib1File;
  GribMergeFile grib2File;
  GribMergeFile mergedFile;
  wxString error;

  EXPECT_TRUE(GribMerge::ReadAndValidate(grib1Path, grib1File, error))
      << error;
  EXPECT_TRUE(GribMerge::ReadAndValidate(grib2Path, grib2File, error))
      << error;
  EXPECT_TRUE(GribMerge::WriteMerged(grib1File, grib2File, outputPath, error))
      << error;
  EXPECT_TRUE(GribMerge::ReadAndValidate(outputPath, mergedFile, error))
      << error;
  EXPECT_EQ(mergedFile.summary.messageCount, 2U);
  EXPECT_EQ(mergedFile.summary.grib1Count, 1U);
  EXPECT_EQ(mergedFile.summary.grib2Count, 1U);

  wxRemoveFile(grib1Path);
  wxRemoveFile(grib2Path);
  wxRemoveFile(outputPath);
}

TEST(GribMerge, RejectsTruncatedMessage) {
  std::vector<unsigned char> data = Grib1Message();
  data[6] = 0x0d;

  wxString path = WriteTempFile(data);
  GribMergeFile file;
  wxString error;

  EXPECT_FALSE(GribMerge::ReadAndValidate(path, file, error));
  EXPECT_TRUE(error.Contains("truncated"));

  wxRemoveFile(path);
}
