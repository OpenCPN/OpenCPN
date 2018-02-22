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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *
 *   S Blackburn's original source license:                                *
 *         "You can use it any way you like."                              *
 *   More recent (2010) license statement:                                 *
 *         "It is BSD license, do with it what you will"                   *
 */


#include "nmea0183.h"
#pragma hdrstop

/*
** Author: Samuel R. Blackburn
** CI$: 76300,326
** Internet: sammy@sed.csc.com
**
** You can use it any way you like.
*/


MANUFACTURER_LIST::MANUFACTURER_LIST()
{
   ManufacturerTable.RemoveAll();
}

MANUFACTURER_LIST::~MANUFACTURER_LIST()
{
   int index = 0;
   int number_of_entries = ManufacturerTable.GetSize();

   while( index < number_of_entries )
   {
      delete (MANUFACTURER *) ManufacturerTable[ index ];
      index++;
   }

   ManufacturerTable.RemoveAll();
}

void MANUFACTURER_LIST::Add( MANUFACTURER *manufacturer_p )
{
   ASSERT_VALID( this );

   ManufacturerTable.Add( manufacturer_p );
}

void MANUFACTURER_LIST::Fill( void )
{
   ASSERT_VALID( this );

   MANUFACTURER *manufacturer_p = (MANUFACTURER *) NULL;

   Add( new MANUFACTURER( "AAR", "Asian American Resources" ) );
   Add( new MANUFACTURER( "ACE", "Auto-Comm Engineering Corporation" ) );
   Add( new MANUFACTURER( "ACR", "ACR Electronics, Inc." ) );
   Add( new MANUFACTURER( "ACS", "Arco Solar, Inc." ) );
   Add( new MANUFACTURER( "ACT", "Advanced Control Technology" ) );
   Add( new MANUFACTURER( "AGI", "Airguide Instrument Company" ) );
   Add( new MANUFACTURER( "AHA", "Autohelm of America" ) );
   Add( new MANUFACTURER( "AIP", "Aiphone Corporation" ) );
   Add( new MANUFACTURER( "ALD", "Alden Electronics, Inc." ) );
   Add( new MANUFACTURER( "AMR", "AMR Systems" ) );
   Add( new MANUFACTURER( "AMT", "Airmar Technology" ) );
   Add( new MANUFACTURER( "ANS", "Antenna Specialists" ) );
   Add( new MANUFACTURER( "ANX", "Analytyx Electronic Systems" ) );
   Add( new MANUFACTURER( "ANZ", "Anschutz of America" ) );
   Add( new MANUFACTURER( "APC", "Apelco" ) );
   Add( new MANUFACTURER( "APN", "American Pioneer, Inc." ) );
   Add( new MANUFACTURER( "APX", "Amperex, Inc." ) );
   Add( new MANUFACTURER( "AQC", "Aqua-Chem, Inc." ) );
   Add( new MANUFACTURER( "AQD", "Aquadynamics, Inc." ) );
   Add( new MANUFACTURER( "AQM", "Aqua Meter Instrument Company" ) );
   Add( new MANUFACTURER( "ASP", "American Solar Power" ) );
   Add( new MANUFACTURER( "ATE", "Aetna Engineering" ) );
   Add( new MANUFACTURER( "ATM", "Atlantic Marketing Company, Inc." ) );
   Add( new MANUFACTURER( "ATR", "Airtron" ) );
   Add( new MANUFACTURER( "ATV", "Activation, Inc." ) );
   Add( new MANUFACTURER( "AVN", "Advanced Navigation, Inc." ) );
   Add( new MANUFACTURER( "AWA", "Awa New Zealand, Limited" ) );
   Add( new MANUFACTURER( "BBL", "BBL Industries, Inc." ) );
   Add( new MANUFACTURER( "BBR", "BBR and Associates" ) );
   Add( new MANUFACTURER( "BDV", "Brisson Development, Inc." ) );
   Add( new MANUFACTURER( "BEC", "Boat Electric Company" ) );
   Add( new MANUFACTURER( "BGS", "Barringer Geoservice" ) );
   Add( new MANUFACTURER( "BGT", "Brookes and Gatehouse, Inc." ) );
   Add( new MANUFACTURER( "BHE", "BH Electronics" ) );
   Add( new MANUFACTURER( "BHR", "Bahr Technologies, Inc." ) );
   Add( new MANUFACTURER( "BLB", "Bay Laboratories" ) );
   Add( new MANUFACTURER( "BMC", "BMC" ) );
   Add( new MANUFACTURER( "BME", "Bartel Marine Electronics" ) );
   Add( new MANUFACTURER( "BNI", "Neil Brown Instrument Systems" ) );
   Add( new MANUFACTURER( "BNS", "Bowditch Navigation Systems" ) );
   Add( new MANUFACTURER( "BRM", "Mel Barr Company" ) );
   Add( new MANUFACTURER( "BRY", "Byrd Industries" ) );
   Add( new MANUFACTURER( "BTH", "Benthos, Inc." ) );
   Add( new MANUFACTURER( "BTK", "Baltek Corporation" ) );
   Add( new MANUFACTURER( "BTS", "Boat Sentry, Inc." ) );
   Add( new MANUFACTURER( "BXA", "Bendix-Avalex, Inc." ) );
   Add( new MANUFACTURER( "CAT", "Catel" ) );
   Add( new MANUFACTURER( "CBN", "Cybernet Marine Products" ) );
   Add( new MANUFACTURER( "CCA", "Copal Corporation of America" ) );
   Add( new MANUFACTURER( "CCC", "Coastal Communications Company" ) );
   Add( new MANUFACTURER( "CCL", "Coastal Climate Company" ) );
   Add( new MANUFACTURER( "CCM", "Coastal Communications" ) );
   Add( new MANUFACTURER( "CDC", "Cordic Company" ) );
   Add( new MANUFACTURER( "CEC", "Ceco Communications, Inc." ) );
   Add( new MANUFACTURER( "CHI", "Charles Industries, Limited" ) );
   Add( new MANUFACTURER( "CKM", "Cinkel Marine Electronics Industries" ) );
   Add( new MANUFACTURER( "CMA", "Societe Nouvelle D'Equiment du Calvados" ) );
   Add( new MANUFACTURER( "CMC", "Coe Manufacturing Company" ) );
   Add( new MANUFACTURER( "CME", "Cushman Electronics, Inc." ) );
   Add( new MANUFACTURER( "CMP", "C-Map, s.r.l." ) );
   Add( new MANUFACTURER( "CMS", "Coastal Marine Sales Company" ) );
   Add( new MANUFACTURER( "CMV", "CourseMaster USA, Inc." ) );
   Add( new MANUFACTURER( "CNV", "Coastal Navigator" ) );
   Add( new MANUFACTURER( "CNX", "Cynex Manufactoring Company" ) );
   Add( new MANUFACTURER( "CPL", "Computrol, Inc." ) );
   Add( new MANUFACTURER( "CPN", "Compunav" ) );
   Add( new MANUFACTURER( "CPS", "Columbus Positioning, Inc." ) );
   Add( new MANUFACTURER( "CPT", "CPT, Inc." ) );
   Add( new MANUFACTURER( "CRE", "Crystal Electronics, Limited" ) );
   Add( new MANUFACTURER( "CRO", "The Caro Group" ) );
   Add( new MANUFACTURER( "CRY", "Crystek Crystals Corporation" ) );
   Add( new MANUFACTURER( "CSM", "Comsat Maritime Services" ) );
   Add( new MANUFACTURER( "CST", "Cast, Inc." ) );
   Add( new MANUFACTURER( "CSV", "Combined Services" ) );
   Add( new MANUFACTURER( "CTA", "Current Alternatives" ) );
   Add( new MANUFACTURER( "CTB", "Cetec Benmar" ) );
   Add( new MANUFACTURER( "CTC", "Cell-tech Communications" ) );
   Add( new MANUFACTURER( "CTE", "Castle Electronics" ) );
   Add( new MANUFACTURER( "CTL", "C-Tech, Limited" ) );
   Add( new MANUFACTURER( "CNI", "Continental Instruments" ) ); // Possible error in NMEA0183 docs
   Add( new MANUFACTURER( "CWD", "Cubic Western Data" ) );
   Add( new MANUFACTURER( "CWV", "Celwave R.F., Inc." ) );
   Add( new MANUFACTURER( "CYZ", "cYz, Incorporated" ) );
   Add( new MANUFACTURER( "DCC", "Dolphin Components Corporation" ) );
   Add( new MANUFACTURER( "DEB", "Debeg Gmbh" ) );
   Add( new MANUFACTURER( "DFI", "Defender Industries, Inc." ) );
   Add( new MANUFACTURER( "DGC", "Digicourse, Inc." ) );
   Add( new MANUFACTURER( "DME", "Digital Marine Electronics Corporation" ) );
   Add( new MANUFACTURER( "DMI", "Datamarine International, Inc." ) );
   Add( new MANUFACTURER( "DNS", "Dornier System Gmbh" ) );
   Add( new MANUFACTURER( "DNT", "Del Norte Technology, Inc." ) );
   Add( new MANUFACTURER( "DPS", "Danaplus, Inc." ) );
   Add( new MANUFACTURER( "DRL", "R.L. Drake Company" ) );
   Add( new MANUFACTURER( "DSC", "Dynascan Corporation" ) );
   Add( new MANUFACTURER( "DYN", "Dynamote Corporation" ) );
   Add( new MANUFACTURER( "DYT", "Dytek Laboratories, Inc." ) );
   Add( new MANUFACTURER( "EBC", "Emergency Beacon, Corporation" ) );
   Add( new MANUFACTURER( "ECT", "Echotec, Inc." ) );
   Add( new MANUFACTURER( "EEV", "EEV, Inc." ) );
   Add( new MANUFACTURER( "EFC", "Efcom Communication Systems" ) );
   Add( new MANUFACTURER( "ELD", "Electronic Devices, Inc." ) );
   Add( new MANUFACTURER( "EMC", "Electric Motion Company" ) );
   Add( new MANUFACTURER( "EMS", "Electro Marine Systems, Inc." ) );
   Add( new MANUFACTURER( "ENA", "Energy Analysts, Inc." ) );
   Add( new MANUFACTURER( "ENC", "Encron, Inc." ) );
   Add( new MANUFACTURER( "EPM", "Epsco Marine" ) );
   Add( new MANUFACTURER( "EPT", "Eastprint, Inc." ) );
   Add( new MANUFACTURER( "ERC", "The Ericsson Corporation" ) );
   Add( new MANUFACTURER( "ESA", "European Space Agency" ) );
   Add( new MANUFACTURER( "FDN", "Fluiddyne" ) );
   Add( new MANUFACTURER( "FHE", "Fish Hawk Electronics" ) );
   Add( new MANUFACTURER( "FJN", "Jon Fluke Company" ) );
   Add( new MANUFACTURER( "FMM", "First Mate Marine Autopilots" ) );
   Add( new MANUFACTURER( "FNT", "Franklin Net and Twine, Limited" ) );
   Add( new MANUFACTURER( "FRC", "The Fredericks Company" ) );
   Add( new MANUFACTURER( "FTG", "T.G. Faria Corporation" ) );
   Add( new MANUFACTURER( "FUJ", "Fujitsu Ten Corporation of America" ) );
   Add( new MANUFACTURER( "FEC", "Furuno Electric Company" ) ); // Possible NMEA document error
   Add( new MANUFACTURER( "FUR", "Furuno, USA Inc." ) );
   Add( new MANUFACTURER( "GAM", "GRE America, Inc." ) );
   Add( new MANUFACTURER( "GCA", "Gulf Cellular Associates" ) );
   Add( new MANUFACTURER( "GES", "Geostar Corporation" ) );
   Add( new MANUFACTURER( "GFC", "Graphic Controls, Corporation" ) );
   Add( new MANUFACTURER( "GIS", "Galax Integrated Systems" ) );
   Add( new MANUFACTURER( "GPI", "Global Positioning Instrument Corporation" ) );
   Add( new MANUFACTURER( "GRM", "Garmin Corporation" ) );
   Add( new MANUFACTURER( "GSC", "Gold Star Company, Limited" ) );
   Add( new MANUFACTURER( "GTO", "Gro Electronics" ) );
   Add( new MANUFACTURER( "GVE", "Guest Corporation" ) );
   Add( new MANUFACTURER( "GVT", "Great Valley Technology" ) );
   Add( new MANUFACTURER( "HAL", "HAL Communications Corporation" ) );
   Add( new MANUFACTURER( "HAR", "Harris Corporation" ) );
   Add( new MANUFACTURER( "HIG", "Hy-Gain" ) );
   Add( new MANUFACTURER( "HIT", "Hi-Tec" ) );
   Add( new MANUFACTURER( "HPK", "Hewlett-Packard" ) );
   Add( new MANUFACTURER( "HRC", "Harco Manufacturing Company" ) );
   Add( new MANUFACTURER( "HRT", "Hart Systems, Inc." ) );
   Add( new MANUFACTURER( "HTI", "Heart Interface, Inc." ) );
   Add( new MANUFACTURER( "HUL", "Hull Electronics Company" ) );
   Add( new MANUFACTURER( "HWM", "Honeywell Marine Systems" ) );
   Add( new MANUFACTURER( "ICO", "Icom of America, Inc." ) );
   Add( new MANUFACTURER( "IFD", "International Fishing Devices" ) );
   Add( new MANUFACTURER( "IFI", "Instruments for Industry" ) );
   Add( new MANUFACTURER( "IME", "Imperial Marine Equipment" ) );
   Add( new MANUFACTURER( "IMI", "I.M.I." ) );
   Add( new MANUFACTURER( "IMM", "ITT MacKay Marine" ) );
   Add( new MANUFACTURER( "IMP", "Impulse Manufacturing, Inc." ) );
   Add( new MANUFACTURER( "IMT", "International Marketing and Trading, Inc." ) );
   Add( new MANUFACTURER( "INM", "Inmar Electronic and Sales, Inc." ) );
   Add( new MANUFACTURER( "INT", "Intech, Inc." ) );
   Add( new MANUFACTURER( "IRT", "Intera Technologies, Limited" ) );
   Add( new MANUFACTURER( "IST", "Innerspace Technology, Inc." ) );
   Add( new MANUFACTURER( "ITM", "Intermarine Electronics, Inc." ) );
   Add( new MANUFACTURER( "ITR", "Itera, Limited" ) );
   Add( new MANUFACTURER( "JAN", "Jan Crystals" ) );
   Add( new MANUFACTURER( "JFR", "Ray Jefferson" ) );
   Add( new MANUFACTURER( "JMT", "Japan Marine Telecommunications" ) );
   Add( new MANUFACTURER( "JRC", "Japan Radio Company, Inc." ) );
   Add( new MANUFACTURER( "JRI", "J-R Industries, Inc." ) );
   Add( new MANUFACTURER( "JTC", "J-Tech Associates, Inc." ) );
   Add( new MANUFACTURER( "JTR", "Jotron Radiosearch, Limited" ) );
   Add( new MANUFACTURER( "KBE", "KB Electronics, Limited" ) );
   Add( new MANUFACTURER( "KBM", "Kennebec Marine Company" ) );
   Add( new MANUFACTURER( "KLA", "Klein Associates, Inc." ) );
   Add( new MANUFACTURER( "KMR", "King Marine Radio Corporation" ) );
   Add( new MANUFACTURER( "KNG", "King Radio Corporation" ) );
   Add( new MANUFACTURER( "KOD", "Koden Electronics Company, Limited" ) );
   Add( new MANUFACTURER( "KRP", "Krupp International, Inc." ) );
   Add( new MANUFACTURER( "KVH", "KVH Company" ) );
   Add( new MANUFACTURER( "KYI", "Kyocera International, Inc." ) );
   Add( new MANUFACTURER( "LAT", "Latitude Corporation" ) );
   Add( new MANUFACTURER( "LEC", "Lorain Electronics Corporation" ) );
   Add( new MANUFACTURER( "LMM", "Lamarche Manufacturing Company" ) );
   Add( new MANUFACTURER( "LRD", "Lorad" ) );
   Add( new MANUFACTURER( "LSE", "Littlemore Scientific Engineering" ) );
   Add( new MANUFACTURER( "LSP", "Laser Plot, Inc." ) );
   Add( new MANUFACTURER( "LTF", "Littlefuse, Inc." ) );
   Add( new MANUFACTURER( "LWR", "Lowrance Electronics Corportation" ) );
   Add( new MANUFACTURER( "MCL", "Micrologic, Inc." ) ); 
   Add( new MANUFACTURER( "MDL", "Medallion Instruments, Inc." ) ); 
   Add( new MANUFACTURER( "MEC", "Marine Engine Center, Inc." ) ); 
   Add( new MANUFACTURER( "MEG", "Maritec Engineering GmbH" ) ); 
   Add( new MANUFACTURER( "MFR", "Modern Products, Limited" ) ); 
   Add( new MANUFACTURER( "MFW", "Frank W. Murphy Manufacturing" ) ); 
   Add( new MANUFACTURER( "MGS", "MG Electronic Sales Corporation" ) ); 
   Add( new MANUFACTURER( "MIE", "Mieco, Inc." ) ); 
   Add( new MANUFACTURER( "MIM", "Marconi International Marine Company" ) ); 
   Add( new MANUFACTURER( "MLE", "Martha Lake Electronics" ) ); 
   Add( new MANUFACTURER( "MLN", "Matlin Company" ) ); 
   Add( new MANUFACTURER( "MLP", "Marlin Products" ) ); 
   Add( new MANUFACTURER( "MLT", "Miller Technologies" ) ); 
   Add( new MANUFACTURER( "MMB", "Marsh-McBirney, Inc." ) ); 
   Add( new MANUFACTURER( "MME", "Marks Marine Engineering" ) ); 
   Add( new MANUFACTURER( "MMP", "Metal Marine Pilot, Inc." ) ); 
   Add( new MANUFACTURER( "MMS", "Mars Marine Systems" ) ); 
   Add( new MANUFACTURER( "MNI", "Micro-Now Instrument Company" ) ); 
   Add( new MANUFACTURER( "MNT", "Marine Technology" ) ); 
   Add( new MANUFACTURER( "MNX", "Marinex" ) ); 
   Add( new MANUFACTURER( "MOT", "Motorola Communications and Electronics" ) ); 
   Add( new MANUFACTURER( "MPN", "Memphis Net and Twine Company, Inc." ) ); 
   Add( new MANUFACTURER( "MQS", "Marquis Industries, Inc." ) ); 
   Add( new MANUFACTURER( "MRC", "Marinecomp, Inc." ) ); 
   Add( new MANUFACTURER( "MRE", "Morad Electronics Corporation" ) ); 
   Add( new MANUFACTURER( "MRP", "Mooring Products of New England" ) ); 
   Add( new MANUFACTURER( "MRR", "II Morrow, Inc." ) ); 
   Add( new MANUFACTURER( "MRS", "Marine Radio Service" ) ); 
   Add( new MANUFACTURER( "MSB", "Mitsubishi Electric Company, Limited" ) ); 
   Add( new MANUFACTURER( "MSE", "Master Electronics" ) ); 
   Add( new MANUFACTURER( "MSM", "Master Mariner, Inc." ) ); 
   Add( new MANUFACTURER( "MST", "Mesotech Systems, Limited" ) ); 
   Add( new MANUFACTURER( "MTA", "Marine Technical Associates" ) ); 
   Add( new MANUFACTURER( "MTG", "Narine Technical Assistance Group" ) ); 
   Add( new MANUFACTURER( "MTK", "Martech, Inc." ) ); 
   Add( new MANUFACTURER( "MTR", "Mitre Corporation, Inc." ) ); 
   Add( new MANUFACTURER( "MTS", "Mets, Inc." ) ); 
   Add( new MANUFACTURER( "MUR", "Murata Erie North America" ) ); 
   Add( new MANUFACTURER( "MVX", "Magnavox Advanced Products and Systems Company" ) ); 
   Add( new MANUFACTURER( "MXX", "Maxxima Marine" ) ); 
   Add( new MANUFACTURER( "MES", "Marine Electronics Service, Inc." ) ); 
   Add( new MANUFACTURER( "NAT", "Nautech, Limited" ) ); 
   Add( new MANUFACTURER( "NEF", "New England Fishing Gear, Inc." ) ); 
   Add( new MANUFACTURER( "NMR", "Newmar" ) ); 
   Add( new MANUFACTURER( "NGS", "Navigation Sciences, Inc." ) ); 
   Add( new MANUFACTURER( "NOM", "Nav-Com, Inc." ) ); 
   Add( new MANUFACTURER( "NOV", "NovAtel Communications, Limited" ) ); 
   Add( new MANUFACTURER( "NSM", "Northstar Marine" ) ); 
   Add( new MANUFACTURER( "NTK", "Novatech Designs, Limited" ) ); 
   Add( new MANUFACTURER( "NVC", "Navico" ) ); 
   Add( new MANUFACTURER( "NVS", "Navstar" ) ); 
   Add( new MANUFACTURER( "NVO", "Navionics, s.p.a." ) ); 
   Add( new MANUFACTURER( "OAR", "O.A.R. Corporation" ) ); 
   Add( new MANUFACTURER( "ODE", "Ocean Data Equipment Corporation" ) ); 
   Add( new MANUFACTURER( "ODN", "Odin Electronics, Inc." ) ); 
   Add( new MANUFACTURER( "OIN", "Ocean instruments, Inc." ) ); 
   Add( new MANUFACTURER( "OKI", "Oki Electronic Industry Company" ) ); 
   Add( new MANUFACTURER( "OLY", "Navstar Limited (Polytechnic Electronics)" ) ); 
   Add( new MANUFACTURER( "OMN", "Omnetics" ) ); 
   Add( new MANUFACTURER( "ORE", "Ocean Research" ) ); 
   Add( new MANUFACTURER( "OTK", "Ocean Technology" ) ); 
   Add( new MANUFACTURER( "PCE", "Pace" ) ); 
   Add( new MANUFACTURER( "PDM", "Prodelco Marine Systems" ) ); 
   Add( new MANUFACTURER( "PLA", "Plath, C. Division of Litton" ) ); 
   Add( new MANUFACTURER( "PLI", "Pilot Instruments" ) ); 
   Add( new MANUFACTURER( "PMI", "Pernicka Marine Products" ) ); 
   Add( new MANUFACTURER( "PMP", "Pacific Marine Products" ) ); 
   Add( new MANUFACTURER( "PRK", "Perko, Inc." ) ); 
   Add( new MANUFACTURER( "PSM", "Pearce-Simpson" ) ); 
   Add( new MANUFACTURER( "PTC", "Petro-Com" ) ); 
   Add( new MANUFACTURER( "PTG", "P.T.I./Guest" ) ); 
   Add( new MANUFACTURER( "PTH", "Pathcom, Inc." ) ); 
   Add( new MANUFACTURER( "RAC", "Racal Marine, Inc." ) ); 
   Add( new MANUFACTURER( "RAE", "RCA Astro-Electronics" ) ); 
   Add( new MANUFACTURER( "RAY", "Raytheon Marine Company" ) ); 
   Add( new MANUFACTURER( "RCA", "RCA Service Company" ) ); 
   Add( new MANUFACTURER( "RCH", "Roach Engineering" ) ); 
   Add( new MANUFACTURER( "RCI", "Rochester Instruments, Inc." ) ); 
   Add( new MANUFACTURER( "RDI", "Radar Devices" ) ); 
   Add( new MANUFACTURER( "RDM", "Ray-Dar Manufacturing Company" ) ); 
   Add( new MANUFACTURER( "REC", "Ross Engineering Company" ) ); 
   Add( new MANUFACTURER( "RFP", "Rolfite Products, Inc." ) ); 
   Add( new MANUFACTURER( "RGC", "RCS Global Communications, Inc." ) ); 
   Add( new MANUFACTURER( "RGY", "Regency Electronics, Inc." ) ); 
   Add( new MANUFACTURER( "RMR", "RCA Missile and Surface Radar" ) ); 
   Add( new MANUFACTURER( "RSL", "Ross Laboratories, Inc." ) ); 
   Add( new MANUFACTURER( "RSM", "Robertson-Shipmate, USA" ) ); 
   Add( new MANUFACTURER( "RWI", "Rockwell International" ) ); 
   Add( new MANUFACTURER( "RME", "Racal Marine Electronics" ) ); 
   Add( new MANUFACTURER( "RTN", "Robertson Tritech Nyaskaien A/S" ) ); 
   Add( new MANUFACTURER( "SAI", "SAIT, Inc." ) ); 
   Add( new MANUFACTURER( "SBR", "Sea-Bird electronics, Inc." ) ); 
   Add( new MANUFACTURER( "SCR", "Signalcrafters, Inc." ) ); 
   Add( new MANUFACTURER( "SEA", "SEA" ) ); 
   Add( new MANUFACTURER( "SEC", "Sercel Electronics of Canada" ) ); 
   Add( new MANUFACTURER( "SEP", "Steel and Engine Products, Inc." ) ); 
   Add( new MANUFACTURER( "SFN", "Seafarer Navigation International, Limited" ) ); 
   Add( new MANUFACTURER( "SGC", "SGC, Inc." ) ); 
   Add( new MANUFACTURER( "SIG", "Signet, Inc." ) ); 
   Add( new MANUFACTURER( "SIM", "Simrad,Inc." ) ); 
   Add( new MANUFACTURER( "SKA", "Skantek Corporation" ) ); 
   Add( new MANUFACTURER( "SKP", "Skipper Electronics A/S" ) ); 
   Add( new MANUFACTURER( "SME", "Shakespeare Marine Electronics" ) ); 
   Add( new MANUFACTURER( "SMF", "Seattle Marine and Fishing Supply Company" ) ); 
   Add( new MANUFACTURER( "SML", "Simerl Instruments" ) ); 
   Add( new MANUFACTURER( "SMI", "Sperry Marine, Inc." ) ); 
   Add( new MANUFACTURER( "SNV", "Starnav Corporation" ) ); 
   Add( new MANUFACTURER( "SOM", "Sound Marine Electronics, Inc." ) ); 
   Add( new MANUFACTURER( "SOV", "Sell Overseas America" ) ); 
   Add( new MANUFACTURER( "SPL", "Spelmar" ) ); 
   Add( new MANUFACTURER( "SPT", "Sound Powered Telephone" ) ); 
   Add( new MANUFACTURER( "SRD", "SRD Labs" ) ); 
   Add( new MANUFACTURER( "SRS", "Scientific Radio Systems, Inc." ) ); 
   Add( new MANUFACTURER( "SRT", "Standard Radio and Telefon AB" ) ); 
   Add( new MANUFACTURER( "SSI", "Sea Scout Industries" ) ); 
   Add( new MANUFACTURER( "STC", "Standard Communications" ) ); 
   Add( new MANUFACTURER( "STI", "Sea-Temp Instrument Corporation" ) ); 
   Add( new MANUFACTURER( "STM", "Si-Tex Marine Electronics" ) ); 
   Add( new MANUFACTURER( "SVY", "Savoy Electronics" ) ); 
   Add( new MANUFACTURER( "SWI", "Swoffer Marine Instruments, Inc." ) ); 
   Add( new MANUFACTURER( "SRS", "Shipmate, Rauff & Sorensen, A/S" ) ); 
   Add( new MANUFACTURER( "TBB", "Thompson Brothers Boat Manufacturing Company" ) ); 
   Add( new MANUFACTURER( "TCN", "Trade Commission of Norway (THE)" ) ); 
   Add( new MANUFACTURER( "TDL", "Tideland Signal" ) ); 
   Add( new MANUFACTURER( "THR", "Thrane and Thrane A/A" ) ); 
   Add( new MANUFACTURER( "TLS", "Telesystems" ) ); 
   Add( new MANUFACTURER( "TMT", "Tamtech, Limited" ) ); 
   Add( new MANUFACTURER( "TNL", "Trimble Navigation" ) ); 
   Add( new MANUFACTURER( "TRC", "Tracor, Inc." ) ); 
   Add( new MANUFACTURER( "TSI", "Techsonic Industries, Inc." ) ); 
   Add( new MANUFACTURER( "TTK", "Talon Technology Corporation" ) ); 
   Add( new MANUFACTURER( "TTS", "Transtector Systems" ) ); 
   Add( new MANUFACTURER( "TWC", "Transworld Communications, Inc." ) ); 
   Add( new MANUFACTURER( "TXI", "Texas Instruments, Inc." ) ); 
   Add( new MANUFACTURER( "UME", "Umec" ) ); 
   Add( new MANUFACTURER( "UNI", "Uniden Corporation of America" ) ); 
   Add( new MANUFACTURER( "UNP", "Unipas, Inc." ) ); 
   Add( new MANUFACTURER( "UNF", "Uniforce Electronics Company" ) ); 
   Add( new MANUFACTURER( "VAN", "Vanner, Inc." ) ); 
   Add( new MANUFACTURER( "VAR", "Varian Eimac Associates" ) ); 
   Add( new MANUFACTURER( "VCM", "Videocom" ) ); 
   Add( new MANUFACTURER( "VEX", "Vexillar" ) ); 
   Add( new MANUFACTURER( "VIS", "Vessel Information Systems, Inc." ) ); 
   Add( new MANUFACTURER( "VMR", "Vast Marketing Corporation" ) ); 
   Add( new MANUFACTURER( "WAL", "Walport USA" ) ); 
   Add( new MANUFACTURER( "WBG", "Westberg Manufacturing, Inc." ) ); 
   Add( new MANUFACTURER( "WEC", "Westinghouse electric Corporation" ) ); 
   Add( new MANUFACTURER( "WHA", "W-H Autopilots" ) ); 
   Add( new MANUFACTURER( "WMM", "Wait Manufacturing and Marine Sales Company" ) ); 
   Add( new MANUFACTURER( "WMR", "Wesmar Electronics" ) ); 
   Add( new MANUFACTURER( "WNG", "Winegard Company" ) ); 
   Add( new MANUFACTURER( "WSE", "Wilson Electronics Corporation" ) ); 
   Add( new MANUFACTURER( "WTC", "Watercom" ) ); 
   Add( new MANUFACTURER( "WST", "West Electronics Limited" ) ); 
   Add( new MANUFACTURER( "YAS", "Yaesu Electronics" ) ); 

   Sort();
}

