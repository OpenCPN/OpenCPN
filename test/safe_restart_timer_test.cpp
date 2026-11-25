// SPDX-License-Identifier: GPL-2.0-or-later

// Standalone Linux GUI regression using the production dialog and wx event
// loop. Build from the repository root:
// c++ -std=c++17 test/safe_restart_timer_test.cpp -Imodel/include \
//   -Imodel/include/model -Ilibs/std_filesystem/include \
//   $(wx-config --cxxflags --libs std,html) -o /tmp/safe-timer-test
// Run on a test display, e.g. xvfb-run -a /tmp/safe-timer-test.
// Does not call safe_mode::CheckLastStart or touch a user profile.

#include <chrono>
#include <cstdio>
#include <wx/app.h>
#include "../gui/src/safe_mode_gui.cpp"

namespace safe_mode {
bool safe_mode = false;
std::string CheckFilePath() { return std::string(); }
}  // namespace safe_mode
class TimerTestApp : public wxApp {
public:
  bool OnInit() override {
    SetExitOnFrameDelete(false);
    return true;
  }
};
wxIMPLEMENT_APP_NO_MAIN(TimerTestApp);
class CloseDialogTimer : public wxTimer {
public:
  CloseDialogTimer(wxDialog& dialog, int code) : dialog_(dialog), code_(code) {}
  void Notify() override { dialog_.EndModal(code_); }

private:
  wxDialog& dialog_;
  int code_;
};
int main(int argc, char** argv) {
  if (!wxEntryStart(argc, argv) || !wxTheApp->CallOnInit()) return 2;
  bool passed = true;
  {
    SafeModeDialog dialog(nullptr, 1);
    CloseDialogTimer watchdog(dialog, wxID_ABORT);
    watchdog.StartOnce(3000);
    const auto start = std::chrono::steady_clock::now();
    const int result = dialog.ShowModal();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    watchdog.Stop();
    dialog.StopTimer();
    std::printf("one-second timeout: result=%d elapsed=%lldms\n", result,
                (long long)elapsed);
    passed = result == wxID_CANCEL && elapsed >= 500 && elapsed < 2800;
  }
  {
    SafeModeDialog dialog(nullptr, 1);
    dialog.StopTimer();
    CloseDialogTimer close(dialog, wxID_OK);
    close.StartOnce(1500);
    const int result = dialog.ShowModal();
    close.Stop();
    std::printf("stopped timer: result=%d\n", result);
    passed = passed && result == wxID_OK;
  }
  wxTheApp->OnExit();
  wxEntryCleanup();
  return passed ? 0 : 1;
}
