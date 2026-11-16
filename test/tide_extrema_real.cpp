// SPDX-License-Identifier: GPL-2.0-or-later
// Real harmonic-data regression; see check_real_tide_cache.py.
#include <wx/init.h>
#include <wx/window.h>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include "tcmgr.h"
int OCPNMessageBox(wxWindow*, const wxString&, const wxString&, int, int, int,
                   int) {
  std::abort();
}
int main(int argc, char** argv) {
  assert(argc == 2);
  wxInitializer init;
  assert(init.IsOk());
  std::vector<std::string> sources{argv[1]};
  constexpr time_t when = 1794826800;
  for (bool highFirst : {true, false}) {
    TCMgr sequential;
    sequential.LoadDataSources(sources);
    int station =
        sequential.GetStationIDXbyNameType("Portsmouth", 50.8, -1.1, 'T');
    assert(station > 0);
    for (bool high : {highFirst, !highFirst}) {
      float now = 0, previous = 0;
      bool rising;
      assert(sequential.GetTideFlowSens(when, -600, station, now, previous,
                                        rising));
      TCMgr fresh;
      fresh.LoadDataSources(sources);
      int freshStation =
          fresh.GetStationIDXbyNameType("Portsmouth", 50.8, -1.1, 'T');
      assert(freshStation > 0);
      float expected = 0, actual = 0;
      time_t expectedTime = 0, actualTime = 0;
      fresh.GetHightOrLowTide(when, high ? 600 : -600, high ? 60 : -60, now,
                              high, freshStation, expected, expectedTime);
      sequential.GetHightOrLowTide(when, high ? 600 : -600, high ? 60 : -60,
                                   now, high, station, actual, actualTime);
      printf("%s-first, %s: sequential %.6f @ %ld, fresh %.6f @ %ld\n",
             highFirst ? "high" : "low", high ? "high" : "low", actual,
             (long)actualTime, expected, (long)expectedTime);
      fflush(stdout);
      assert(std::isfinite(actual));
      assert(std::abs(expected - actual) < 0.00001 &&
             expectedTime == actualTime);
    }
  }
}
