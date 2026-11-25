"""Compile the actual waypoint API bodies with isolated manager/conversion doubles.

This focused test does not replace compiling or exercising OpenCPN's GUI.
Usage: python3 check_waypoint_api.py source.cpp [--before]
"""
import pathlib
import subprocess
import sys
import tempfile

source = pathlib.Path(sys.argv[1]).read_text()
names = ['GetSingleWaypoint', 'GetSingleWaypointEx', 'GetSingleWaypointExV2']
bodies = []
for name in names:
    start = source.index('bool ' + name + '(')
    end = source.index('\n}', start) + 2
    bodies.append(source[start:end])
header = '''
#include <cassert>
#include <string>
using wxString = std::string;
struct RoutePoint {};
struct PlugIn_Waypoint { bool copied = false; };
struct PlugIn_Waypoint_Ex { bool copied = false; };
struct PlugIn_Waypoint_ExV2 { bool copied = false; };
struct WayPointman {
  RoutePoint point;
  int calls = 0;
  RoutePoint* FindRoutePointByGUID(const wxString& id) {
    calls++;
    return id == "known" ? &point : nullptr;
  }
};
WayPointman* pWayPointMan = nullptr;
void PlugInFromRoutePoint(PlugIn_Waypoint* out, RoutePoint*) { out->copied = true; }
void PlugInExFromRoutePoint(PlugIn_Waypoint_Ex* out, RoutePoint*) { out->copied = true; }
void PlugInExV2FromRoutePoint(PlugIn_Waypoint_ExV2* out, RoutePoint*) { out->copied = true; }
'''
tests = []
for name, typ in zip(names, ['PlugIn_Waypoint', 'PlugIn_Waypoint_Ex', 'PlugIn_Waypoint_ExV2']):
    tests.append('''{
      TYPE output;
      pWayPointMan = nullptr;
      assert(!FUNCTION("known", &output));
      assert(!output.copied);
      WayPointman manager;
      pWayPointMan = &manager;
      assert(!FUNCTION("known", nullptr));
      assert(manager.calls == 0);
      assert(!FUNCTION("missing", &output));
      assert(!output.copied);
      assert(FUNCTION("known", &output));
      assert(output.copied);
      pWayPointMan = nullptr;
    }'''.replace('TYPE', typ).replace('FUNCTION', name))
with tempfile.TemporaryDirectory() as directory:
    path = pathlib.Path(directory)
    (path / 'test.cpp').write_text(header + '\n'.join(bodies) + '\nint main() {\n' + '\n'.join(tests) + '\n}')
    subprocess.run(['g++', '-std=c++17', '-fsanitize=undefined', '-fno-sanitize-recover=all', '-g', str(path / 'test.cpp'), '-o', str(path / 'test')], check=True)
    result = subprocess.run([str(path / 'test')])
    if '--before' in sys.argv:
        assert result.returncode != 0, 'Expected the original null-manager call to fail'
        print('Original null-manager failure reproduced')
    else:
        assert result.returncode == 0
        print('All three APIs pass: absent manager, null output, missing GUID, valid GUID')
