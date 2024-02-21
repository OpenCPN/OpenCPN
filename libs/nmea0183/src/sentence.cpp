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
#include <math.h>

#if !defined(NAN)

//static const long long lNaN = 0x7fffffffffffffff;

//#define NaN (*(double*)&lNaN)
//#else
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)

#endif

wxUniChar SENTENCE::extCharTable[] = {
   wxT('é'), wxT('è'), wxT('ë'), wxT('ê'), wxT('ẽ'),
   wxT('É'), wxT('È'), wxT('Ë'), wxT('Ê'), wxT('Ẽ'),
   wxT('á'), wxT('à'), wxT('ä'), wxT('â'), wxT('ã'),
   wxT('Á'), wxT('À'), wxT('Ä'), wxT('Â'), wxT('Ã'),
   wxT('í'), wxT('ì'), wxT('ï'), wxT('î'), wxT('ĩ'),
   wxT('Í'), wxT('Ì'), wxT('Ï'), wxT('Î'), wxT('Ĩ'),
   wxT('ú'), wxT('ù'), wxT('ü'), wxT('û'), wxT('ũ'),
   wxT('Ú'), wxT('Ù'), wxT('Ü'), wxT('Û'), wxT('Ũ'),
   wxT('ó'), wxT('ò'), wxT('ö'), wxT('ô'), wxT('õ'),
   wxT('Ó'), wxT('Ò'), wxT('Ö'), wxT('Ô'), wxT('Õ'),
   wxT('ń'), wxT('ǹ'), wxT('ñ'), wxT('ç'),
   wxT('Ń'), wxT('Ǹ'), wxT('Ñ'), wxT('Ç')
   };

char SENTENCE::substCharTable[] = {
   'e', 'e', 'e', 'e', 'e',
   'E', 'E', 'E', 'E', 'E',
   'a', 'a', 'a', 'a', 'a',
   'A', 'A', 'A', 'A', 'A',
   'i', 'i', 'i', 'i', 'i',
   'I', 'I', 'I', 'I', 'I',
   'u', 'u', 'u', 'u', 'u',
   'U', 'U', 'U', 'U', 'U',
   'o', 'o', 'o', 'o', 'o',
   'O', 'O', 'O', 'O', 'O',
   'n', 'n', 'n', 'c',
   'N', 'N', 'N', 'C'
   };

/*
** Author: Samuel R. Blackburn
** CI$: 76300,326
** Internet: sammy@sed.csc.com
**
** You can use it any way you like.
*/


SENTENCE::SENTENCE()
{
   Sentence.Empty();
}

SENTENCE::~SENTENCE()
{
   Sentence.Empty();
}

NMEA0183_BOOLEAN SENTENCE::Boolean( int field_number ) const
{
//   ASSERT_VALID( this );

   wxString field_data;

   field_data = Field( field_number );

   if ( field_data.StartsWith(_T("A")) )
   {
      return( NTrue );
   }
   else if ( field_data.StartsWith(_T("V")) )
   {
      return( NFalse );
   }
   else
   {
      return( Unknown0183 );
   }
}

COMMUNICATIONS_MODE SENTENCE::CommunicationsMode( int field_number ) const
{
//   ASSERT_VALID( this );

   wxString field_data;

   field_data = Field( field_number );

   if ( field_data == _T("d") )
   {
      return( F3E_G3E_SimplexTelephone );
   }
   else if ( field_data == _T("e") )
   {
      return( F3E_G3E_DuplexTelephone );
   }
   else if ( field_data == _T("m") )
   {
      return( J3E_Telephone );
   }
   else if ( field_data == _T("o") )
   {
      return( H3E_Telephone );
   }
   else if ( field_data == _T("q") )
   {
      return( F1B_J2B_FEC_NBDP_TelexTeleprinter );
   }
   else if ( field_data == _T("s") )
   {
      return( F1B_J2B_ARQ_NBDP_TelexTeleprinter );
   }
   else if ( field_data == _T("w") )
   {
      return( F1B_J2B_ReceiveOnlyTeleprinterDSC );
   }
   else if ( field_data == _T("x") )
   {
      return( A1A_MorseTapeRecorder );
   }
   else if ( field_data == _T("{") )
   {
      return( A1A_MorseKeyHeadset );
   }
   else if ( field_data == _T("|") )
   {
      return( F1C_F2C_F3C_FaxMachine );
   }
   else
   {
      return( CommunicationsModeUnknown );
   }
}

