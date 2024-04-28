/* -----------------------------------------------------------------------------

	Copyright (c) 2015 Sean D'EPagnier

	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files (the
	"Software"), to	deal in the Software without restriction, including
	without limitation the rights to use, copy, modify, merge, publish,
	distribute, sublicense, and/or sell copies of the Software, and to
	permit persons to whom the Software is furnished to do so, subject to
	the following conditions:

	The above copyright notice and this permission notice shall be included
	in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
	OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
	CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
	TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   -------------------------------------------------------------------------- */

#include <string.h>

#include "singlecolourfitfast.h"
#include "colourset.h"
#include "colourblock.h"

namespace squish {

SingleColourFitFast::SingleColourFitFast( ColourSet * colours, int flags )
  : ColourFit( colours, flags )
{
	// grab the single colour
	u8 const* values = m_colours->GetPointsu8();
        memcpy(m_colour, values, 3);
}

void SingleColourFitFast::Compress3( void* block )
{
	// get the block as bytes
	u8* bytes = ( u8* )block;

        int a = m_colour[0] << 8 | m_colour[1] << 3 | m_colour[2] >> 3;

	// write the endpoints
	bytes[0] = ( u8 )( a & 0xff );
	bytes[1] = ( u8 )( a >> 8 );
        memset(bytes + 2, 0, 6);
}

void SingleColourFitFast::Compress4( void* block )
{
}

} // namespace squish
