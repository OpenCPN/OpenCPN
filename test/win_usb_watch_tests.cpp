// SPDX-License-Identifier: GPL-2.0-or-later
#include <wx/app.h>
#include <wx/frame.h>

#include <gtest/gtest.h>

#include "model/sys_events.h"
#include "model/win_usb_watch.h"

class UsbWatchTestApp : public wxApp {
public:
  bool OnInit() override { return true; }
};

wxIMPLEMENT_APP_NO_MAIN(UsbWatchTestApp);

TEST(WinUsbWatch, RepeatedStartAndStop) {
  auto* parent = new wxFrame(nullptr, wxID_ANY, "USB watcher test");
  wxTheApp->SetTopWindow(parent);
  WinUsbWatchDaemon daemon(SystemEvents::GetInstance());
  const auto children = parent->GetChildren().GetCount();
  daemon.Start();
  daemon.Start();
  EXPECT_EQ(parent->GetChildren().GetCount(), children + 1);
  daemon.Stop();
  daemon.Stop();
  EXPECT_EQ(parent->GetChildren().GetCount(), children);
  delete parent;
  wxTheApp->SetTopWindow(nullptr);
}

TEST(WinUsbWatch, ParentDestructionBeforeStopAndRestart) {
  WinUsbWatchDaemon daemon(SystemEvents::GetInstance());
  for (int i = 0; i < 3; ++i) {
    auto* parent = new wxFrame(nullptr, wxID_ANY, "USB watcher parent");
    wxTheApp->SetTopWindow(parent);
    daemon.Start();
    // wxWidgets 3.3 destroys the application's windows before OnExit(),
    // where OpenCPN calls Stop(). The parent owns this hidden child frame.
    delete parent;
    wxTheApp->SetTopWindow(nullptr);
    daemon.Stop();
    daemon.Stop();
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  if (!wxEntryStart(argc, argv)) return 1;
  if (!wxTheApp->CallOnInit()) {
    wxEntryCleanup();
    return 1;
  }
  const int result = RUN_ALL_TESTS();
  wxTheApp->OnExit();
  wxEntryCleanup();
  return result;
}
