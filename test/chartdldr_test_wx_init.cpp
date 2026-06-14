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

#include <wx/init.h>

namespace {

class ChartDldrWxEnvironment : public ::testing::Environment {
public:
  void SetUp() override { init_ = std::make_unique<wxInitializer>(); }

private:
  std::unique_ptr<wxInitializer> init_;
};

[[maybe_unused]] const ::testing::Environment* const chartdldr_wx_environment =
    ::testing::AddGlobalTestEnvironment(new ChartDldrWxEnvironment);

}  // namespace
