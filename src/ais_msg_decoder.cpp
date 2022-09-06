/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */
#include <cmath>

#ifdef __MINGW32__
#undef IPV6STRICT  // mingw FTBS fix:  missing struct ip_mreq
#include <windows.h>
#endif

#include <wx/colour.h>
#include <wx/datetime.h>
#include <wx/log.h>
#include <wx/string.h>
#include <wx/tokenzr.h>
#include <wx/utils.h>

#include "ais_msg_decoder.h"
#include "AIS_Defs.h"
#include "ocpn_types.h"

#ifdef AIS_DEBUG
static int first_rx_ticks;
static int rx_ticks;
#endif

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double *)&lNaN)
#endif

//----------------------------------------------------------------------------------
//      Decode a single AIVDO sentence to a Generic Position Report
//----------------------------------------------------------------------------------
AIS_Error AisMsgDecoder::DecodeSingleVDO(const wxString &str,
                                         GenericPosDatEx *pos,
                                         AisDecodeCtx ctx) {
  //  Make some simple tests for validity
  if (str.Len() > 100) return AIS_NMEAVDX_TOO_LONG;

  if (!NMEACheckSumOK(str)) return AIS_NMEAVDX_CHECKSUM_BAD;

  if (!pos) return AIS_GENERIC_ERROR;

  if (!ctx.accumulator) return AIS_GENERIC_ERROR;

  //  We only process AIVDO messages
  if (!str.Mid(1, 5).IsSameAs(_T("AIVDO"))) return AIS_GENERIC_ERROR;

  //  Use a tokenizer to pull out the first 4 fields
  wxStringTokenizer tkz(str, _T(","));

  wxString token;
  token = tkz.GetNextToken();  // !xxVDx

  token = tkz.GetNextToken();
  int nsentences = atoi(token.mb_str());

  token = tkz.GetNextToken();
  int isentence = atoi(token.mb_str());

  token = tkz.GetNextToken();  // skip 2 fields
  token = tkz.GetNextToken();

  wxString string_to_parse;
  string_to_parse.Clear();

  //  Fill the output structure with all NANs
  pos->kLat = NAN;
  pos->kLon = NAN;
  pos->kCog = NAN;
  pos->kSog = NAN;
  pos->kHdt = NAN;
  pos->kVar = NAN;
  pos->kHdm = NAN;

  //  Simple case first
  //  First and only part of a one-part sentence
  if ((1 == nsentences) && (1 == isentence)) {
    string_to_parse = tkz.GetNextToken();  // the encapsulated data
  }

  else if (nsentences > 1) {
    if (1 == isentence) {
      *ctx.accumulator = tkz.GetNextToken();  // the encapsulated data
    }

    else {
      ctx.accumulator->Append(tkz.GetNextToken());
    }

    if (isentence == nsentences) {  // ready to parse
      string_to_parse = *ctx.accumulator;
    }
  }

  if (string_to_parse.IsEmpty() &&
      (nsentences > 1)) {             // not ready, so return with NAN
    return AIS_INCOMPLETE_MULTIPART;  // and non-zero return
  }

  //  Create the bit accessible string
  AIS_Bitstring strbit(string_to_parse.mb_str());

  AIS_Target_Data TargetData;

  bool bdecode_result = Parse_VDXBitstring(&strbit, &TargetData, ctx);

  if (bdecode_result) {
    switch (TargetData.MID) {
      case 1:
      case 2:
      case 3:
      case 18: {
        if (!TargetData.b_positionDoubtful) {
          pos->kLat = TargetData.Lat;
          pos->kLon = TargetData.Lon;
        } else {
          pos->kLat = NAN;
          pos->kLon = NAN;
        }

        if (TargetData.COG == 360.0)
          pos->kCog = NAN;
        else
          pos->kCog = TargetData.COG;

        if (TargetData.SOG > 102.2)
          pos->kSog = NAN;
        else
          pos->kSog = TargetData.SOG;

        if ((int)TargetData.HDG == 511)
          pos->kHdt = NAN;
        else
          pos->kHdt = TargetData.HDG;

        //  VDO messages do not contain variation or magnetic heading
        pos->kVar = NAN;
        pos->kHdm = NAN;
        break;
      }
      default:
        return AIS_GENERIC_ERROR;  // unrecognised sentence
    }

    return AIS_NoError;
  } else
    return AIS_GENERIC_ERROR;
}

