"""Isolated test of the actual extremum method, with a synthetic tide curve."""
from pathlib import Path
import subprocess
import sys
import tempfile
s=Path(sys.argv[1]).read_text();a=s.index('void TCMgr::GetHightOrLowTide(');b=s.index('\n}',a)+2
preamble='''
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cassert>
constexpr time_t base=1700000000;
constexpr int TC_NO_ERROR=0;
struct IDX_entry;
struct Source { int LoadHarmonicData(IDX_entry*) {return 0;} };
struct IDX_entry {
 bool IDX_Useable=true;
 Source* pDataSource=nullptr;
 time_t recent_highlow_calc_time=0, recent_high_calc_time=0, recent_low_calc_time=0;
 float recent_high_level=0, recent_low_level=0;
 time_t recent_high_time=0, recent_low_time=0;
 double max_amplitude=0;
};
int samples=0;
int yearoftimet(time_t){return 2023;}
void happy_new_year(IDX_entry*,int){}
double time2asecondary(time_t t,IDX_entry*) {
 ++samples;
 return std::sin((t-base)*2*3.141592653589793/3600);
}
struct TCMgr {
 IDX_entry* m_Combined_IDX_array[1];
 void GetHightOrLowTide(time_t,int,int,float,bool,int,float&,time_t&);
};
'''
test='''
int main(){
 for(bool firstHigh : {true,false}) {
  IDX_entry station;TCMgr mgr{{&station}};
  for(bool high : {firstHigh,!firstHigh}) {
   float value=0;time_t when=0;
   mgr.GetHightOrLowTide(base,high?600:-600,high?60:-60,0,high,0,value,when);
   assert(high ? value>0.9f : value<-0.9f);
   assert(when!=0);
   int before=samples;
   float repeated=0;time_t repeatedTime=0;
   mgr.GetHightOrLowTide(base,high?600:-600,high?60:-60,0,high,0,repeated,repeatedTime);
   assert(samples==before);
   assert(value==repeated && when==repeatedTime);
  }
 }
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.cpp').write_text(preamble+'\n'+s[a:b]+'\n'+test)
 subprocess.run(['g++','-std=c++17','-include','initializer_list','-fsanitize=undefined','-fno-sanitize-recover=all',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 r=subprocess.run([str(p/'test')]);assert (r.returncode!=0 if '--before' in sys.argv else r.returncode==0)
 print('Original cache failure reproduced' if '--before' in sys.argv else 'Both query orders and repeated-query cache hits pass')
