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


#include "nmea0183.h"

/*
** Author: Samuel R. Blackburn
** CI$: 76300,326
** Internet: sammy@sed.csc.com
**
** You can use it any way you like.
*/

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(MRL);


NMEA0183::NMEA0183()
{
   initialize();

/*
   response_table.Append( (RESPONSE *) &Aam );
   response_table.Append( (RESPONSE *) &Alm );
   response_table.Append( (RESPONSE *) &Apb );
   response_table.Append( (RESPONSE *) &Asd );
   response_table.Append( (RESPONSE *) &Bec );
   response_table.Append( (RESPONSE *) &Bod );
   response_table.Append( (RESPONSE *) &Bwc );
   response_table.Append( (RESPONSE *) &Bwr );
   response_table.Append( (RESPONSE *) &Bww );
*/
   response_table.Append( (RESPONSE *) &Dbt );
/*
   response_table.Append( (RESPONSE *) &Dcn );
*/
   response_table.Append( (RESPONSE *) &Dpt );
/*
   response_table.Append( (RESPONSE *) &Fsi );
*/
   response_table.Append( (RESPONSE *) &Gga );
/*
   response_table.Append( (RESPONSE *) &Glc );
*/
   response_table.Append( (RESPONSE *) &Gll );
   response_table.Append( (RESPONSE *) &Gsv );
/*
   response_table.Append( (RESPONSE *) &Gxa );
*/
   response_table.Append( (RESPONSE *) &Hdm );
   response_table.Append( (RESPONSE *) &Hdg );
   response_table.Append( (RESPONSE *) &Hdt );
/*
   response_table.Append( (RESPONSE *) &Hsc );
   response_table.Append( (RESPONSE *) &Lcd );
*/
   response_table.Append( (RESPONSE *) &Mwd );
   response_table.Append( (RESPONSE *) &Mda ); //Barometric pressure
   response_table.Append( (RESPONSE *) &Mta ); //Air Temperature
   response_table.Append( (RESPONSE *) &Mtw ); //Water Temperature
   response_table.Append( (RESPONSE *) &Mwv );
/*
   response_table.Append( (RESPONSE *) &Oln );
   response_table.Append( (RESPONSE *) &Osd );
   response_table.Append( (RESPONSE *) &Proprietary );
   response_table.Append( (RESPONSE *) &Rma );
*/
   response_table.Append( (RESPONSE *) &Rmb );
   response_table.Append( (RESPONSE *) &Rmc );
/*
   response_table.Append( (RESPONSE *) &Rot );
   response_table.Append( (RESPONSE *) &Rpm );
*/
   response_table.Append( (RESPONSE *) &Rsa );
/*
   response_table.Append( (RESPONSE *) &Rsd );
*/
   response_table.Append( (RESPONSE *) &Rte );
/*
   response_table.Append( (RESPONSE *) &Sfi );
   response_table.Append( (RESPONSE *) &Stn );
   response_table.Append( (RESPONSE *) &Trf );
   response_table.Append( (RESPONSE *) &Ttm );
   response_table.Append( (RESPONSE *) &Vbw );
*/
   response_table.Append( (RESPONSE *) &Vhw );
   response_table.Append( (RESPONSE *) &Vlw );
   /*
   response_table.Append( (RESPONSE *) &Vdr );

   response_table.Append( (RESPONSE *) &Vpw );
*/
   response_table.Append( (RESPONSE *) &Vtg );
   response_table.Append( (RESPONSE *) &Vwr );
   response_table.Append( (RESPONSE *) &Vwt );
/*
   response_table.Append( (RESPONSE *) &Wcv );
   response_table.Append( (RESPONSE *) &Wnc );
*/
   response_table.Append( (RESPONSE *) &Wpl );

   response_table.Append( (RESPONSE *) &Xdr );
/*   response_table.Append( (RESPONSE *) &Xte );
   response_table.Append( (RESPONSE *) &Xtr );
*/
   response_table.Append( (RESPONSE *) &Zda );
/*
   response_table.Append( (RESPONSE *) &Zfo );
   response_table.Append( (RESPONSE *) &Ztg );
*/
   sort_response_table();
   set_container_pointers();
}

NMEA0183::~NMEA0183()
{
   initialize();
}

void NMEA0183::initialize( void )
{
//   ASSERT_VALID( this );

   ErrorMessage.Empty();
}

