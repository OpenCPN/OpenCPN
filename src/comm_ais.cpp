/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David S. Register                               *
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

#include "comm_ais.h"
#include <wx/tokenzr.h>

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double *)&lNaN)
#endif

//----------------------------------------------------------------------------------
//      Decode a single AIVDO sentence to a Generic Position Report
//----------------------------------------------------------------------------------
AisError DecodeSingleVDO(const wxString &str,
                          GenericPosDatEx *pos) {
  //  Make some simple tests for validity
  if (str.Len() > 100) return AIS_NMEAVDX_TOO_LONG;

  if (!NMEA_AISCheckSumOK(str)) return AIS_NMEAVDX_CHECKSUM_BAD;

  if (!pos) return AIS_GENERIC_ERROR;

  //if (!ctx.accumulator) return AIS_GENERIC_ERROR;

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

  //  Simple case only
  //  First and only part of a one-part sentence
  if ((1 == nsentences) && (1 == isentence)) {
    string_to_parse = tkz.GetNextToken();  // the encapsulated data
  }
  else {
    wxASSERT_MSG(false, wxT("Multipart AIVDO detected"));
    return AIS_INCOMPLETE_MULTIPART;  // and non-zero return
  }

  //  Create the bit accessible string
  AisBitstring strbit(string_to_parse.mb_str());

  AisTargetData TargetData;

  bool bdecode_result = Parse_VDXBitstring(&strbit, &TargetData);

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
bool Parse_VDXBitstring(AisBitstring* bstr,
                        AisTargetData* ptd) {
  bool parse_result = false;
  bool b_posn_report = false;

  wxDateTime now = wxDateTime::Now();
  now.MakeGMT();
  int message_ID = bstr->GetInt(1, 6);  // Parse on message ID
  ptd->MID = message_ID;

  // MMSI is always in the same spot in the bitstream
  ptd->MMSI = bstr->GetInt(9, 30);

  switch (message_ID) {
    case 1:  // Position Report
    case 2:
    case 3: {

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

      if ((lon_tentative <= 180.) && (lat_tentative <= 90.))
          // Ship does not report Lat or Lon "unavailable"
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

      // Convert to indicated ROT
      ptd->ROTIND = round(rot_dir * pow((((double)ptd->ROTAIS) / 4.733), 2));

      ptd->m_utc_sec = bstr->GetInt(138, 6);

      if ((1 == message_ID) || (2 == message_ID))
      // decode SOTDMA per 7.6.7.2.2
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
      // Class B targets have no status.  Enforce this...
      ptd->NavStatus = UNDEFINED;

      ptd->SOG = 0.1 * (bstr->GetInt(47, 10));

      int lon = bstr->GetInt(58, 28);
      if (lon & 0x08000000)  // negative?
        lon |= 0xf0000000;
      double lon_tentative = lon / 600000.;

      int lat = bstr->GetInt(86, 27);
      if (lat & 0x04000000)  // negative?
        lat |= 0xf8000000;
      double lat_tentative = lat / 600000.;

      if ((lon_tentative <= 180.) && (lat_tentative <= 90.))
          // Ship does not report Lat or Lon "unavailable"
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

    default: {
      break;
    }
  }

  if (b_posn_report) ptd->b_lost = false;

  if (true == parse_result) {
    // Revalidate the target under some conditions
    if (!ptd->b_active && !ptd->b_positionDoubtful && b_posn_report)
      ptd->b_active = true;
  }

  return parse_result;
}

bool NMEA_AISCheckSumOK(const wxString &str_in) {
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
