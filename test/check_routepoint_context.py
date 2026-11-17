# SPDX-License-Identifier: GPL-2.0-or-later
#!/usr/bin/env python3
"""Test the production context-result block with supplied selection outcomes.

Run: python3 test/check_routepoint_context.py [repository-root]
Requires g++. Selection hit-testing and a loaded-plugin GUI are not simulated.
"""
from pathlib import Path
import subprocess
import sys
import tempfile

repo = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[1]
source = (repo / "gui/src/chcanv.cpp").read_text()
start = source.index("ChartCanvas::GetCanvasContextAtPoint(")
start = source.index("  // Populate the return struct", start)
end = source.index("\nvoid ChartCanvas::MouseTimedEvent", start)
block = source[start:end]
prefix = r'''
#include <cassert>
#include <memory>
#include <string>
#include <cstdio>
struct wxString {
  std::string value;
  void Printf(const char* f,int n) { char b[32]; std::snprintf(b,sizeof b,f,n);value=b; }
  std::string ToStdString() const { return value; }
};
namespace HostApi121 {
enum class PiContextObjectType {kObjectChart,kObjectAisTarget,kObjectRoutepoint,kObjectRoutesegment,kObjectTracksegment};
struct PiPointContext { PiContextObjectType object_type; std::string object_ident; };
}
// Non-overlapping selection flags, as in the production selector.
enum {SELTYPE_AISTARGET=1,SELTYPE_MARKPOINT=2,SELTYPE_ROUTEPOINT=4,SELTYPE_ROUTESEGMENT=8,SELTYPE_TRACKSEGMENT=16};
struct Object { wxString m_GUID; } point{{"point-guid"}},route{{"route-guid"}},track{{"track-guid"}};
std::shared_ptr<HostApi121::PiPointContext> Context(int seltype,Object* FoundRoutePoint,
    Object* SelectedRoute,Object* m_pSelectedTrack,int FoundAIS_MMSI=123456789) {
'''
main = r'''
int main() {
  using T=HostApi121::PiContextObjectType;
  auto check=[](int flags,Object* p,Object* r,Object* t,T type,const char* id) {
    auto result=Context(flags,p,r,t);
    assert(result->object_type==type && result->object_ident==id);
  };
  check(SELTYPE_ROUTEPOINT,&point,&route,nullptr,T::kObjectRoutepoint,"point-guid");
  check(SELTYPE_ROUTEPOINT|SELTYPE_ROUTESEGMENT,&point,&route,nullptr,T::kObjectRoutepoint,"point-guid");
  check(SELTYPE_MARKPOINT,&point,nullptr,nullptr,T::kObjectRoutepoint,"point-guid");
  check(SELTYPE_ROUTESEGMENT,nullptr,&route,nullptr,T::kObjectRoutesegment,"route-guid");
  check(SELTYPE_TRACKSEGMENT,nullptr,nullptr,&track,T::kObjectTracksegment,"track-guid");
  check(SELTYPE_AISTARGET,nullptr,nullptr,nullptr,T::kObjectAisTarget,"123456789");
  check(0,nullptr,nullptr,nullptr,T::kObjectChart,"");
  check(SELTYPE_ROUTEPOINT,nullptr,nullptr,nullptr,T::kObjectChart,"");
}
'''
with tempfile.TemporaryDirectory() as directory:
    cpp = Path(directory) / "test.cpp"
    exe = Path(directory) / "test"
    cpp.write_text(prefix + block + main)
    subprocess.run(["g++", "-std=c++17", "-fsanitize=undefined", str(cpp), "-o", str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
print("Eight context classification cases passed")
