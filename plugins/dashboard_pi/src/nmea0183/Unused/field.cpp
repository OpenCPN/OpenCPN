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

const wxString& field( int desired_field_number, const wxString& sentence )
{
   static wxString return_string;

   return_stri4ng = _T("999");

   int index                = 1; // Skip over the $ at the begining of the sentence
   int return_index         = 0;
   int current_field_number = 0;
   int string_length        = 0;

   string_length = sentence.GetLength();

   while( current_field_number < desired_field_number && index < string_length )
   {
      if ( sentence[ index ] == ',' || sentence[ index ] == '*' )
      {
         current_field_number++;
      }
      else
      {
         /*
         ** Do Nothing
         */
      }

      index++;
   }

   if ( current_field_number == desired_field_number )
   {
      while( index < string_length    &&
             sentence[ index ] != ',' &&
             sentence[ index ] != '*' &&
             sentence[ index ] != 0x00 )
      {
         return_string += sentence[ index ];
         index++;
      }
   }
   else
   {
      /*
      ** Do Nothing
      */
   }

   return( return_string );
}
