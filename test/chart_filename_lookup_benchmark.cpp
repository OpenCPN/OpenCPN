// Standalone comparison: c++ -O2 chart_filename_lookup_benchmark.cpp
// $(wx-config --cxxflags --libs base) -o filename_lookup && ./filename_lookup
// Measures filename checks only, not complete chart database updates.
#include <wx/string.h>
#include <wx/hashmap.h>
#include <cassert>
#include <chrono>
#include <iostream>
WX_DECLARE_STRING_HASH_MAP(int, ChartCollisionsHashMap);
bool scan(const ChartCollisionsHashMap& map, const wxString& name) {
  for (auto it = map.begin(); it != map.end(); ++it)
    if (it->first.IsSameAs(name)) return true;
  return false;
}
bool lookup(const ChartCollisionsHashMap& map, const wxString& name) {
  return map.find(name) != map.end();
}
int main() {
  ChartCollisionsHashMap map;
  for (const auto& s :
       {wxString("Chart.KAP"), wxString::FromUTF8("côte.000"), wxString("")})
    map[s] = 1;
  for (const auto& s :
       {wxString("Chart.KAP"), wxString("chart.kap"),
        wxString::FromUTF8("côte.000"), wxString("missing"), wxString("")})
    assert(scan(map, s) == lookup(map, s));
  for (auto fn : {scan, lookup}) {
    map.clear();
    auto start = std::chrono::steady_clock::now();
    size_t duplicates = 0;
    for (int i = 0; i < 10000; i++) {
      auto name = wxString::Format("chart-%d.000", i);
      duplicates += fn(map, name);
      map[name] = i;
    }
    auto ms = std::chrono::duration<double, std::milli>(
                  std::chrono::steady_clock::now() - start)
                  .count();
    std::cout << (fn == scan ? "scan" : "lookup") << ": " << ms
              << " ms, duplicates=" << duplicates << "\n";
  }
}