unsigned char SENTENCE::ComputeChecksum( void ) const
{
   unsigned char checksum_value = 0;

   int string_length = Sentence.Length();
   int index = 1; // Skip over the $ at the begining of the sentence

   while( index < string_length    &&
       Sentence[ index ] != '*' &&
       Sentence[ index ] != CARRIAGE_RETURN &&
       Sentence[ index ] != LINE_FEED )
   {
       checksum_value ^= (char)Sentence[ index ];
         index++;
   }

   return( checksum_value );
}

double SENTENCE::Double( int field_number ) const
{
 //  ASSERT_VALID( this );
      if(Field( field_number ).Len() == 0)
            return (NAN);

      wxCharBuffer abuf = Field( field_number).ToUTF8();
      if( !abuf.data() )                            // badly formed sentence?
        return (NAN);

      return( ::atof( abuf.data() ));

}


EASTWEST SENTENCE::EastOrWest( int field_number ) const
{
//   ASSERT_VALID( this );

   wxString field_data;

   field_data = Field( field_number );

   if ( field_data == _T("E") )
   {
      return( East );
   }
   else if ( field_data == _T("W") )
   {
      return( West );
   }
   else
   {
      return( EW_Unknown );
   }
}

const wxString& SENTENCE::Field( int desired_field_number ) const
{
//   ASSERT_VALID( this );

   static wxString return_string;
   return_string.Empty();

   int index                = 1; // Skip over the $ at the begining of the sentence
   int current_field_number = 0;
   int string_length        = 0;

   string_length = Sentence.Len();

   while( current_field_number < desired_field_number && index < string_length )
   {
      if ( Sentence[ index ] == ',' || Sentence[ index ] == '*' )
      {
         current_field_number++;
      }

      if( Sentence[ index ] == '*')
          return_string += Sentence[ index ];
      index++;
   }

   if ( current_field_number == desired_field_number )
   {
      while( index < string_length    &&
             Sentence[ index ] != ',' &&
             Sentence[ index ] != '*' &&
             Sentence[ index ] != 0x00 )
      {
         return_string += Sentence[ index ];
         index++;
      }
   }


   return( return_string );
}

int SENTENCE::GetNumberOfDataFields( void ) const
{
//   ASSERT_VALID( this );

   int index                = 1; // Skip over the $ at the begining of the sentence
   int current_field_number = 0;
   int string_length        = 0;

   string_length = Sentence.Len();

   while( index < string_length )
   {
      if ( Sentence[ index ] == '*' )
      {
         return( (int) current_field_number );
      }

      if ( Sentence[ index ] == ',' )
      {
         current_field_number++;
      }

      index++;
   }

   return( (int) current_field_number );
}

void SENTENCE::Finish( void )
{
//   ASSERT_VALID( this );

   unsigned char checksum = ComputeChecksum();

   wxString temp_string;

   temp_string.Printf(_T("*%02X%c%c"), (int) checksum, CARRIAGE_RETURN, LINE_FEED );
   Sentence += temp_string;
}

int SENTENCE::Integer( int field_number ) const
{
//   ASSERT_VALID( this );
    wxCharBuffer abuf = Field( field_number).ToUTF8();
    if( !abuf.data() )                            // badly formed sentence?
        return 0;

    return( ::atoi( abuf.data() ));
}

NMEA0183_BOOLEAN SENTENCE::IsChecksumBad( int checksum_field_number ) const
{
//   ASSERT_VALID( this );

   /*
   ** Checksums are optional, return TRUE if an existing checksum is known to be bad
   */

   wxString checksum_in_sentence = Field( checksum_field_number );

   if ( checksum_in_sentence == _T("") )
   {
      return( Unknown0183 );
   }

   wxString check = checksum_in_sentence.Mid( 1 );
   if ( ComputeChecksum() != HexValue( check ) )
   {
      return( NTrue );
   }

   return( NFalse );
}

LEFTRIGHT SENTENCE::LeftOrRight( int field_number ) const
{
//   ASSERT_VALID( this );

   wxString field_data;

   field_data = Field( field_number );

   if ( field_data == _T("L") )
   {
      return( Left );
   }
   else if ( field_data == _T("R") )
   {
      return( Right );
   }
   else
   {
      return( LR_Unknown );
   }
}

