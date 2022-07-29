/*
N2kMsg.cpp

Copyright (c) 2015-2022 Timo Lappalainen, Kave Oy, www.kave.fi

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "N2kMsg.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
//#include <MemoryFree.h>  // For testing used memory

#define Escape 0x10
#define StartOfText 0x02
#define EndOfText 0x03
#define MsgTypeN2k 0x93

#define MaxActisenseMsgBuf 400

// NMEA2000 uses little endian for binary data. Swap the endian if we are
// running on a big endian machine. There is no reliable, portable compile
// check for this so each compiler has to be added manually.
#if defined(__GNUC__) && defined (__BYTE_ORDER__)
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define HOST_IS_BIG_ENDIAN
#endif
#endif

//*****************************************************************************
// On Arduino round() is a macro, hence the definition check. On other systems
// it is a function that may or may not be implemented so we do it ourselves.
#if !defined(round)
double round(double val) {
  return val >= 0
      ? floor(val + 0.5)
      : ceil(val - 0.5);
}
#endif

//*****************************************************************************
tN2kMsg::tN2kMsg(unsigned char _Source, unsigned char _Priority, unsigned long _PGN, int _DataLen) {
  Init(_Priority,_PGN,_Source,255);
  if ( _DataLen>0 && _DataLen<MaxDataLen ) DataLen=_DataLen;
  ResetData();
  if ( PGN!=0 ) MsgTime=millis();
}

//*****************************************************************************
void tN2kMsg::SetPGN(unsigned long _PGN) {
  Clear();
  if ( PGN==0 ) PGN=_PGN;
  MsgTime=millis();
}

//*****************************************************************************
void tN2kMsg::Init(unsigned char _Priority, unsigned long _PGN, unsigned char _Source, unsigned char _Destination) {
  DataLen=0;
  Priority=_Priority & 0x7;
  SetPGN(_PGN);
  Source=_Source;
  Destination=_Destination;
#if !defined(N2K_NO_ISO_MULTI_PACKET_SUPPORT)
  TPMessage=false;
#endif
}

//*****************************************************************************
void tN2kMsg::ResetData() {
  if ( DataLen>0 ) {
    memset(Data,0xff,DataLen);
  }
}

//*****************************************************************************
void tN2kMsg::Clear() {
  PGN=0;
  DataLen=0;
  MsgTime=0;
}

//*****************************************************************************
void tN2kMsg::AddFloat(float v, float UndefVal) {
  if (v!=UndefVal) {
    SetBufFloat(v,DataLen,Data);
  } else {
    SetBuf4ByteUInt(N2kInt32NA,DataLen,Data);
  }
}

//*****************************************************************************
void tN2kMsg::Add8ByteDouble(double v, double precision, double UndefVal) {
  if (v!=UndefVal) {
    SetBuf8ByteDouble(v,precision,DataLen,Data);
  } else {
    SetBuf4ByteUInt(N2kUInt32NA,DataLen,Data);
    SetBuf4ByteUInt(N2kInt32NA,DataLen,Data);
  }
}

//*****************************************************************************
void tN2kMsg::Add4ByteDouble(double v, double precision, double UndefVal) {
  if (v!=UndefVal) {
    SetBuf4ByteDouble(v,precision,DataLen,Data);
  } else {
    SetBuf4ByteUInt(N2kInt32NA,DataLen,Data);
  }
}

//*****************************************************************************
void tN2kMsg::Add4ByteUDouble(double v, double precision, double UndefVal) {
  if (v!=UndefVal) {
    SetBuf4ByteUDouble(v,precision,DataLen,Data);
  } else {
    SetBuf4ByteUInt(N2kUInt32NA,DataLen,Data);
  }
}

//*****************************************************************************
void tN2kMsg::Add3ByteDouble(double v, double precision, double UndefVal) {
  if (v!=UndefVal) {
    SetBuf3ByteDouble(v,precision,DataLen,Data);
  } else {
    SetBuf3ByteInt(0x7fffff,DataLen,Data);
  }
}

//*****************************************************************************
void tN2kMsg::Add2ByteDouble(double v, double precision, double UndefVal) {
  if (v!=UndefVal) {
    SetBuf2ByteDouble(v,precision,DataLen,Data);
  } else {
    SetBuf2ByteUInt(N2kInt16NA,DataLen,Data);
  }
}

//*****************************************************************************
void tN2kMsg::Add2ByteUDouble(double v, double precision, double UndefVal) {
  if (v!=UndefVal) {
    SetBuf2ByteUDouble(v,precision,DataLen,Data);
  } else {
    SetBuf2ByteUInt(N2kUInt16NA,DataLen,Data);
  }
}

//*****************************************************************************
void tN2kMsg::Add1ByteDouble(double v, double precision, double UndefVal) {
  if (v!=UndefVal) {
    SetBuf1ByteDouble(v,precision,DataLen,Data);
  } else {
    AddByte(N2kInt8NA);
  }
}

//*****************************************************************************
void tN2kMsg::Add1ByteUDouble(double v, double precision, double UndefVal) {
  if (v!=UndefVal) {
    SetBuf1ByteUDouble(v,precision,DataLen,Data);
  } else {
    AddByte(N2kUInt8NA);
  }
}

//*****************************************************************************
void tN2kMsg::Add2ByteInt(int16_t v) {
  SetBuf2ByteInt(v,DataLen,Data);
}

//*****************************************************************************
void tN2kMsg::Add2ByteUInt(uint16_t v) {
  SetBuf2ByteUInt(v,DataLen,Data);
}

//*****************************************************************************
void tN2kMsg::Add3ByteInt(int32_t v) {
  SetBuf3ByteInt(v,DataLen,Data);
}

//*****************************************************************************
void tN2kMsg::Add4ByteUInt(uint32_t v) {
  SetBuf4ByteUInt(v,DataLen,Data);
}

//*****************************************************************************
void tN2kMsg::AddUInt64(uint64_t v) {
  SetBufUInt64(v,DataLen,Data);
}

//*****************************************************************************
void tN2kMsg::AddByte(unsigned char v) {
  Data[DataLen]=v; DataLen++;
}

//*****************************************************************************
void tN2kMsg::AddStr(const char *str, int len, bool UsePgm) {
  SetBufStr(str,len,DataLen,Data,UsePgm,0xff);
}

//*****************************************************************************
void tN2kMsg::AddVarStr(const char *str, bool UsePgm) {
  int len=(str!=0?strlen(str):0);
  AddByte(len+2);
  AddByte(1);
  if ( len>0 ) SetBufStr(str,len,DataLen,Data,UsePgm,0xff);
}

//*****************************************************************************
void tN2kMsg::AddBuf(const void *buf, size_t bufLen) {
  if ( DataLen<MaxDataLen ) {
    if ( DataLen+bufLen>MaxDataLen ) bufLen=MaxDataLen-DataLen;
  } else bufLen=0;

  if ( bufLen>0 ) {
    memcpy(Data+DataLen,buf,bufLen);
    DataLen+=bufLen;
  }
}

//*****************************************************************************
unsigned char tN2kMsg::GetByte(int &Index) const {
  if (Index<DataLen) {
    return Data[Index++];
  } else return 0xff;
}

//*****************************************************************************
int16_t tN2kMsg::Get2ByteInt(int &Index, int16_t def) const {
  if (Index+2<=DataLen) {
    return GetBuf2ByteInt(Index,Data);
  } else return def;
}

//*****************************************************************************
uint16_t tN2kMsg::Get2ByteUInt(int &Index, uint16_t def) const {
  if (Index+2<=DataLen) {
    return GetBuf2ByteUInt(Index,Data);
  } else return def;
}

//*****************************************************************************
uint32_t tN2kMsg::Get3ByteUInt(int &Index, uint32_t def) const {
  if (Index+3<=DataLen) {
    return GetBuf3ByteUInt(Index,Data);
  } else return def;
}

//*****************************************************************************
uint32_t tN2kMsg::Get4ByteUInt(int &Index, uint32_t def) const {
  if (Index+4<=DataLen) {
    return GetBuf4ByteUInt(Index,Data);
  } else return def;
}

//*****************************************************************************
uint64_t tN2kMsg::GetUInt64(int &Index, uint64_t def) const {
  if (Index+8<=DataLen) {
    return GetBuf8ByteUInt(Index,Data);
  } else return def;
}

//*****************************************************************************
double tN2kMsg::Get1ByteDouble(double precision, int &Index, double def) const {
  if (Index<DataLen) {
    return GetBuf1ByteDouble(precision,Index,Data,def);
  } else return def;
}

//*****************************************************************************
double tN2kMsg::Get1ByteUDouble(double precision, int &Index, double def) const {
  if (Index<DataLen) {
    return GetBuf1ByteUDouble(precision,Index,Data,def);
  } else return def;
}

//*****************************************************************************
double tN2kMsg::Get2ByteDouble(double precision, int &Index, double def) const {
  if (Index+2<=DataLen) {
    return GetBuf2ByteDouble(precision,Index,Data,def);
  } else return def;
}

//*****************************************************************************
double tN2kMsg::Get2ByteUDouble(double precision, int &Index, double def) const {
  if (Index+2<=DataLen) {
    return GetBuf2ByteUDouble(precision,Index,Data,def);
  } else return def;
}

//*****************************************************************************
double tN2kMsg::Get3ByteDouble(double precision, int &Index, double def) const {
  if (Index+3<=DataLen) {
    return GetBuf3ByteDouble(precision,Index,Data,def);
  } else return def;
}

//*****************************************************************************
double tN2kMsg::Get4ByteDouble(double precision, int &Index, double def) const {
  if (Index+4<=DataLen) {
    return GetBuf4ByteDouble(precision,Index,Data,def);
  } else return def;
}

//*****************************************************************************
double tN2kMsg::Get4ByteUDouble(double precision, int &Index, double def) const {
  if (Index+4<=DataLen) {
    return GetBuf4ByteUDouble(precision,Index,Data,def);
  } else return def;
}

//*****************************************************************************
double tN2kMsg::Get8ByteDouble(double precision, int &Index, double def) const {
  if (Index+8<=DataLen) {
    return GetBuf8ByteDouble(precision,Index,Data,def);
  } else return def;
}

//*****************************************************************************
float  tN2kMsg::GetFloat(int &Index, float def) const {
  if (Index+4<=DataLen) {
    return GetBufFloat(Index,Data,def);
  } else return def;
}

//*****************************************************************************
bool tN2kMsg::GetStr(char *StrBuf, size_t Length, int &Index) const {
  unsigned char vb;
  bool nullReached = false;
  StrBuf[0] = '\0';
  if ((size_t)Index+Length<=(size_t)DataLen) {
    for (size_t i=0; i<Length; i++) {
      vb = GetByte(Index);
      if (! nullReached) {
        if (vb == 0x00 || vb == '@') {
          nullReached = true; // either null or '@' (AIS null character)
          StrBuf[i] = '\0';
          StrBuf[i+1] = '\0';
        } else {
          StrBuf[i] = vb;
          StrBuf[i+1] = '\0';
        }
      } else {
        StrBuf[i] = '\0';
        StrBuf[i+1] = '\0';
      }
    }
    return true;
  } else return false;
}

//*****************************************************************************
bool tN2kMsg::GetStr(size_t StrBufSize, char *StrBuf, size_t Length, unsigned char nulChar, int &Index) const {
  unsigned char vb;
  bool nullReached = false;
  if ( StrBufSize==0 || StrBuf==0 ) {
    Index+=Length;
    return true;
  }
  StrBuf[0] = '\0';
  if ((size_t)Index+Length<=(size_t)DataLen) {
    size_t i;
    for (i=0; i<Length && i<StrBufSize-1; i++) {
      vb = GetByte(Index);
      if (! nullReached) {
        if (vb == 0x00 || vb == nulChar ) {
          nullReached = true; // either null or '@' (AIS null character)
          StrBuf[i] = '\0';
        } else {
          StrBuf[i] = vb;
        }
      } else {
        StrBuf[i] = '\0';
      }
    }
    StrBuf[i] = '\0';
    for (;i<Length;i++) GetByte(Index);  // Stopped by buffer size, so read out bytes from message
    for (;i<StrBufSize;i++) StrBuf[i] = '\0';  // Stopped by length, fill buffer with 0
    return true;
  } else return false;
}

//*****************************************************************************
bool tN2kMsg::GetVarStr(size_t &StrBufSize, char *StrBuf, int &Index) const {
  size_t Len=GetByte(Index)-2;
  uint8_t Type=GetByte(Index);
  if ( Type!=0x01 ) { StrBufSize=0; return false; }
  if ( StrBuf!=0 ) {
    GetStr(StrBufSize,StrBuf,Len,0xff,Index);
  } else {
    Index+=Len; // Just pass this string
  }
  StrBufSize=Len;
  return true;
}

//*****************************************************************************
bool tN2kMsg::GetBuf(void *buf, size_t Length, int &Index) const {
  bool ret=true;

  if ((size_t)Index+Length<=(size_t)DataLen) {
    if ( buf!=0 ) {
      memcpy(buf,Data+Index,Length);
    } else {
      Index+=Length; // Just pass this string
    }
  } else {
    Index=DataLen;
    ret=false;
  }
  return ret;
}

//*****************************************************************************
bool tN2kMsg::SetByte(uint8_t v, int &Index) {
  if (Index<DataLen) {
    Data[Index]=v;
    Index++;
    return true;
  } else
    return false;
}

//*****************************************************************************
bool tN2kMsg::Set2ByteUInt(uint16_t v, int &Index) {
  if (Index+1<DataLen) {
    SetBuf2ByteUInt(v,Index,Data);
    return true;
  } else
    return false;
}

//*****************************************************************************
template<typename T>
T byteswap(T val);

template<>
uint8_t byteswap(uint8_t val) {
  return val;
}

template<>
int8_t byteswap(int8_t val) {
  return val;
}

template<>
uint16_t byteswap(uint16_t val) {
  return (val << 8) | (val >> 8);
}

template<>
int16_t byteswap(int16_t val)
{
  return byteswap((uint16_t) val);
}

template<>
uint32_t byteswap(uint32_t val) {
  return ((val << 24)) |
         ((val << 8)  & 0xff0000UL) |
         ((val >> 8)  & 0xff00UL) |
         ((val >> 24));
}

template<>
int32_t byteswap(int32_t val) {
  return byteswap((uint32_t) val);
}

template<>
uint64_t byteswap(uint64_t val) {
  return ((val << 56)) |
      ((val << 40) & 0xff000000000000ULL) |
      ((val << 24) & 0xff0000000000ULL) |
      ((val << 8)  & 0xff00000000ULL) |
      ((val >> 8)  & 0xff000000ULL) |
      ((val >> 24) & 0xff0000ULL) |
      ((val >> 40) & 0xff00ULL) |
      ((val >> 56));
}

template<>
int64_t byteswap(int64_t val) {
  return byteswap((uint64_t) val);
}

//*****************************************************************************
template<typename T>
T GetBuf(size_t len, int& index, const unsigned char* buf) {
  T v{0};

  // This could be improved by casting the buffer to a pointer of T and
  // doing a direct copy. That is, if unaligned data access is allowed.
  memcpy(&v, &buf[index], len);
  index += len;

#if defined(HOST_IS_BIG_ENDIAN)
  v = byteswap(v);
#endif

  return v;
}

//*****************************************************************************
template<typename T>
void SetBuf(T v, size_t len, int& index, unsigned char* buf) {
#if defined(HOST_IS_BIG_ENDIAN)
  v = byteswap(v);
#endif

  // This could be improved by casting the buffer to a pointer of T and
  // doing a direct copy. That is, if unaligned data access is allowed.
  memcpy(&buf[index], &v, len);
  index += len;
}

//*****************************************************************************
void SetBufDouble(double v, int &index, unsigned char *buf) {
  if ( sizeof(double)==8 && !N2kIsNA(v) ) {
    int64_t iv;
    memcpy(&iv,&v,8);
    SetBuf(iv, 8, index, buf);
  } else {  // on AVR double=float
    SetBuf((int64_t)N2kInt64NA,8,index,buf);
  }
}

//*****************************************************************************
void SetBufFloat(float v, int &index, unsigned char *buf) {
  int32_t iv;
  if ( !N2kIsNA(v) ) {
    memcpy(&iv,&v,4);
  } else {
    iv=N2kInt32NA;
  }
  SetBuf(iv, 4, index, buf);
}

#define N2kInt8OR 0x7e
#define N2kUInt8OR 0xfe
#define N2kInt16OR 0x7ffe
#define N2kUInt16OR 0xfffe
#define N2kInt32OR 0x7ffffffe
#define N2kUInt32OR 0xfffffffe

#define N2kInt32Min -2147483648L
#define N2kInt24OR  8388606L
#define N2kInt24Min -8388608L
#define N2kInt16Min -32768
#define N2kInt8Min  -128

//*****************************************************************************
void SetBuf8ByteDouble(double v, double precision, int &index, unsigned char *buf) {
  int64_t vll;
  if ( !N2kIsNA(v) ) {
    if ( sizeof(double)<8 ) {
      double fp=precision*1e6;
      int64_t fpll=1/fp;
      vll=v*1e6L;
      vll*=fpll;
    } else {
      vll=v/precision;
    }
  } else {
    vll=N2kInt64NA;
  }
  SetBuf(vll, 8, index, buf);
}

//*****************************************************************************
void SetBuf4ByteDouble(double v, double precision, int &index, unsigned char *buf) {
  double vd=round(v/precision);
  int32_t vi = (vd>=N2kInt32Min && vd<N2kInt32OR)?(int32_t)vd:N2kInt32OR;
  SetBuf<int32_t>(vi, 4, index, buf);
}

//*****************************************************************************
void SetBuf4ByteUDouble(double v, double precision, int &index, unsigned char *buf) {
  double vd=round(v/precision);
  uint32_t vi = (vd>=0 && vd<N2kUInt32OR)?(uint32_t)vd:N2kUInt32OR;
  SetBuf<uint32_t>(vi, 4, index, buf);
}

//*****************************************************************************
void SetBuf3ByteDouble(double v, double precision, int &index, unsigned char *buf) {
  double vd=round(v/precision);
  int32_t vi = (vd>=N2kInt24Min && vd<N2kInt24OR)?(int32_t)vd:N2kInt24OR;
  SetBuf<int32_t>(vi, 3, index, buf);
}

//*****************************************************************************
int16_t GetBuf2ByteInt(int &index, const unsigned char *buf) {
  return GetBuf<int16_t>(2, index, buf);
}

//*****************************************************************************
uint16_t GetBuf2ByteUInt(int &index, const unsigned char *buf) {
  return GetBuf<uint16_t>(2, index, buf);
}

//*****************************************************************************
uint32_t GetBuf3ByteUInt(int &index, const unsigned char *buf) {
  return GetBuf<uint32_t>(3, index, buf);
}

//*****************************************************************************
uint32_t GetBuf4ByteUInt(int &index, const unsigned char *buf) {
  return GetBuf<uint32_t>(4, index, buf);
}

//*****************************************************************************
uint64_t GetBuf8ByteUInt(int &index, const unsigned char *buf) {
  return GetBuf<uint64_t>(8, index, buf);
}

//*****************************************************************************
double GetBuf1ByteDouble(double precision, int &index, const unsigned char *buf, double def) {
  int8_t vl = GetBuf<int8_t>(1, index, buf);
  if (vl==0x7f) return def;

  return vl * precision;
}

//*****************************************************************************
double GetBuf1ByteUDouble(double precision, int &index, const unsigned char *buf, double def) {
  uint8_t vl = GetBuf<uint8_t>(1, index, buf);
  if (vl==0xff) return def;

  return vl * precision;
}

//*****************************************************************************
double GetBuf2ByteDouble(double precision, int &index, const unsigned char *buf, double def) {
  int16_t vl = GetBuf<int16_t>(2, index, buf);
  if (vl==0x7fff) return def;

  return vl * precision;
}

//*****************************************************************************
double GetBuf2ByteUDouble(double precision, int &index, const unsigned char *buf, double def) {
  uint16_t vl = GetBuf<uint16_t>(2, index, buf);
  if (vl==0xffff) return def;

  return vl * precision;
}

//*****************************************************************************
double GetBuf8ByteDouble(double precision, int &index, const unsigned char *buf, double def) {
  int64_t vl = GetBuf<int64_t>(8, index, buf);
  if (vl==0x7fffffffffffffffLL) return def;

  return vl * precision;
}

//*****************************************************************************
double GetBufDouble(int &index, const unsigned char *buf, double def) {
  int64_t vl = GetBuf<int64_t>(8, index, buf);
  double ret;
   // On avr double==float, so we test it also. Currently no handling for avr.
  if ( sizeof(double)==8 && !N2kIsNA(vl) ) {
    memcpy(&ret,&vl,8);
    if ( isnan(ret) ) ret=def;
  } else {
    ret=def;
  }
  return ret;
}

//*****************************************************************************
float GetBufFloat(int &index, const unsigned char *buf, float def) {
  int32_t vl = GetBuf<int32_t>(4, index, buf);
  float ret;
  if ( !N2kIsNA(vl) ) {
    memcpy(&ret,&vl,4);
    if ( isnan(ret) ) ret=def;
  } else { // On avr double==float
    ret=def;
  }
  return ret;
}

//*****************************************************************************
double GetBuf3ByteDouble(double precision, int &index, const unsigned char *buf, double def) {
  int32_t vl = GetBuf<int32_t>(3, index, buf);
  if (vl==0x007fffff) return def;

  return vl * precision;
}

//*****************************************************************************
double GetBuf4ByteDouble(double precision, int &index, const unsigned char *buf, double def) {
  int32_t vl = GetBuf<int32_t>(4, index, buf);
  if (vl==0x7fffffff) return def;

  return vl * precision;
}

//*****************************************************************************
double GetBuf4ByteUDouble(double precision, int &index, const unsigned char *buf, double def) {
  uint32_t vl = GetBuf<uint32_t>(4, index, buf);
  if (vl==0xffffffff) return def;

  return vl * precision;
}

//*****************************************************************************
void SetBuf2ByteDouble(double v, double precision, int &index, unsigned char *buf) {
  double vd=round(v/precision);
  int16_t vi = (vd>=N2kInt16Min && vd<N2kInt16OR)?(int16_t)vd:N2kInt16OR;
  SetBuf(vi, 2, index, buf);
}

//*****************************************************************************
void SetBuf2ByteUDouble(double v, double precision, int &index, unsigned char *buf) {
  double vd=round(v/precision);
  uint16_t vi = (vd>=0 && vd<N2kUInt16OR)?(uint16_t)vd:N2kUInt16OR;
  SetBuf(vi, 2, index, buf);
}

//*****************************************************************************
void SetBuf1ByteDouble(double v, double precision, int &index, unsigned char *buf) {
  double vd=round(v/precision);
  int8_t vi = (vd>=N2kInt8Min && vd<N2kInt8OR)?(int8_t)vd:N2kInt8OR;
  SetBuf(vi, 1, index, buf);
}

//*****************************************************************************
void SetBuf1ByteUDouble(double v, double precision, int &index, unsigned char *buf) {
  double vd=round(v/precision);
  uint8_t vi = (vd>=0 && vd<N2kUInt8OR)?(uint8_t)vd:N2kUInt8OR;
  SetBuf(vi, 1, index, buf);
}

//*****************************************************************************
void SetBuf2ByteInt(int16_t v, int &index, unsigned char *buf) {
  SetBuf(v, 2, index, buf);
}

//*****************************************************************************
void SetBuf2ByteUInt(uint16_t v, int &index, unsigned char *buf) {
  SetBuf(v, 2, index, buf);
}

//*****************************************************************************
void SetBuf3ByteInt(int32_t v, int &index, unsigned char *buf) {
  SetBuf(v, 3, index, buf);
}

//*****************************************************************************
void SetBuf4ByteUInt(uint32_t v, int &index, unsigned char *buf) {
  SetBuf(v, 4, index, buf);
}

//*****************************************************************************
void SetBufUInt64(uint64_t v, int &index, unsigned char *buf) {
  SetBuf(v, 8, index, buf);
}

//*****************************************************************************
void SetBufStr(const char *str, int len, int &index, unsigned char *buf, bool UsePgm, unsigned char fillChar) {
  int i=0;
  if ( UsePgm ) {
    for (; i<len && str[i]!=0; i++, index++) {
      buf[index]=pgm_read_byte(&(str[i]));
    }
  } else {
    for (; i<len && str[i]!=0; i++, index++) {
      buf[index]=str[i];
    }
  }
  for (; i<len; i++, index++) {
    buf[index]=fillChar;
  }
}

#if 0 //DSR
//*****************************************************************************
void PrintBuf(N2kStream *port, unsigned char len, const unsigned char *pData, bool AddLF) {
  if (port==0) return;

  for(int i = 0; i<len; i++) {
    if (i>0) { port->print(F(",")); };
    // Print bytes as hex.
    port->print(pData[i], 16);
  }

  if (AddLF) port->println(F(""));
}

//*****************************************************************************
void tN2kMsg::Print(N2kStream *port, bool NoData) const {
  if (port==0 || !IsValid()) return;
  port->print(millis()); port->print(F(" : "));
  port->print(F("Pri:")); port->print(Priority);
  port->print(F(" PGN:")); port->print(PGN);
  port->print(F(" Source:")); port->print(Source);
  port->print(F(" Dest:")); port->print(Destination);
  port->print(F(" Len:")); port->print(DataLen);
  if (!NoData) {
    port->print(F(" Data:"));
    PrintBuf(port,DataLen,Data);
  }
  port->println(F(""));
}
#endif

//*****************************************************************************
void AddByteEscapedToBuf(unsigned char byteToAdd, uint8_t &idx, unsigned char *buf, int &byteSum)
{
  buf[idx++]=byteToAdd;
  byteSum+=byteToAdd;

  if (byteToAdd == Escape) {
    buf[idx++]=Escape;
  }
}

#if 0  //DSR
//*****************************************************************************
// Actisense Format:
// <10><02><93><length (1)><priority (1)><PGN (3)><destination (1)><source (1)><time (4)><len (1)><data (len)><CRC (1)><10><03>
void tN2kMsg::SendInActisenseFormat(N2kStream *port) const {
  unsigned long _PGN=PGN;
  unsigned long _MsgTime=MsgTime;
  uint8_t msgIdx=0;
  int byteSum = 0;
  uint8_t CheckSum;
  unsigned char ActisenseMsgBuf[MaxActisenseMsgBuf];

  if (port==0 || !IsValid()) return;
  // Serial.print("freeMemory()="); Serial.println(freeMemory());

  ActisenseMsgBuf[msgIdx++]=Escape;
  ActisenseMsgBuf[msgIdx++]=StartOfText;
  AddByteEscapedToBuf(MsgTypeN2k,msgIdx,ActisenseMsgBuf,byteSum);
  AddByteEscapedToBuf(DataLen+11,msgIdx,ActisenseMsgBuf,byteSum); //length does not include escaped chars
  AddByteEscapedToBuf(Priority,msgIdx,ActisenseMsgBuf,byteSum);
  AddByteEscapedToBuf(_PGN & 0xff,msgIdx,ActisenseMsgBuf,byteSum); _PGN>>=8;
  AddByteEscapedToBuf(_PGN & 0xff,msgIdx,ActisenseMsgBuf,byteSum); _PGN>>=8;
  AddByteEscapedToBuf(_PGN & 0xff,msgIdx,ActisenseMsgBuf,byteSum);
  AddByteEscapedToBuf(Destination,msgIdx,ActisenseMsgBuf,byteSum);
  AddByteEscapedToBuf(Source,msgIdx,ActisenseMsgBuf,byteSum);
  // Time?
  AddByteEscapedToBuf(_MsgTime & 0xff,msgIdx,ActisenseMsgBuf,byteSum); _MsgTime>>=8;
  AddByteEscapedToBuf(_MsgTime & 0xff,msgIdx,ActisenseMsgBuf,byteSum); _MsgTime>>=8;
  AddByteEscapedToBuf(_MsgTime & 0xff,msgIdx,ActisenseMsgBuf,byteSum); _MsgTime>>=8;
  AddByteEscapedToBuf(_MsgTime & 0xff,msgIdx,ActisenseMsgBuf,byteSum);
  AddByteEscapedToBuf(DataLen,msgIdx,ActisenseMsgBuf,byteSum);


  for (int i = 0; i < DataLen; i++) AddByteEscapedToBuf(Data[i],msgIdx,ActisenseMsgBuf,byteSum);
  byteSum %= 256;

  CheckSum = (uint8_t)((byteSum == 0) ? 0 : (256 - byteSum));
  ActisenseMsgBuf[msgIdx++]=CheckSum;
  if (CheckSum==Escape) ActisenseMsgBuf[msgIdx++]=CheckSum;

  ActisenseMsgBuf[msgIdx++] = Escape;
  ActisenseMsgBuf[msgIdx++] = EndOfText;

//  if ( port->availableForWrite()>msgIdx ) {  // 16.7.2017 did not work yet
    port->write(ActisenseMsgBuf,msgIdx);
//  }
    //Serial.print("Actisense data:");
    //PrintBuf(msgIdx,ActisenseMsgBuf);
    //Serial.print("\r\n");
}
#endif
