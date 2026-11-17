# SPDX-License-Identifier: GPL-2.0-or-later
#!/usr/bin/env python3
"""Exercise the production API function with a bounds-checking canvas double.

Run: python3 test/check_canvas_index.py [repository-root]
Requires a C++17 compiler. This is not a loaded-plugin/GUI integration test.
"""
from pathlib import Path
import subprocess
import sys
import tempfile

repo = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[1]
source = (repo / "gui/src/api_121.cpp").read_text()
start = source.index("static std::shared_ptr<HostApi121::PiPointContext> GetContextAtPoint(")
function = source[start:source.index("\nstatic wxBitmap GetObjectIcon_PlugIn", start)]
prefix = r'''
#include <cassert>
#include <climits>
#include <memory>
#include <string>
#include <vector>
namespace HostApi121 {
enum class PiContextObjectType { kObjectUnknown, kObjectChart };
struct PiPointContext { PiContextObjectType object_type; std::string object_ident; };
}
struct ChartCanvas {
  int calls = 0;
  std::shared_ptr<HostApi121::PiPointContext> GetCanvasContextAtPoint(int x,int y) {
    assert(x==12 && y==34); ++calls;
    return std::make_shared<HostApi121::PiPointContext>(
        HostApi121::PiPointContext{HostApi121::PiContextObjectType::kObjectChart,"delegated"});
  }
};
struct CanvasArray {
  std::vector<ChartCanvas*> values;
  size_t GetCount() { return values.size(); }
  ChartCanvas* Item(size_t index) { return values.at(index); }
} g_canvasArray;
'''
main = r'''
int main() {
  auto unknown=[](int i) {
    auto r=GetContextAtPoint(12,34,i);
    assert(r && r->object_type==HostApi121::PiContextObjectType::kObjectUnknown);
    assert(r->object_ident.empty());
  };
  unknown(0); unknown(-1); unknown(INT_MIN); unknown(INT_MAX);
  ChartCanvas first,second;
  g_canvasArray.values={&first,nullptr,&second};
  unknown(-1); unknown(3); unknown(INT_MAX); unknown(1);
  for (int i: {0,2}) assert(GetContextAtPoint(12,34,i)->object_ident=="delegated");
  assert(first.calls==1 && second.calls==1);
}
'''
with tempfile.TemporaryDirectory() as directory:
    cpp = Path(directory) / "test.cpp"
    exe = Path(directory) / "test"
    cpp.write_text(prefix + function + main)
    subprocess.run(["g++", "-std=c++17", "-fsanitize=undefined", str(cpp), "-o", str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
print("Eight invalid/null cases and two valid delegation cases passed")