NORTHSOUTH SENTENCE::NorthOrSouth( int field_number ) const
{
//   ASSERT_VALID( this );

   wxString field_data;

   field_data = Field( field_number );

   if ( field_data == _T("N") )
   {
      return( North );
   }
   else if ( field_data == _T("S") )
   {
      return( South );
   }
   else
   {
      return( NS_Unknown );
   }
}

REFERENCE SENTENCE::Reference( int field_number ) const
{
//   ASSERT_VALID( this );

   wxString field_data;

   field_data = Field( field_number );

   if ( field_data == _T("B") )
   {
      return( BottomTrackingLog );
   }
   else if ( field_data == _T("M") )
   {
      return( ManuallyEntered );
   }
   else if ( field_data == _T("W") )
   {
      return( WaterReferenced );
   }
   else if ( field_data == _T("R") )
   {
      return( RadarTrackingOfFixedTarget );
   }
   else if ( field_data == _T("P") )
   {
      return( PositioningSystemGroundReference );
   }
   else
   {
      return( ReferenceUnknown );
   }
}

TRANSDUCER_TYPE SENTENCE::TransducerType( int field_number ) const
{
//   ASSERT_VALID( this );

   wxString field_data;

   field_data = Field( field_number );

   if ( field_data == _T("A") )
   {
      return( AngularDisplacementTransducer );
   }
   else if ( field_data == _T("D") )
   {
      return( LinearDisplacementTransducer );
   }
   else if ( field_data == _T("C") )
   {
      return( TemperatureTransducer );
   }
   else if ( field_data == _T("F") )
   {
      return( FrequencyTransducer );
   }
   else if ( field_data == _T("N") )
   {
      return( ForceTransducer );
   }
   else if ( field_data == _T("P") )
   {
      return( PressureTransducer );
   }
   else if ( field_data == _T("R") )
   {
      return( FlowRateTransducer );
   }
   else if ( field_data == _T("T") )
   {
      return( TachometerTransducer );
   }
   else if ( field_data == _T("H") )
   {
      return( HumidityTransducer );
   }
   else if ( field_data == _T("V") )
   {
      return( VolumeTransducer );
   }
   else
   {
      return( TransducerUnknown );
   }
}

/*
** Operators
*/

SENTENCE::operator wxString() const
{
//   ASSERT_VALID( this );

   return( Sentence );
}

const SENTENCE& SENTENCE::operator = ( const SENTENCE& source )
{
//   ASSERT_VALID( this );

   Sentence = source.Sentence;

   return( *this );
}

const SENTENCE& SENTENCE::operator = ( const wxString& source )
{
//   ASSERT_VALID( this );

   Sentence = source;

   return( *this );
}

const SENTENCE& SENTENCE::operator += ( const wxString& source )
{
//   ASSERT_VALID( this );

    Sentence += _T(",");
   Sentence += source;

   return( *this );
}

const SENTENCE& SENTENCE::operator += ( double value )
{
//   ASSERT_VALID( this );

   wxString temp_string;

   temp_string.Printf(_T("%.3f"), value );

   Sentence += _T(",");
   Sentence += temp_string;

   return( *this );
}

SENTENCE& SENTENCE::Add ( double value, int precision )
{
//   ASSERT_VALID( this );

    wxString temp_string;
    wxString s_Precision;

    s_Precision.Printf(_T("%c.%if"), '%', precision );
    temp_string.Printf( s_Precision, value );

    Sentence += _T(",");
    Sentence += temp_string;

    return( *this );
}
const SENTENCE& SENTENCE::operator += ( COMMUNICATIONS_MODE mode )
{
//   ASSERT_VALID( this );

    Sentence += _T(",");

   switch( mode )
   {
      case F3E_G3E_SimplexTelephone:

          Sentence += _T("d");
               break;

      case F3E_G3E_DuplexTelephone:

          Sentence += _T("e");
               break;

      case J3E_Telephone:

          Sentence += _T("m");
               break;

      case H3E_Telephone:

          Sentence += _T("o");
               break;

      case F1B_J2B_FEC_NBDP_TelexTeleprinter:

          Sentence += _T("q");
               break;

      case F1B_J2B_ARQ_NBDP_TelexTeleprinter:

          Sentence += _T("s");
               break;

      case F1B_J2B_ReceiveOnlyTeleprinterDSC:

          Sentence += _T("w");
               break;

      case A1A_MorseTapeRecorder:

          Sentence += _T("x");
               break;

      case A1A_MorseKeyHeadset:

          Sentence += _T("{");
               break;

       case F1C_F2C_F3C_FaxMachine:

           Sentence += _T("|");
           break;

       case CommunicationsModeUnknown:

           break;
   }

   return( *this );
}

