# SPDX-License-Identifier: GPL-2.0-or-later
#!/usr/bin/env python3
"""Exercise both production plugin dispatch loops with typed plugin doubles.

Run: python3 test/check_tide_dispatch.py [repository-root]
Requires g++. Does not load real plugins, render charts or operate hardware.
"""
from pathlib import Path
import subprocess
import sys
import tempfile

repo = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[1]
source = (repo / "gui/src/chcanv.cpp").read_text()
section = source[source.index("    //  Tide and current points"):]
blocks = []
cursor = 0
for _ in range(2):
    start = section.index("        auto plugin_array =", cursor)
    end = section.index("        // Default action", start)
    blocks.append(section[start:end])
    cursor = end
prefix = r'''
#include <cassert>
#include <ctime>
#include <functional>
#include <string>
#include <vector>
enum { CURRENT_STATION=1,TIDE_STATION=2,WANTS_TIDECURRENT_CLICK=4 };
struct TCClickInfo {
  int point_type,index,tz_offset_minutes; std::string name;
  std::function<bool(time_t,int,float&,float&)> getTide;
};
struct opencpn_plugin {virtual ~opencpn_plugin()=default;};
struct opencpn_plugin_121:opencpn_plugin {
  int calls=0; TCClickInfo last;
  void OnTideCurrentClick(const TCClickInfo& info) {++calls;last=info;}
};
struct PlugInContainer {opencpn_plugin* m_pplugin;bool m_enabled=true,m_init_state=true;int m_cap_flag=WANTS_TIDECURRENT_CLICK;};
struct Array {std::vector<PlugInContainer*> values;size_t GetCount(){return values.size();} PlugInContainer* Item(size_t i){return values.at(i);}} plugins;
struct PluginLoader {static PluginLoader* GetInstance(){static PluginLoader p;return &p;} Array* GetPlugInArray(){return &plugins;}};
struct Manager {
 bool GetTideOrCurrentMeters(time_t,int,float& v,float& d){v=12;d=34;return true;}
 bool GetTideOrCurrent(time_t,int,float& v,float& d){v=56;d=78;return true;}
} manager;
Manager* ptcmgr=&manager;
struct Index {int IDX_rec_num=42;std::string IDX_station_name="station";int station_tz_offset=60;} station;
Index* m_pIDXCandidate=&station;
'''
main = r'''
int main() {
  for (int kind=0;kind<2;++kind) {
    auto dispatch=kind?Tide:Current;
    opencpn_plugin old; opencpn_plugin_121 good,other;
    PlugInContainer incompatible{&old},compatible{&good},second{&other};
    plugins.values={&incompatible}; assert(!dispatch());
    plugins.values={&incompatible,&compatible,&second}; assert(dispatch());
    assert(good.calls==1 && other.calls==0);
    assert(good.last.index==42 && good.last.name=="station" && good.last.tz_offset_minutes==60);
    assert(good.last.point_type==(kind?TIDE_STATION:CURRENT_STATION));
    float v=0,d=0;assert(good.last.getTide(0,42,v,d));assert(v==(kind?56:12));
    compatible.m_enabled=false;plugins.values={&compatible};assert(!dispatch());
    compatible.m_enabled=true;compatible.m_init_state=false;assert(!dispatch());
    compatible.m_init_state=true;compatible.m_cap_flag=0;assert(!dispatch());
    compatible.m_cap_flag=WANTS_TIDECURRENT_CLICK;
    ptcmgr=nullptr;assert(!dispatch());ptcmgr=&manager;
    m_pIDXCandidate=nullptr;assert(!dispatch());m_pIDXCandidate=&station;
    plugins.values.clear();assert(!dispatch());
  }
}
'''
with tempfile.TemporaryDirectory() as directory:
    cpp = Path(directory) / "test.cpp"
    exe = Path(directory) / "test"
    cpp.write_text(prefix + "bool Current(){\n" + blocks[0] + "return false;}\n"
                   + "bool Tide(){\n" + blocks[1] + "return false;}\n" + main)
    subprocess.run(["g++", "-std=c++17", "-fsanitize=undefined", str(cpp), "-o", str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
print("Both dispatch paths: incompatible/compatible ordering, fallback and eligibility checks passed")
