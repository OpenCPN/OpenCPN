/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include <wx/init.h>
#include <wx/log.h>

#include "model/logger.h"

namespace {

// Route wx logs to a file for the unit-test process. Intentional failure
// paths (wxCopyFile, wxLogError) otherwise print "Error:" / "(error N: ...)"
// to stderr; MSBuild treats those lines as build failures even when gtest
// reports all tests passed.
class ChartDldrWxEnvironment : public ::testing::Environment {
public:
  void SetUp() override {
    init_ = std::make_unique<wxInitializer>();
    const std::string logfile =
        std::string(CMAKE_BINARY_DIR) + "/chartdldr_unittests.log";
    previous_log_ = wxLog::SetActiveTarget(new OcpnLog(logfile.c_str()));
    wxLog::SetLogLevel(wxLOG_Debug);
    wxLog::FlushActive();
  }

  void TearDown() override {
    delete wxLog::SetActiveTarget(previous_log_);
    previous_log_ = nullptr;
    init_.reset();
  }

private:
  std::unique_ptr<wxInitializer> init_;
  wxLog* previous_log_ = nullptr;
};

[[maybe_unused]] const ::testing::Environment* const chartdldr_wx_environment =
    ::testing::AddGlobalTestEnvironment(new ChartDldrWxEnvironment);

}  // namespace
