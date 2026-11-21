#!/usr/bin/env python3
"""Compile the production GSHHS predicate and check its geometry contract.

Only wxLineF's two coordinate pairs are replaced by a stand-in. The function
body is extracted unchanged from this checkout's gui/src/gshhs.cpp. This is a
predicate regression, not a replacement for host/API and real-data tests.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
import tempfile


def extract(source):
    start = source.index('static inline bool my_intersects(')
    cursor = source.index('{', start) + 1
    depth = 1
    while depth:
        if source[cursor] == '{':
            depth += 1
        elif source[cursor] == '}':
            depth -= 1
        cursor += 1
    return source[start:cursor]


HARNESS = r'''
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
using std::fabs;
#define wxMin std::min
#define wxMax std::max
struct Point { double x, y; };
struct wxLineF { Point m_p1, m_p2; };
@PRODUCTION@
struct IntegerPoint { int64_t x,y; };
int64_t Orient(IntegerPoint a,IntegerPoint b,IntegerPoint c) {
  return (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);
}
bool On(IntegerPoint p,IntegerPoint a,IntegerPoint b) {
  return Orient(a,b,p)==0 && p.x>=std::min(a.x,b.x) &&
    p.x<=std::max(a.x,b.x) && p.y>=std::min(a.y,b.y) &&
    p.y<=std::max(a.y,b.y);
}
bool Exact(IntegerPoint a,IntegerPoint b,IntegerPoint c,IntegerPoint d) {
  auto p=Orient(a,b,c),q=Orient(a,b,d),r=Orient(c,d,a),s=Orient(c,d,b);
  return ((((p>0&&q<0)||(p<0&&q>0))&&((r>0&&s<0)||(r<0&&s>0))) ||
    On(a,c,d)||On(b,c,d)||On(c,a,b)||On(d,a,b));
}
int main() {
  unsigned checked=0, failed=0;
  auto check=[&](wxLineF a,wxLineF b,bool expected) {
    ++checked;
    if(my_intersects(a,b)!=expected) {
      if(failed<10) std::cerr << "Mismatch at case " << checked << '\n';
      ++failed;
    }
  };
  wxLineF a{{0,0},{2,2}},b{{0,2},{2,0}};
  for(int i=0;i<4;++i) {
    auto x=a,y=b;
    if(i&1)std::swap(x.m_p1,x.m_p2);
    if(i&2)std::swap(y.m_p1,y.m_p2);
    check(x,y,true);
  }
  check({{0,0},{1,0}},{{2,0},{3,0}},false);
  check({{0,0},{1,0}},{{3,0},{2,0}},false);
  check({{0,0},{0,1}},{{0,2},{0,3}},false);
  check({{0,0},{1,0}},{{1,0},{2,0}},true);
  check({{0,0},{2,0}},{{1,0},{3,0}},true);
  check({{0,0},{1,0}},{{0,1},{1,1}},false);
  wxLineF lizard{{-5.1706755,49.9979254},{-5.1702730,49.9979394}};
  wxLineF edge{{-5.170917,49.997917},{-5.171722,49.997889}};
  check(lizard,edge,false);
  std::swap(lizard.m_p1,lizard.m_p2);
  check(lizard,edge,false);
  std::mt19937 rng(5535);
  std::uniform_int_distribution<int> grid(0,6);
  for(int i=0;i<2000000;++i) {
    IntegerPoint p{grid(rng),grid(rng)},q{grid(rng),grid(rng)};
    IntegerPoint r{grid(rng),grid(rng)},s{grid(rng),grid(rng)};
    if((p.x==q.x&&p.y==q.y)||(r.x==s.x&&r.y==s.y)){--i;continue;}
    auto cv=[](IntegerPoint p){return Point{double(p.x),double(p.y)};};
    check({cv(p),cv(q)},{cv(r),cv(s)},Exact(p,q,r,s));
  }
  // Preserve the patch's bounded scope: relative-tolerance changes and
  // degenerate segments are separate work, not assertions of this fix.
  std::cout << "{\"checked\":" << checked << ",\"failed\":" << failed << "}\n";
  return failed?1:0;
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path,
                        default=Path(__file__).resolve().parents[1] /
                        'gui/src/gshhs.cpp')
    parser.add_argument('--sanitize', action='store_true')
    args = parser.parse_args()
    predicate = extract(args.source.read_text())
    with tempfile.TemporaryDirectory(prefix='gshhs-regression-') as directory:
        cpp = Path(directory) / 'regression.cpp'
        binary = Path(directory) / 'regression'
        cpp.write_text(HARNESS.replace('@PRODUCTION@', predicate))
        flags = ['-fsanitize=address,undefined', '-fno-omit-frame-pointer'] \
            if args.sanitize else []
        subprocess.run([*shlex.split(os.environ.get('CXX', 'c++')),
                        '-std=c++17', '-O2', '-Wall', '-Wextra', '-Werror',
                        *flags, str(cpp), '-o', str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
    print(json.dumps({'source': str(args.source),
                      'predicate_sha256': hashlib.sha256(
                          predicate.encode()).hexdigest(),
                      'sanitizers': args.sanitize}))


if __name__ == '__main__':
    main()