//----------------------------------------------------------------------------
//      Parse a NMEA VDM/VDO Bitstring
//----------------------------------------------------------------------------
bool AisMsgDecoder::Parse_VDXBitstring(AIS_Bitstring* bstr,
                                       AIS_Target_Data* ptd,
                                       AisDecodeCtx ctx) {
  bool parse_result = false;
  bool b_posn_report = false;

  wxDateTime now = wxDateTime::Now();
  now.MakeGMT();
  int message_ID = bstr->GetInt(1, 6);  // Parse on message ID
  ptd->MID = message_ID;
  ptd->MMSI =
      bstr->GetInt(9, 30);  // MMSI is always in the same spot in the bitstream

  switch (message_ID) {
    case 1:  // Position Report
    case 2:
    case 3: {
#ifdef AIS_DEBUG
      (ctx.n_msg1)++;
#endif

      ptd->NavStatus = bstr->GetInt(39, 4);
      ptd->SOG = 0.1 * (bstr->GetInt(51, 10));

      int lon = bstr->GetInt(62, 28);
      if (lon & 0x08000000)  // negative?
        lon |= 0xf0000000;
      double lon_tentative = lon / 600000.;

      int lat = bstr->GetInt(90, 27);
      if (lat & 0x04000000)  // negative?
        lat |= 0xf8000000;
      double lat_tentative = lat / 600000.;

      if ((lon_tentative <= 180.) &&
          (lat_tentative <=
           90.))  // Ship does not report Lat or Lon "unavailable"
      {
        ptd->Lon = lon_tentative;
        ptd->Lat = lat_tentative;
        ptd->b_positionDoubtful = false;
        ptd->b_positionOnceValid = true;  // Got the position at least once
        ptd->PositionReportTicks = now.GetTicks();
      } else
        ptd->b_positionDoubtful = true;

      //    decode balance of message....
      ptd->COG = 0.1 * (bstr->GetInt(117, 12));
      ptd->HDG = 1.0 * (bstr->GetInt(129, 9));

      ptd->ROTAIS = bstr->GetInt(43, 8);
      double rot_dir = 1.0;

      if (ptd->ROTAIS == 128)
        ptd->ROTAIS = -128;  // not available codes as -128
      else if ((ptd->ROTAIS & 0x80) == 0x80) {
        ptd->ROTAIS = ptd->ROTAIS - 256;  // convert to twos complement
        rot_dir = -1.0;
      }

      ptd->ROTIND = wxRound(rot_dir * pow((((double)ptd->ROTAIS) / 4.733),
                                          2));  // Convert to indicated ROT

      ptd->m_utc_sec = bstr->GetInt(138, 6);

      if ((1 == message_ID) ||
          (2 == message_ID))  // decode SOTDMA per 7.6.7.2.2
      {
        ptd->SyncState = bstr->GetInt(151, 2);
        ptd->SlotTO = bstr->GetInt(153, 2);
        if ((ptd->SlotTO == 1) && (ptd->SyncState == 0))  // UTCDirect follows
        {
          ptd->m_utc_hour = bstr->GetInt(155, 5);

          ptd->m_utc_min = bstr->GetInt(160, 7);

          if ((ptd->m_utc_hour < 24) && (ptd->m_utc_min < 60) &&
              (ptd->m_utc_sec < 60)) {
            wxDateTime rx_time(ptd->m_utc_hour, ptd->m_utc_min, ptd->m_utc_sec);
#ifdef AIS_DEBUG
            rx_ticks = rx_time.GetTicks();
            if (!b_firstrx) {
              first_rx_ticks = rx_ticks;
              b_firstrx = true;
            }
#endif
          }
        }
      }

      //    Capture Euro Inland special passing arrangement signal ("stbd-stbd")
      ptd->blue_paddle = bstr->GetInt(144, 2);
      ptd->b_blue_paddle = (ptd->blue_paddle == 2);  // paddle is set

      if (!ptd->b_isDSCtarget)
        ptd->Class = AIS_CLASS_A;

      //    Check for SART and friends by looking at first two digits of MMSI
      int mmsi_start = ptd->MMSI / 10000000;

      if (mmsi_start == 97) {
        ptd->Class = AIS_SART;
        ptd->StaticReportTicks =
            now.GetTicks();  // won't get a static report, so fake it here

        //    On receipt of Msg 3, force any existing SART target out of
        //    acknowledge mode by adjusting its ack_time to yesterday This will
        //    cause any previously "Acknowledged" SART to re-alert.

        //    On reflection, re-alerting seems a little excessive in real life
        //    use. After all, the target is on-screen, and in the AIS target
        //    list. So lets just honor the programmed ACK timout value for SART
        //    targets as well
        // ptd->m_ack_time = wxDateTime::Now() - wxTimeSpan::Day();
      }

      parse_result = true;  // so far so good
      b_posn_report = true;

      break;
    }

    case 18: {
      ptd->NavStatus =
          UNDEFINED;  // Class B targets have no status.  Enforce this...

      ptd->SOG = 0.1 * (bstr->GetInt(47, 10));

      int lon = bstr->GetInt(58, 28);
      if (lon & 0x08000000)  // negative?
        lon |= 0xf0000000;
      double lon_tentative = lon / 600000.;

      int lat = bstr->GetInt(86, 27);
      if (lat & 0x04000000)  // negative?
        lat |= 0xf8000000;
      double lat_tentative = lat / 600000.;

      if ((lon_tentative <= 180.) &&
          (lat_tentative <=
           90.))  // Ship does not report Lat or Lon "unavailable"
      {
        ptd->Lon = lon_tentative;
        ptd->Lat = lat_tentative;
        ptd->b_positionDoubtful = false;
        ptd->b_positionOnceValid = true;  // Got the position at least once
        ptd->PositionReportTicks = now.GetTicks();
      } else
        ptd->b_positionDoubtful = true;

      ptd->COG = 0.1 * (bstr->GetInt(113, 12));
      ptd->HDG = 1.0 * (bstr->GetInt(125, 9));

      ptd->m_utc_sec = bstr->GetInt(134, 6);

      if (!ptd->b_isDSCtarget)
        ptd->Class = AIS_CLASS_B;

      parse_result = true;  // so far so good
      b_posn_report = true;

      break;
    }

    case 19: {  // Class B mes_ID 19 Is same as mes_ID 18 until bit 139
      ptd->NavStatus =
          UNDEFINED;  // Class B targets have no status.  Enforce this...
      ptd->SOG = 0.1 * (bstr->GetInt(47, 10));
      int lon = bstr->GetInt(58, 28);
      if (lon & 0x08000000)  // negative?
        lon |= 0xf0000000;
      double lon_tentative = lon / 600000.;

      int lat = bstr->GetInt(86, 27);
      if (lat & 0x04000000)  // negative?
        lat |= 0xf8000000;
      double lat_tentative = lat / 600000.;

      if ((lon_tentative <= 180.) &&
          (lat_tentative <=
           90.))  // Ship does not report Lat or Lon "unavailable"
      {
        ptd->Lon = lon_tentative;
        ptd->Lat = lat_tentative;
        ptd->b_positionDoubtful = false;
        ptd->b_positionOnceValid = true;  // Got the position at least once
        ptd->PositionReportTicks = now.GetTicks();
      } else
        ptd->b_positionDoubtful = true;

      ptd->COG = 0.1 * (bstr->GetInt(113, 12));
      ptd->HDG = 1.0 * (bstr->GetInt(125, 9));
      ptd->m_utc_sec = bstr->GetInt(134, 6);
      // From bit 140 and forward data as of mes 5
      bstr->GetStr(144, 120, &ptd->ShipName[0], 20);
      ptd->b_nameValid = true;
      if (!ptd->b_isDSCtarget) {
        ptd->ShipType = (unsigned char)bstr->GetInt(264, 8);
      }
      ptd->DimA = bstr->GetInt(272, 9);
      ptd->DimB = bstr->GetInt(281, 9);
      ptd->DimC = bstr->GetInt(290, 6);
      ptd->DimD = bstr->GetInt(296, 6);

      if (!ptd->b_isDSCtarget)
        ptd->Class = AIS_CLASS_B;
      parse_result = true;  // so far so good
      b_posn_report = true;

      break;
    }

    case 27: {
      // Long-range automatic identification system broadcast message
      // This message is used for long-range detection of AIS Class A and Class
      // B vessels (typically by satellite).

      // Define the constant to do the covertion from the internal encoded
      // position in message 27. The position is less accuate :  1/10 minute
      // position resolution.
      int bitCorrection = 10;
      int resolution = 10;

      // Default aout of bounce values.
      double lon_tentative = 181.;
      double lat_tentative = 91.;

#ifdef AIS_DEBUG
      printf("AIS Message 27 - received:\r\n");
      printf("MMSI      : %i\r\n", ptd->MMSI);
#endif

      // It can be both a CLASS A and a CLASS B vessel - We have decided for
      // CLASS A
      // TODO: Lookup to see if we have seen it as a CLASS B, and adjust.
      if (!ptd->b_isDSCtarget)
        ptd->Class = AIS_CLASS_A;

      ptd->NavStatus = bstr->GetInt(39, 4);

      int lon = bstr->GetInt(45, 18);
      int lat = bstr->GetInt(63, 17);

      lat_tentative = lat;
      lon_tentative = lon;

      // Negative latitude?
      if (lat >= (0x4000000 >> bitCorrection)) {
        lat_tentative = (0x8000000 >> bitCorrection) - lat;
        lat_tentative *= -1;
      }

      // Negative longitude?
      if (lon >= (0x8000000 >> bitCorrection)) {
        lon_tentative = (0x10000000 >> bitCorrection) - lon;
        lon_tentative *= -1;
      }

      // Decode the internal position format.
      lat_tentative = lat_tentative / resolution / 60.0;
      lon_tentative = lon_tentative / resolution / 60.0;

#ifdef AIS_DEBUG
      printf("Latitude  : %f\r\n", lat_tentative);
      printf("Longitude : %f\r\n", lon_tentative);
#endif

      // Get the latency of the position report.
      int positionLatency = bstr->GetInt(95, 1);

      if ((lon_tentative <= 180.) &&
          (lat_tentative <=
           90.))  // Ship does not report Lat or Lon "unavailable"
      {
        ptd->Lon = lon_tentative;
        ptd->Lat = lat_tentative;
        ptd->b_positionDoubtful = false;
        ptd->b_positionOnceValid = true;  // Got the position at least once
        if (positionLatency == 0) {
// The position is less than 5 seconds old.
#ifdef AIS_DEBUG
          printf("Low latency position report.\r\n");
#endif
          ptd->PositionReportTicks = now.GetTicks();
        }
      } else
        ptd->b_positionDoubtful = true;

      ptd->SOG = 1.0 * (bstr->GetInt(80, 6));
      ptd->COG = 1.0 * (bstr->GetInt(85, 9));

      b_posn_report = true;
      parse_result = true;
      break;
    }

    case 5: {
#ifdef AIS_DEBUG
      (ctx.n_msg5)++;
#endif
      if (!ptd->b_isDSCtarget)
        ptd->Class = AIS_CLASS_A;

      //          Get the AIS Version indicator
      //          0 = station compliant with Recommendation ITU-R M.1371-1
      //          1 = station compliant with Recommendation ITU-R M.1371-3
      //          2-3 = station compliant with future editions
      int AIS_version_indicator = bstr->GetInt(39, 2);
      if (AIS_version_indicator < 4) {
        ptd->IMO = bstr->GetInt(41, 30);

        bstr->GetStr(71, 42, &ptd->CallSign[0], 7);
        bstr->GetStr(113, 120, &ptd->ShipName[0], 20);
        ptd->b_nameValid = true;
        if (!ptd->b_isDSCtarget) {
          ptd->ShipType = (unsigned char)bstr->GetInt(233, 8);
        }

        ptd->DimA = bstr->GetInt(241, 9);
        ptd->DimB = bstr->GetInt(250, 9);
        ptd->DimC = bstr->GetInt(259, 6);
        ptd->DimD = bstr->GetInt(265, 6);

        ptd->ETA_Mo = bstr->GetInt(275, 4);
        ptd->ETA_Day = bstr->GetInt(279, 5);
        ptd->ETA_Hr = bstr->GetInt(284, 5);
        ptd->ETA_Min = bstr->GetInt(289, 6);

        ptd->Draft = (double)(bstr->GetInt(295, 8)) / 10.0;

        bstr->GetStr(303, 120, &ptd->Destination[0], 20);

        ptd->StaticReportTicks = now.GetTicks();

        parse_result = true;
      }

      break;
    }

    case 24: {  // Static data report
      int part_number = bstr->GetInt(39, 2);
      if (0 == part_number) {
        bstr->GetStr(41, 120, &ptd->ShipName[0], 20);
        ptd->b_nameValid = true;
        parse_result = true;
#ifdef AIS_DEBUG
        (ctx.n_msg24)++;
#endif
      } else if (1 == part_number) {
        if (!ptd->b_isDSCtarget) {
          ptd->ShipType = (unsigned char)bstr->GetInt(41, 8);
        }
        bstr->GetStr(91, 42, &ptd->CallSign[0], 7);

        ptd->DimA = bstr->GetInt(133, 9);
        ptd->DimB = bstr->GetInt(142, 9);
        ptd->DimC = bstr->GetInt(151, 6);
        ptd->DimD = bstr->GetInt(157, 6);
        parse_result = true;
      }
      break;
    }
    case 4:  // base station
    {
      ptd->Class = AIS_BASE;

      ptd->m_utc_hour = bstr->GetInt(62, 5);
      ptd->m_utc_min = bstr->GetInt(67, 6);
      ptd->m_utc_sec = bstr->GetInt(73, 6);
      //                              (79,  1);
      int lon = bstr->GetInt(80, 28);
      if (lon & 0x08000000)  // negative?
        lon |= 0xf0000000;
      double lon_tentative = lon / 600000.;

      int lat = bstr->GetInt(108, 27);
      if (lat & 0x04000000)  // negative?
        lat |= 0xf8000000;
      double lat_tentative = lat / 600000.;

      if ((lon_tentative <= 180.) &&
          (lat_tentative <=
           90.))  // Ship does not report Lat or Lon "unavailable"
      {
        ptd->Lon = lon_tentative;
        ptd->Lat = lat_tentative;
        ptd->b_positionDoubtful = false;
        ptd->b_positionOnceValid = true;  // Got the position at least once
        ptd->PositionReportTicks = now.GetTicks();
      } else
        ptd->b_positionDoubtful = true;

      ptd->COG = -1.;
      ptd->HDG = 511;
      ptd->SOG = -1.;

      parse_result = true;
      b_posn_report = true;

      break;
    }
    case 9:  // Special Position Report (Standard SAR Aircraft Position Report)
    {
      ptd->SOG = bstr->GetInt(51, 10);

      int lon = bstr->GetInt(62, 28);
      if (lon & 0x08000000)  // negative?
        lon |= 0xf0000000;
      double lon_tentative = lon / 600000.;

      int lat = bstr->GetInt(90, 27);
      if (lat & 0x04000000)  // negative?
        lat |= 0xf8000000;
      double lat_tentative = lat / 600000.;

      if ((lon_tentative <= 180.) &&
          (lat_tentative <=
           90.))  // Ship does not report Lat or Lon "unavailable"
      {
        ptd->Lon = lon_tentative;
        ptd->Lat = lat_tentative;
        ptd->b_positionDoubtful = false;
        ptd->b_positionOnceValid = true;  // Got the position at least once
        ptd->PositionReportTicks = now.GetTicks();
      } else
        ptd->b_positionDoubtful = true;

      //    decode balance of message....
      ptd->COG = 0.1 * (bstr->GetInt(117, 12));

      int alt_tent = bstr->GetInt(39, 12);
      ptd->altitude = alt_tent;

      ptd->b_SarAircraftPosnReport = true;

      parse_result = true;
      b_posn_report = true;

      break;
    }
    case 21:  // Test Message (Aid to Navigation)
    {
      ptd->ShipType = (unsigned char)bstr->GetInt(39, 5);
      ptd->IMO = 0;
      ptd->SOG = 0;
      ptd->HDG = 0;
      ptd->COG = 0;
      ptd->ROTAIS = -128;  // i.e. not available
      ptd->DimA = bstr->GetInt(220, 9);
      ptd->DimB = bstr->GetInt(229, 9);
      ptd->DimC = bstr->GetInt(238, 6);
      ptd->DimD = bstr->GetInt(244, 6);
      ptd->Draft = 0;

      ptd->m_utc_sec = bstr->GetInt(254, 6);

      int offpos = bstr->GetInt(260, 1);  // off position flag
      int virt = bstr->GetInt(270, 1);    // virtual flag

      if (virt)
        ptd->NavStatus = ATON_VIRTUAL;
      else
        ptd->NavStatus = ATON_REAL;
      if (ptd->m_utc_sec <= 59 /*&& !virt*/) {
        ptd->NavStatus += 1;
        if (offpos) ptd->NavStatus += 1;
      }

      bstr->GetStr(
          44, 120, &ptd->ShipName[0],
          20);  // short name only, extension wont fit in Ship structure

      if (bstr->GetBitCount() > 276) {
        int nx = ((bstr->GetBitCount() - 272) / 6) * 6;
        bstr->GetStr(273, nx, &ptd->ShipNameExtension[0], 14);
        ptd->ShipNameExtension[14] = 0;
      } else {
        ptd->ShipNameExtension[0] = 0;
      }

      ptd->b_nameValid = true;

      parse_result = true;  // so far so good

      ptd->Class = AIS_ATON;

      int lon = bstr->GetInt(165, 28);

      if (lon & 0x08000000)  // negative?
        lon |= 0xf0000000;
      double lon_tentative = lon / 600000.;

      int lat = bstr->GetInt(193, 27);

      if (lat & 0x04000000)  // negative?
        lat |= 0xf8000000;
      double lat_tentative = lat / 600000.;

      if ((lon_tentative <= 180.) &&
          (lat_tentative <=
           90.))  // Ship does not report Lat or Lon "unavailable"
      {
        ptd->Lon = lon_tentative;
        ptd->Lat = lat_tentative;
        ptd->b_positionDoubtful = false;
        ptd->b_positionOnceValid = true;  // Got the position at least once
        ptd->PositionReportTicks = now.GetTicks();
      } else
        ptd->b_positionDoubtful = true;

      b_posn_report = true;
      break;
    }
    case 8:  // Binary Broadcast
    {
      int dac = bstr->GetInt(41, 10);
      int fi = bstr->GetInt(51, 6);
      if (dac == 200)  // European inland
      {
        if (fi == 10)  // "Inland ship static and voyage related data"
        {
          ptd->b_isEuroInland = true;

          bstr->GetStr(57, 48, &ptd->Euro_VIN[0], 8);
          ptd->Euro_Length = ((double)bstr->GetInt(105, 13)) / 10.0;
          ptd->Euro_Beam = ((double)bstr->GetInt(118, 10)) / 10.0;
          ptd->UN_shiptype = bstr->GetInt(128, 14);
          ptd->Euro_Draft = ((double)bstr->GetInt(145, 11)) / 100.0;
          parse_result = true;
        }
      }
      if (dac == 1)  // IMO
      {
        if (fi == 22)  // Area Notice
        {
          if (bstr->GetBitCount() >= 111) {
            Ais8_001_22 an;
            an.link_id = bstr->GetInt(57, 10);
            an.notice_type = bstr->GetInt(67, 7);
            an.month = bstr->GetInt(74, 4);
            an.day = bstr->GetInt(78, 5);
            an.hour = bstr->GetInt(83, 5);
            an.minute = bstr->GetInt(88, 6);
            an.duration_minutes = bstr->GetInt(94, 18);

            wxDateTime now = wxDateTime::Now();
            now.MakeGMT();

            an.start_time.Set(an.day, wxDateTime::Month(an.month - 1),
                              now.GetYear(), an.hour, an.minute);

            // msg is not supposed to be transmitted more than a day before it
            // comes into effect, so a start_time less than a day or two away
            // might indicate a month rollover
            if (an.start_time > now + wxTimeSpan::Hours(48))
              an.start_time.Set(an.day, wxDateTime::Month(an.month - 1),
                                now.GetYear() - 1, an.hour, an.minute);

            an.expiry_time =
                an.start_time + wxTimeSpan::Minutes(an.duration_minutes);

            // msg is not supposed to be transmitted beyond expiration, so
            // taking into account a fudge factor for clock issues, assume an
            // expiry date in the past indicates incorrect year
            if (an.expiry_time < now - wxTimeSpan::Hours(24)) {
              an.start_time.Set(an.day, wxDateTime::Month(an.month - 1),
                                now.GetYear() + 1, an.hour, an.minute);
              an.expiry_time =
                  an.start_time + wxTimeSpan::Minutes(an.duration_minutes);
            }

            int subarea_count = (bstr->GetBitCount() - 111) / 87;
            for (int i = 0; i < subarea_count; ++i) {
              int base = 111 + i * 87;
              Ais8_001_22_SubArea sa;
              sa.shape = bstr->GetInt(base + 1, 3);
              int scale_factor = 1;
              if (sa.shape == AIS8_001_22_SHAPE_TEXT) {
                char t[15];
                t[14] = 0;
                bstr->GetStr(base + 4, 84, t, 14);
                sa.text = wxString(t, wxConvUTF8);
              } else {
                int scale_multipliers[4] = {1, 10, 100, 1000};
                scale_factor = scale_multipliers[bstr->GetInt(base + 4, 2)];
                switch (sa.shape) {
                  case AIS8_001_22_SHAPE_CIRCLE:
                  case AIS8_001_22_SHAPE_SECTOR:
                    sa.radius_m = bstr->GetInt(base + 58, 12) * scale_factor;
                    // FALL THROUGH
                  case AIS8_001_22_SHAPE_RECT:
                    sa.longitude = bstr->GetInt(base + 6, 25, true) / 60000.0;
                    sa.latitude = bstr->GetInt(base + 31, 24, true) / 60000.0;
                    break;
                  case AIS8_001_22_SHAPE_POLYLINE:
                  case AIS8_001_22_SHAPE_POLYGON:
                    for (int i = 0; i < 4; ++i) {
                      sa.angles[i] = bstr->GetInt(base + 6 + i * 20, 10) * 0.5;
                      sa.dists_m[i] =
                          bstr->GetInt(base + 16 + i * 20, 10) * scale_factor;
                    }
                }
                if (sa.shape == AIS8_001_22_SHAPE_RECT) {
                  sa.e_dim_m = bstr->GetInt(base + 58, 8) * scale_factor;
                  sa.n_dim_m = bstr->GetInt(base + 66, 8) * scale_factor;
                  sa.orient_deg = bstr->GetInt(base + 74, 9);
                }
                if (sa.shape == AIS8_001_22_SHAPE_SECTOR) {
                  sa.left_bound_deg = bstr->GetInt(70, 9);
                  sa.right_bound_deg = bstr->GetInt(79, 9);
                }
              }
              an.sub_areas.push_back(sa);
            }
            ptd->area_notices[an.link_id] = an;
            parse_result = true;
          }
        }
      }
      break;
    }
    case 14:  // Safety Related Broadcast
    {
      //  Always capture the MSG_14 text
      char msg_14_text[968];
      if (bstr->GetBitCount() > 40) {
        int nx = ((bstr->GetBitCount() - 40) / 6) * 6;
        int nd = bstr->GetStr(41, nx, msg_14_text, 968);
        nd = wxMax(0, nd);
        nd = wxMin(nd, 967);
        msg_14_text[nd] = 0;
        ptd->MSG_14_text = wxString(msg_14_text, wxConvUTF8);
      }
      parse_result = true;  // so far so good

      break;
    }

    case 6:  // Addressed Binary Message
    {
      break;
    }
    case 7:  // Binary Ack
    {
      break;
    }
    default: {
      break;
    }
  }

  if (b_posn_report) ptd->b_lost = false;

  if (true == parse_result) {
    //      Revalidate the target under some conditions
    if (!ptd->b_active && !ptd->b_positionDoubtful && b_posn_report)
      ptd->b_active = true;
  }

  return parse_result;
}