const wxString& MANUFACTURER_LIST::Find( const CString& mnemonic )
{
   ASSERT_VALID( this );

   static wxString return_value;

   MANUFACTURER *manufacturer_p = (MANUFACTURER *) NULL;

   return_value.Empty();

   int lower_limit = 0;
   int upper_limit = 0;
   int index       = 0;
   int drop_dead   = 0;
   int comparison  = 0;
   int exit_loop   = 0;

   /*
   ** Do a Binary Search to call the appropriate function
   */

   lower_limit = 0;
   upper_limit = ManufacturerTable.GetSize();
   index       = upper_limit / 2;
   drop_dead   = ( index < 10 ) ? 10 : index + 2;

   /*
   ** The drop_dead is here as an insurance policy that we will never get stuck in an endless loop.
   ** I have encountered situations where the inaccuracy of the division leaves the loop stuck when
   ** it can't find something.
   */

   while( exit_loop == 0 )
   {
      manufacturer_p = (MANUFACTURER *) ManufacturerTable[ index ];

      comparison = mnemonic.Compare( manufacturer_p->Mnemonic );

      if ( comparison == 0 )
      {
         return_value = manufacturer_p->CompanyName;
         exit_loop = 1;
      }
      else
      {
         if ( comparison < 0 )
         {
            upper_limit = index;
         }
         else
         {
            lower_limit = index;
         }

         if ( lower_limit == upper_limit )
         {
            exit_loop = 1;
         }
         else
         {
            index = ( lower_limit + upper_limit ) / 2;
         }
      }

      drop_dead--;

      if ( drop_dead < 0 )
      {
         exit_loop = 1;
      }
   }

   return( return_value );
}

void MANUFACTURER_LIST::Sort( void )
{
   ASSERT_VALID( this );

   int index = 0;
   int number_of_entries_in_table = ManufacturerTable.GetSize() - 1;

   MANUFACTURER *this_manufacturer = (MANUFACTURER *) NULL;
   MANUFACTURER *that_manufacturer = (MANUFACTURER *) NULL;

   BOOL sorted = FALSE;

   while( sorted == FALSE )
   {
      sorted = TRUE;

      index = 0;

      while( index < number_of_entries_in_table )
      {
         this_manufacturer = (MANUFACTURER *) ManufacturerTable[ index     ];
         that_manufacturer = (MANUFACTURER *) ManufacturerTable[ index + 1 ];

         if ( this_manufacturer->Mnemonic.Compare( that_manufacturer->Mnemonic ) > 0 )
         {
            ManufacturerTable[ index     ] = that_manufacturer;
            ManufacturerTable[ index + 1 ] = this_manufacturer;

            sorted = FALSE;
         }

         index++;
      }
   }
}

const wxString& MANUFACTURER_LIST::operator [] ( const char *mnemonic )
{
   ASSERT_VALID( this );

   return( Find( mnemonic ) );
}