void NMEA0183::set_container_pointers( void )
{
//   ASSERT_VALID( this );

   int index = 0;
   int number_of_entries_in_table = response_table.GetCount();

   RESPONSE *this_response = (RESPONSE *) NULL;

   index = 0;

   while( index < number_of_entries_in_table )
   {
      this_response = (RESPONSE *) response_table[ index ];

      this_response->SetContainer( this );

      index++;
   }
}

void NMEA0183::sort_response_table( void )
{
//   ASSERT_VALID( this );

/*
   int index = 0;
   int number_of_entries_in_table = response_table.GetSize();

   RESPONSE *this_response = (RESPONSE *) NULL;
   RESPONSE *that_response = (RESPONSE *) NULL;

   bool sorted = FALSE;

   while( sorted == FALSE )
   {
      sorted = TRUE;

      index = 0;

      while( index < number_of_entries_in_table )
      {
         this_response = (RESPONSE *) response_table.Item( index     );
         that_response = (RESPONSE *) response_table.Item( index + 1 );

         if ( this_response->Mnemonic.Compare( that_response->Mnemonic ) > 0 )
         {
            response_table[ index     ] = that_response;
            response_table[ index + 1 ] = this_response;

            sorted = FALSE;
         }

         index++;
      }
   }
*/
}

/*
** Public Interface
*/

bool NMEA0183::IsGood( void ) const
{
//   ASSERT_VALID( this );

   /*
   ** NMEA 0183 sentences begin with $ and and with CR LF
   */

   if ( sentence.Sentence[ 0 ] != '$' )
   {
      return( FALSE );
   }

   /*
   ** Next to last character must be a CR
   */
   /*  This seems too harsh for cross platform work
    *
   if ( sentence.Sentence.Mid( sentence.Sentence.Len() - 2, 1 ) != wxString(_T("\r")) )
   {
      return( FALSE );
   }

   if ( sentence.Sentence.Right( 1 ) != _T("\n") )
   {
      return( FALSE );
   }
   */

   return( TRUE );
}


bool NMEA0183::PreParse( void )
{
    wxCharBuffer buf = sentence.Sentence.ToUTF8();
    if( !buf.data() )                            // badly formed sentence?
        return false;

      if ( IsGood() )
      {
            wxString mnemonic = sentence.Field( 0 );

      /*
            ** See if this is a proprietary field
      */

            if ( mnemonic.Left( 1 ) == 'P' )
                  mnemonic = _T("P");

            else
                  mnemonic = mnemonic.Right( 3 );


            LastSentenceIDReceived = mnemonic;

            return true;
      }
      else
            return false;
}


bool NMEA0183::Parse( void )
{
   bool return_value = FALSE;

   if(PreParse())
   {

      wxString mnemonic = sentence.Field( 0 );

      /*
      ** See if this is a proprietary field
      */

      if ( mnemonic.Left( 1 ) == 'P' )
      {
          mnemonic = _T("P");
      }
      else
      {
         mnemonic = mnemonic.Right( 3 );
      }

      /*
      ** Set up our default error message
      */

      ErrorMessage = mnemonic;
      ErrorMessage += _T(" is an unknown type of sentence");

      LastSentenceIDReceived = mnemonic;

      RESPONSE *response_p = (RESPONSE *) NULL;


//          Traverse the response list to find a mnemonic match

       wxMRLNode *node = response_table.GetFirst();

       int comparison  = 0;

        while(node)
        {
           RESPONSE *resp = node->GetData();

            comparison = mnemonic.Cmp( resp->Mnemonic );

            if ( comparison == 0 )
            {
                        response_p = (RESPONSE *) resp;
                        return_value = response_p->Parse( sentence );

                        /*
                        ** Set your ErrorMessage
                        */

                        if ( return_value == TRUE )
                        {
                           ErrorMessage = _T("No Error");
                           LastSentenceIDParsed = response_p->Mnemonic;
                           TalkerID = talker_id( sentence );
                           ExpandedTalkerID = expand_talker_id( TalkerID );
                        }
                        else
                        {
                           ErrorMessage = response_p->ErrorMessage;
                        }

                        break;
                   }

              node = node->GetNext();
        }

   }
   else
   {
      return_value = FALSE;
   }

   return( return_value );
}

NMEA0183& NMEA0183::operator << ( wxString & source )
{
//   ASSERT_VALID( this );

   sentence = source;

   return( *this );
}

NMEA0183& NMEA0183::operator >> ( wxString& destination )
{
//   ASSERT_VALID( this );

   destination = sentence;

   return( *this );
}
