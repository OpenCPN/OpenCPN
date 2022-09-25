/*
N2kMsg.h

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

Definition for NMEA2000 message class used in my NMEA2000 libraries.

*/

#ifndef _tN2kMsg_H_
#define _tN2kMsg_H_

//#include "N2kStream.h"
#include "N2kDef.h"
#include <stdint.h>
#include <stddef.h>

const double   N2kDoubleNA=-1e9;
const float    N2kFloatNA=-1e9;
const uint8_t  N2kUInt8NA=0xff;
const int8_t   N2kInt8NA=0x7f;
const uint16_t N2kUInt16NA=0xffff;
const int16_t  N2kInt16NA=0x7fff;
const uint32_t N2kUInt32NA=0xffffffff;
const int32_t  N2kInt32NA=0x7fffffff;
const uint64_t N2kUInt64NA=0xffffffffffffffffLL;
const int64_t  N2kInt64NA=0x7fffffffffffffffLL;

#ifndef BIT
#define BIT(n) (1 << n)
#endif

inline bool N2kIsNA(double v) { return v==N2kDoubleNA; }
inline bool N2kIsNA(float v) { return v==N2kFloatNA; }
inline bool N2kIsNA(uint8_t v) { return v==N2kUInt8NA; }
inline bool N2kIsNA(int8_t v) { return v==N2kInt8NA; }
inline bool N2kIsNA(uint16_t v) { return v==N2kUInt16NA; }
inline bool N2kIsNA(int16_t v) { return v==N2kInt16NA; }
inline bool N2kIsNA(uint32_t v) { return v==N2kUInt32NA; }
inline bool N2kIsNA(int32_t v) { return v==N2kInt32NA; }
inline bool N2kIsNA(uint64_t v) { return v==N2kUInt64NA; }
inline bool N2kIsNA(int64_t v) { return v==N2kInt64NA; }

void SetBufFloat(float v, int &index, unsigned char *buf);
void SetBufDouble(double v, int &index, unsigned char *buf);
void SetBuf8ByteDouble(double v, double precision, int &index, unsigned char *buf);
void SetBuf4ByteDouble(double v, double precision, int &index, unsigned char *buf);
void SetBuf4ByteUDouble(double v, double precision, int &index, unsigned char *buf);
void SetBuf3ByteDouble(double v, double precision, int &index, unsigned char *buf);
void SetBuf2ByteDouble(double v, double precision, int &index, unsigned char *buf);
void SetBuf2ByteUDouble(double v, double precision, int &index, unsigned char *buf);
void SetBuf1ByteDouble(double v, double precision, int &index, unsigned char *buf);
void SetBuf1ByteUDouble(double v, double precision, int &index, unsigned char *buf);
void SetBuf2ByteInt(int16_t v, int &index, unsigned char *buf);
void SetBuf2ByteUInt(uint16_t v, int &index, unsigned char *buf);
void SetBuf3ByteInt(int32_t v, int &index, unsigned char *buf);
void SetBuf4ByteUInt(uint32_t v, int &index, unsigned char *buf);
void SetBufUInt64(uint64_t v, int &index, unsigned char *buf);
void SetBufStr(const char *str, int len, int &index, unsigned char *buf, bool UsePgm=false, unsigned char fillChar=0x0);

int16_t GetBuf2ByteInt(int &index, const unsigned char *buf);
uint16_t GetBuf2ByteUInt(int &index, const unsigned char *buf);
uint32_t GetBuf3ByteUInt(int &index, const unsigned char *buf);
uint32_t GetBuf4ByteUInt(int &index, const unsigned char *buf);
uint64_t GetBuf8ByteUInt(int &index, const unsigned char *buf);
double GetBuf1ByteDouble(double precision, int &index, const unsigned char *buf, double def=0);
double GetBuf1ByteUDouble(double precision, int &index, const unsigned char *buf, double def=-1);
double GetBuf2ByteDouble(double precision, int &index, const unsigned char *buf, double def=0);
double GetBuf2ByteUDouble(double precision, int &index, const unsigned char *buf, double def=-1);
double GetBuf3ByteDouble(double precision, int &index, const unsigned char *buf, double def=0);
double GetBuf4ByteDouble(double precision, int &index, const unsigned char *buf, double def=0);
double GetBuf4ByteUDouble(double precision, int &index, const unsigned char *buf, double def=-1);
double GetBuf8ByteDouble(double precision, int &index, const unsigned char *buf, double def=0);
double GetBufDouble(int &index, const unsigned char *buf, double def=0);
float GetBufFloat(int &index, const unsigned char *buf, float def=0);