const SENTENCE& SENTENCE::operator += ( TRANSDUCER_TYPE transducer )
{
//   ASSERT_VALID( this );

    Sentence += _T(",");

   switch( transducer )
   {
      case TemperatureTransducer:

          Sentence += _T("C");
               break;

      case AngularDisplacementTransducer:

          Sentence += _T("A");
               break;

      case LinearDisplacementTransducer:

          Sentence += _T("D");
               break;

      case FrequencyTransducer:

          Sentence += _T("F");
               break;

      case ForceTransducer:

          Sentence += _T("N");
               break;

      case PressureTransducer:

          Sentence += _T("P");
               break;

      case FlowRateTransducer:

          Sentence += _T("R");
               break;

      case TachometerTransducer:

          Sentence += _T("T");
               break;

      case HumidityTransducer:

          Sentence += _T("H");
               break;

      case VolumeTransducer:

          Sentence += _T("V");
               break;

      case TransducerUnknown:

          Sentence += _T("?");
               break;

   }

   return( *this );
}

const SENTENCE& SENTENCE::operator += ( NORTHSOUTH northing )
{
//   ASSERT_VALID( this );

    Sentence += _T(",");

   if ( northing == North )
   {
       Sentence += _T("N");
   }
   else if ( northing == South )
   {
       Sentence += _T("S");
   }

   return( *this );
}

const SENTENCE& SENTENCE::operator += ( int value )
{
//   ASSERT_VALID( this );

   wxString temp_string;

   temp_string.Printf(_T("%d"), value );

   Sentence += _T(",");
   Sentence += temp_string;

   return( *this );
}

const SENTENCE& SENTENCE::operator += ( EASTWEST easting )
{
//   ASSERT_VALID( this );

    Sentence += _T(",");

   if ( easting == East )
   {
       Sentence += _T("E");
   }
   else if ( easting == West )
   {
       Sentence += _T("W");
   }

   return( *this );
}

const SENTENCE& SENTENCE::operator += ( NMEA0183_BOOLEAN boolean )
{
//   ASSERT_VALID( this );

    Sentence += _T(",");

   if ( boolean == NTrue )
   {
       Sentence += _T("A");
   }
   else if ( boolean == NFalse )
   {
       Sentence += _T("V");
   }

   return( *this );
}

const SENTENCE& SENTENCE::operator += ( LATLONG& source )
{
//   ASSERT_VALID( this );

   source.Write( *this );

   return( *this );
}

/// @brief Convert a Unicode string into a 7-bit, NMEA compatible string.
/// All characters which are not compatible with NMEA string restrictions
/// are either converted to their ASCII counterpart, either removed
/// from the string.
/// @param str String to be converted
/// @return Converted string
wxString SENTENCE::ToNmeaString(const wxString& str) {
  wxString nmeaString;
  wxUniChar c;
  char ac;

  // Process characters one by one
  for (int i = 0; i < str.length(); i++) {
    c = str.GetChar(i);

    // Check if the character is a standard ASCII one
    if (c.IsAscii()) {
      // We have a standard, non-extended 7-bit character : we still have to check that it fits
      // NMEA restrictions. If not, the character is ignored.

      if ((c < 0x20) || (c > 0x7d))
      {
        // No characters below 0x20 or above 0x7d are valid : ignore it
        continue;
      }

      // Check for NMEA reserved characters
      ac = c.GetValue();
      switch (ac) {
        case '!':
        case '$':
        case '*':
        case ',':
        case '\\':
        case '^':
          // Character is reserved : ignore it
          break;
        default:
          // Character is valid : add it to the converted string
          nmeaString.append(ac);
          break;
      }
    } else {
      // Character is not ASCII : check if we have an ASCII substitution for it
      int nbSubtChars = sizeof(substCharTable);
      for (int i = 0; i < nbSubtChars; i++)
      {
         if (c == extCharTable[i])
         {
            // Yes : convert it
            nmeaString.append(substCharTable[i]);
            break;
         }
      }
    }
  }

  return nmeaString;
}
