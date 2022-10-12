/*
N2kDef.h

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

Type definitions and utility macros used in the NMEA2000 libraries.

*/

#ifndef _tN2kDef_H_
#define _tN2kDef_H_

#include <stdint.h>

#if !defined(ARDUINO)
extern "C" {
// Application execution delay. Must be implemented by application.
extern void delay(uint32_t ms);

// Current uptime in milliseconds. Must be implemented by application.
extern uint32_t millis();
}
#endif

// Declare PROGMEM macros to nothing on non-AVR targets.
#if !defined(__AVR__) && !defined(ARDUINO)
// ESP8266 provides it's own definition - Do not override it.
#if !defined(ARDUINO_ARCH_ESP8266)
#define PROGMEM
#define pgm_read_byte(var)  *var
#define pgm_read_word(var)  *var
#define pgm_read_dword(var) *var
#endif
#endif

// Definition for the F(str) macro. On Arduinos use what the framework
// provides to utilize the Stream class. On standard AVR8 we declare
// our own helper class which is handled by the N2kStream. On anything
// else we resort to char strings.
#if defined(ARDUINO)
#include <WString.h>
#elif defined(__AVR__)
#include <avr/pgmspace.h>
class __FlashStringHelper;
#define F(str) (reinterpret_cast<const __FlashStringHelper*>(PSTR(str)))
#else
#ifndef F
#define F(str) str
#endif
#endif

#endif