class tN2kMsg
{
public:
  static const int MaxDataLen=223; // with fast packet 1. frame can have 6 byte and rest 31 frames 7 bytes
  unsigned char Priority;
  unsigned long PGN;
  mutable unsigned char Source;
  mutable unsigned char Destination;
  int DataLen;
  unsigned char Data[MaxDataLen];
  unsigned long MsgTime;
protected:
  void ResetData();
#if !defined(N2K_NO_ISO_MULTI_PACKET_SUPPORT)
protected:
  bool TPMessage;
public:
  void SetIsTPMessage(bool tp=true) { TPMessage=tp; }
  bool IsTPMessage() const { return TPMessage; }
#endif
public:
  tN2kMsg(unsigned char _Source=15, unsigned char _Priority=6, unsigned long _PGN=0, int _DataLen=0);
  void SetPGN(unsigned long _PGN);
  void ForceSource(unsigned char _Source) const { Source=_Source; }
  void CheckDestination() const { if ( (PGN & 0xff)!=0 ) Destination=0xff; } // We can send to specified destination only for PGN:s low byte=0
  void Init(unsigned char _Priority, unsigned long _PGN, unsigned char _Source, unsigned char _Destination=0xff);
  virtual void Clear();
  bool IsValid() const { return (PGN!=0 && DataLen>0); }

  int GetRemainingDataLength(int Index) const { return DataLen>Index?DataLen-Index:0; }
  int GetAvailableDataLength() const { return MaxDataLen-DataLen; }

  void AddFloat(float v, float UndefVal=N2kFloatNA);
  void Add8ByteDouble(double v, double precision, double UndefVal=N2kDoubleNA);
  void Add4ByteDouble(double v, double precision, double UndefVal=N2kDoubleNA);
  void Add4ByteUDouble(double v, double precision, double UndefVal=N2kDoubleNA);
  void Add3ByteDouble(double v, double precision, double UndefVal=N2kDoubleNA);
  void Add2ByteUDouble(double v, double precision, double UndefVal=N2kDoubleNA);
  void Add2ByteDouble(double v, double precision, double UndefVal=N2kDoubleNA);
  void Add1ByteDouble(double v, double precision, double UndefVal=N2kDoubleNA);
  void Add1ByteUDouble(double v, double precision, double UndefVal=N2kDoubleNA);
  void Add2ByteInt(int16_t v);
  void Add2ByteUInt(uint16_t v);
  void Add3ByteInt(int32_t v);
  void Add4ByteUInt(uint32_t v);
  void AddUInt64(uint64_t v);
  void AddByte(unsigned char v);
  void AddStr(const char *str, int len, bool UsePgm=false);
  void AddVarStr(const char *str, bool UsePgm=false);
  void AddBuf(const void *buf, size_t bufLen);

  unsigned char GetByte(int &Index) const;
  int16_t Get2ByteInt(int &Index, int16_t def=0x7fff) const;
  uint16_t Get2ByteUInt(int &Index, uint16_t def=0xffff) const;
  uint32_t Get3ByteUInt(int &Index, uint32_t def=0xffffffff) const;
  uint32_t Get4ByteUInt(int &Index, uint32_t def=0xffffffff) const;
  uint64_t GetUInt64(int &Index, uint64_t def=0xffffffffffffffffULL) const;
  double Get1ByteDouble(double precision, int &Index, double def=N2kDoubleNA) const;
  double Get1ByteUDouble(double precision, int &Index, double def=N2kDoubleNA) const;
  double Get2ByteDouble(double precision, int &Index, double def=N2kDoubleNA) const;
  double Get2ByteUDouble(double precision, int &Index, double def=N2kDoubleNA) const;
  double Get3ByteDouble(double precision, int &Index, double def=N2kDoubleNA) const;
  double Get4ByteDouble(double precision, int &Index, double def=N2kDoubleNA) const;
  double Get4ByteUDouble(double precision, int &Index, double def=N2kDoubleNA) const;
  double Get8ByteDouble(double precision, int &Index, double def=N2kDoubleNA) const;
  float  GetFloat(int &Index, float def=N2kFloatNA) const;
  bool GetStr(char *StrBuf, size_t Length, int &Index) const;
  bool GetStr(size_t StrBufSize, char *StrBuf, size_t Length, unsigned char nulChar, int &Index) const;
  bool GetVarStr(size_t &StrBufSize, char *StrBuf, int &Index) const;
  bool GetBuf(void *buf, size_t Length, int &Index) const;

  bool SetByte(uint8_t v, int &Index);
  bool Set2ByteUInt(uint16_t v, int &Index);

  //void Print(N2kStream *port, bool NoData=false) const;
  //void SendInActisenseFormat(N2kStream *port) const;
};

//void PrintBuf(N2kStream *port, unsigned char len, const unsigned char *pData, bool AddLF=false);

#endif