bool AisMsgDecoder::NMEACheckSumOK(const wxString &str_in) {
  unsigned char checksum_value = 0;
  int sentence_hex_sum;

  wxCharBuffer buf = str_in.ToUTF8();
  if (!buf.data()) return false;  // cannot decode string

  char str_ascii[AIS_MAX_MESSAGE_LEN + 1];
  strncpy(str_ascii, buf.data(), AIS_MAX_MESSAGE_LEN);
  str_ascii[AIS_MAX_MESSAGE_LEN] = '\0';

  int string_length = strlen(str_ascii);

  int payload_length = 0;
  while ((payload_length < string_length) &&
         (str_ascii[payload_length] != '*'))  // look for '*'
    payload_length++;

  if (payload_length == string_length)
    return false;  // '*' not found at all, no checksum

  int index = 1;  // Skip over the $ at the begining of the sentence

  while (index < payload_length) {
    checksum_value ^= str_ascii[index];
    index++;
  }

  if (string_length > 4) {
    char scanstr[3];
    scanstr[0] = str_ascii[payload_length + 1];
    scanstr[1] = str_ascii[payload_length + 2];
    scanstr[2] = 0;
    sscanf(scanstr, "%2x", &sentence_hex_sum);

    if (sentence_hex_sum == checksum_value) return true;
  }

  return false;
}
