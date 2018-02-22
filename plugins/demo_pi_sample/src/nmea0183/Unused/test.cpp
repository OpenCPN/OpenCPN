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

/*
** This is the test scenario. Add more sentences to the list to test them
*/

int main()
{
   NMEA0183 nmea0183;

   CStringList test_sentences;
   CString     sentence;

   test_sentences.RemoveAll();

   /*
   ** Load our test sentences and keep track of where they came from
   */

   test_sentences.AddTail( "$LCAAM,V,A,.15,N,CHAT-N6*56\r\n" ); // NMEA0183 document, pg 39
   test_sentences.AddTail( "$GPAPB,A,A,0.0,R,N,V,V,86.8,T,001,86.9,T,86.9,T\r\n" ); // I can't remember
   test_sentences.AddTail( "$GPAPB,A,A,0.0,R,N,V,V,86.8,T,001,86.9,T,86.9,T\r\n" ); // I can't remember
   test_sentences.AddTail( "$GPBWC,104432.69,3337.36,N,11154.54,W,86.9,T,74.6,M,3.2,N,001\r\n" ); // I can't remember
   test_sentences.AddTail( "$GPBWC,104435.12,3337.36,N,11154.54,W,86.9,T,74.6,M,3.2,N,001\r\n" ); // I can't remember
   test_sentences.AddTail( "$GPBWC,225444,4917.24,N,12309.57,W,051.9,T,031.6,M,001.3,N,004*29\r\n" ); // Peter Bennett, bennett@triumf.ca
   test_sentences.AddTail( "$CTFSI,020230,026140,m,5*11\r\n" ); // NMEA0183 document, pg 41
   test_sentences.AddTail( "$CDFSI,900016,,d,9*08\r\n" ); // NMEA0183 document, pg 41
   test_sentences.AddTail( "$CTFSI,416193,,s,0*00\r\n" ); // NMEA0183 document, pg 41
   test_sentences.AddTail( "$CXFSI,,021875,t,*3A\r\n" ); // NMEA0183 document, pg 41
   test_sentences.AddTail( "$GPGGA,104435.12,3337.19,N,11158.43,W,1,06,4.5,,,,,,\r\n" ); // I can't remember
   test_sentences.AddTail( "$GPGGA,104432.69,3337.19,N,11158.43,W,1,06,5.1,,,,,,\r\n" ); // I can't remember
   test_sentences.AddTail( "$GPGLL,3337.190,N,11158.430,W,104432.686,A\r\n" ); // I can't remember
   test_sentences.AddTail( "$GPGLL,3337.190,N,11158.430,W,104435.122,A\r\n" ); // I can't remember
   test_sentences.AddTail( "$LCGLL,4728.31,N,12254.25,W,091342,A*21\r\n" ); // NMEA0183 document, pg 39
   test_sentences.AddTail( "$GPGLL,4916.45,N,12311.12,W*71\r\n" ); // Peter Bennett, bennett@triumf.ca
   test_sentences.AddTail( "$PGRMM,NAD27 Canada*2F\r\n" ); // Peter Bennett, bennett@triumf.ca
   test_sentences.AddTail( "$PGRMZ,93,f,3*21\r\n" ); // Peter Bennett, bennett@triumf.ca
   test_sentences.AddTail( "$PSLIB,,,J*22\r\n" ); // Peter Bennett, bennett@triumf.ca
   test_sentences.AddTail( "$PSRDA003[470738][1224523]??RST47,3809,A004*47\r\n" ); // NMEA0183 document, pg 40
   test_sentences.AddTail( "$GPR00,MINST,CHATN,CHAT1,CHATW,CHATM,CHATE,003,004,005,006,007,,,*05\r\n" ); // Peter Bennett, bennett@triumf.ca
   test_sentences.AddTail( "$LCRMA,V,,,,,14162.8,,,,,*0D\r\n" ); // NMEA0183 document, pg 40
   test_sentences.AddTail( "$LCRMA,V,,,,,14172.3,26026.7,,,,*2E\r\n" ); // NMEA0183 document, pg 40
   test_sentences.AddTail( "$LCRMA,A,,,,,14182.3,26026.7,,,,*36\r\n" ); // NMEA0183 document, pg 40
   test_sentences.AddTail( "$LCRMA,A,4226.26,N,07125.89,W,14182.3,26026.7,8.5,275.,14.0,W*68\r\n" ); // NMEA0183 document, pg 40
   test_sentences.AddTail( "$LCRMA,V,4226.26,N,07125.89,W,14182.3,26026.7,8.5,275.,14.0,W*7F\r\n" ); // NMEA0183 document, pg 40
   test_sentences.AddTail( "$LCRMA,A,4226.265,N,07125.890,W,14172.33,26026.71,8.53,275.,14.0,W*53\r\n" ); // NMEA0183 document, pg 40
   test_sentences.AddTail( "$GPRMB,A,0.66,L,003,004,4917.24,N,12309.57,W,001.3,052.5,000.5,V*0B\r\n" ); // Peter Bennett, bennett@triumf.ca
   test_sentences.AddTail( "$GPRMC,225446,A,4916.45,N,12311.12,W,000.5,054.7,191194,020.3,E*68\r\n" ); // Peter Bennett, bennett@triumf.ca
   test_sentences.AddTail( "$GPVTG,0.0,T,347.6,M,0.0,N,0.0,K\r\n" ); // I can't remember
   test_sentences.AddTail( "$GPVTG,0.0,T,347.6,M,0.0,N,0.0,K\r\n" ); // I can't remember
   test_sentences.AddTail( "$GPVTG,054.7,T,034.4,M,000.5,N,000.9,K*47\r\n" ); // Peter Bennett, bennett@triumf.ca
   test_sentences.AddTail( "$GPWPL,4917.16,N,12310.64,W,003*65\r\n" ); // Peter Bennett, bennett@triumf.ca
   test_sentences.AddTail( "$GPXTE,A,A,0.67,L,N*6F\r\n" ); // Peter Bennett, bennett@triumf.ca

   /*
   ** Now loop through the list and attempt to parse each sentence
   */

   POSITION position = test_sentences.GetHeadPosition();

   while( position != NULL )
   {
      /*
      ** Give the sentence to the NMEA0183 class
      */

      nmea0183 << test_sentences.GetNext( position );

      if ( nmea0183.Parse() != TRUE )
      {
         nmea0183 >> sentence;
         TRACE1( "\nCan't parse \"%s\"\n", (const char *) sentence );
         TRACE1( "Failed with \"%s\"\n", (const char *) nmea0183.ErrorMessage );
         TRACE1( "Last Sentence Received is \"%s\"\n", (const char *) nmea0183.LastSentenceIDReceived );
         TRACE1( "Last Sentence Parsed is   \"%s\"\n\n", (const char *) nmea0183.LastSentenceIDParsed );
      }
      else
      {
         TRACE2( "%s parsed OK from %s\n", (const char *) nmea0183.LastSentenceIDParsed, (const char *) nmea0183.ExpandedTalkerID );
      }
   }

   return( EXIT_SUCCESS );
}
