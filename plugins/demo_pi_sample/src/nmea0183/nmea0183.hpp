/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA0183 Support Classes
 * Author:   Samuel R. Blackburn, David S. Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by Samuel R. Blackburn, David S Register           *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
 *   S Blackburn's original source license:                                *
 *         "You can use it any way you like."                              *
 *   More recent (2010) license statement:                                 *
 *         "It is BSD license, do with it what you will"                   *
 */


#if ! defined( NMEA_0183_CLASS_HEADER )
#define NMEA_0183_CLASS_HEADER

/*
** Author: Samuel R. Blackburn
** CI$: 76300,326
** Internet: sammy@sed.csc.com
**
** You can use it any way you like.
*/

/*
** General Purpose Classes
*/

#include "Sentence.hpp"
#include "Response.hpp"
#include "LatLong.hpp"
//#include "LoranTD.hpp"
//#include "Manufact.hpp"
//#include "MList.hpp"
//#include "OmegaPar.hpp"
//#include "DeccaLOP.hpp"
//#include "RatioPls.hpp"
//#include "RadarDat.hpp"
//#include "FreqMode.hpp"

/*
** Response Classes
*/

/*
#include "AAM.hpp"
#include "ALM.hpp"
#include "APB.hpp"
#include "ASD.hpp"
#include "BEC.hpp"
#include "BOD.hpp"
#include "BWC.hpp"
#include "BWR.hpp"
#include "BWW.hpp"
#include "DBT.hpp"
#include "DCN.hpp"
#include "DPT.hpp"
#include "FSI.hpp"
#include "GGA.hpp"
#include "GLC.hpp"
#include "GLL.hpp"
#include "GXA.hpp"
#include "HSC.hpp"
#include "LCD.hpp"
#include "MTW.hpp"
#include "MWV.hpp"
#include "OLN.hpp"
#include "OSD.hpp"
#include "P.hpp"
#include "RMA.hpp"
*/
#include "hdm.hpp"
#include "hdg.hpp"
#include "hdt.hpp"
#include "RMB.hpp"
#include "RMC.HPP"
#include "wpl.hpp"
#include "rte.hpp"
#include "gll.hpp"
#include "vtg.hpp"
#include "gsv.hpp"
#include "gga.hpp"
#include "GPwpl.hpp"
#include "apb.hpp"
#include "xte.hpp"
#include "mwd.hpp"
#include "mwv.hpp"

/*
#include "ROT.hpp"
#include "RPM.hpp"
#include "RSA.hpp"
#include "RSD.hpp"
#include "SFI.hpp"
#include "STN.hpp"
#include "TRF.hpp"
#include "TTM.hpp"
#include "VBW.hpp"
#include "VDR.hpp"
#include "VHW.hpp"
#include "VLW.hpp"
#include "VPW.hpp"
#include "VTG.hpp"
#include "WCV.hpp"
#include "WNC.hpp"
#include "XDR.hpp"
#include "XTR.hpp"
#include "ZDA.hpp"
#include "ZFO.hpp"
#include "ZTG.hpp"
*/

WX_DECLARE_LIST(RESPONSE, MRL);

class NMEA0183
{

   private:

      SENTENCE sentence;

      void initialize( void );

   protected:

      MRL response_table;

      void set_container_pointers( void );
      void sort_response_table( void );

   public:
      /** For use in main opencpn with access to globals. */
      NMEA0183(const NmeaContext& ctx);

      /** For use in plugins without access to globals. */
      NMEA0183();

      virtual ~NMEA0183();

      const NmeaContext caller_ctx;

      wxArrayString GetRecognizedArray(void);

      /*
      ** NMEA 0183 Sentences we understand
      */

/*
      AAM Aam;
      ALM Alm;
      APB Apb;
      ASD Asd;
      BEC Bec;
      BOD Bod;
      BWC Bwc;
      BWR Bwr;
      BWW Bww;
      DBT Dbt;
      DCN Dcn;
      DPT Dpt;
      FSI Fsi;
      GGA Gga;
      GLC Glc;
      GLL Gll;
      GXA Gxa;
      HSC Hsc;
      LCD Lcd;
      MTW Mtw;
      MWV Mwv;
      OLN Oln;
      OSD Osd;
      P   Proprietary;
      RMA Rma;
*/
       HDM Hdm;
       HDG Hdg;
       HDT Hdt;
       RMB Rmb;
       RMC Rmc;
       WPL Wpl;
       RTE Rte;
       GLL Gll;
       VTG Vtg;
       GSV Gsv;
       GGA Gga;
       GPWPL GPwpl;
       APB Apb;
       XTE Xte;
       MWD Mwd;
       MWV Mwv;
 /*
      ROT Rot;
      RPM Rpm;
      RSA Rsa;
      RSD Rsd;
      SFI Sfi;
      STN Stn;
      TRF Trf;
      TTM Ttm;
      VBW Vbw;
      VDR Vdr;
      VHW Vhw;
      VLW Vlw;
      VPW Vpw;
      VTG Vtg;
      WCV Wcv;
      WNC Wnc;
      XDR Xdr;
      XTE Xte;
      XTR Xtr;
      ZDA Zda;
      ZFO Zfo;
      ZTG Ztg;
*/
      wxString ErrorMessage; // Filled when Parse returns FALSE
      wxString LastSentenceIDParsed; // ID of the lst sentence successfully parsed
      wxString LastSentenceIDReceived; // ID of the last sentence received, may not have parsed successfully

      wxString TalkerID;
      wxString ExpandedTalkerID;

//      MANUFACTURER_LIST Manufacturers;

      virtual bool IsGood( void ) const;
      virtual bool Parse( void );
      virtual bool PreParse( void );

      NMEA0183& operator << ( wxString& source );
      NMEA0183& operator >> ( wxString& destination );
};

#endif // NMEA_0183_CLASS_HEADER
