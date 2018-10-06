/***************************************************************************
 *   Copyright (C) 2007..2010 by David S. Register, Richard M Smith        *
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
 *
 */

#include "wx/wxprec.h"
#ifdef __WXOSX__

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <paths.h>
#include <termios.h>
#include <sysexits.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <AvailabilityMacros.h>

#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#if defined(MAC_OS_X_VERSION_10_3) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_3)
#include <IOKit/serial/ioss.h>
#endif
#include <IOKit/IOBSD.h>

// We need CoreGraphics to read the monitor physical size.
// In 10.7 CoreGraphics is part of ApplicationServices.
#include <ApplicationServices/ApplicationServices.h>
// When we stop building against 10.7 we will probably want to link agains CoreGraphics directly:
//#include <CoreGraphics/CoreGraphics.h>


// Returns an iterator across all known serial ports. Caller is responsible for
// releasing the iterator when iteration is complete.
static kern_return_t FindSerialPorts(io_iterator_t *matchingServices)
{
    kern_return_t			kernResult;
    CFMutableDictionaryRef	classesToMatch;

/*! @function IOServiceMatching
    @abstract Create a matching dictionary that specifies an IOService class match.
    @discussion A very common matching criteria for IOService is based on its class. IOServiceMatching will create a matching dictionary that specifies any IOService of a class, or its subclasses. The class is specified by C-string name.
    @param name The class name, as a const C-string. Class matching is successful on IOService's of this class or any subclass.
    @result The matching dictionary created, is returned on success, or zero on failure. The dictionary is commonly passed to IOServiceGetMatchingServices or IOServiceAddNotification which will consume a reference, otherwise it should be released with CFRelease by the caller. */

    // Serial devices are instances of class IOSerialBSDClient
    classesToMatch = IOServiceMatching(kIOSerialBSDServiceValue);
    if (classesToMatch == NULL)
    {
        printf("IOServiceMatching returned a NULL dictionary.\n");
    }
    else {
/*!
	@function CFDictionarySetValue
	Sets the value of the key in the dictionary.
	@param theDict The dictionary to which the value is to be set. If this
		parameter is not a valid mutable CFDictionary, the behavior is
		undefined. If the dictionary is a fixed-capacity dictionary and
		it is full before this operation, and the key does not exist in
		the dictionary, the behavior is undefined.
	@param key The key of the value to set into the dictionary. If a key
		which matches this key is already present in the dictionary, only
		the value is changed ("add if absent, replace if present"). If
		no key matches the given key, the key-value pair is added to the
		dictionary. If added, the key is retained by the dictionary,
		using the retain callback provided
		when the dictionary was created. If the key is not of the sort
		expected by the key retain callback, the behavior is undefined.
	@param value The value to add to or replace into the dictionary. The value
		is retained by the dictionary using the retain callback provided
		when the dictionary was created, and the previous value if any is
		released. If the value is not of the sort expected by the
		retain or release callbacks, the behavior is undefined.
*/
        CFDictionarySetValue(classesToMatch,
                             CFSTR(kIOSerialBSDTypeKey),
                             CFSTR(kIOSerialBSDRS232Type));

		// Each serial device object has a property with key
        // kIOSerialBSDTypeKey and a value that is one of kIOSerialBSDAllTypes,
        // kIOSerialBSDModemType, or kIOSerialBSDRS232Type. You can experiment with the
        // matching by changing the last parameter in the above call to CFDictionarySetValue.

        // As shipped, this sample is only interested in serial ports,
        // so add this property to the CFDictionary we're matching on.
        // This will find devices that advertise themselves as serial ports,
        // such as built-in and USB serial ports. However, this match won't find serial serial ports.
    }

    /*! @function IOServiceGetMatchingServices
        @abstract Look up registered IOService objects that match a matching dictionary.
        @discussion This is the preferred method of finding IOService objects currently registered by IOKit. IOServiceAddNotification can also supply this information and install a notification of new IOServices. The matching information used in the matching dictionary may vary depending on the class of service being looked up.
        @param masterPort The master port obtained from IOMasterPort().
        @param matching A CF dictionary containing matching information, of which one reference is consumed by this function. IOKitLib can contruct matching dictionaries for common criteria with helper functions such as IOServiceMatching, IOOpenFirmwarePathMatching.
        @param existing An iterator handle is returned on success, and should be released by the caller when the iteration is finished.
        @result A kern_return_t error code. */

    kernResult = IOServiceGetMatchingServices(kIOMasterPortDefault, classesToMatch, matchingServices);
    if (KERN_SUCCESS != kernResult)
    {
        printf("IOServiceGetMatchingServices returned %d\n", kernResult);
		goto exit;
    }

exit:
    return kernResult;
}

// Given an iterator across a set of serial ports, return the BSD path to the first one.
// If no serial ports are found the path name is set to an empty string.
static int GetSerialPortPath(io_iterator_t serialPortIterator, char** pNames, int iMaxNames, CFIndex maxPathSize)
{
    io_object_t		modemService;
//    kern_return_t	kernResult = KERN_FAILURE;
    Boolean			modemFound = false;
    char bsdPath[maxPathSize] ;
	int				iCurrentNameIndex = 0 ;
    // Initialize the returned path
    *bsdPath = '\0';

    // Iterate across all serial ports found.

    while ((modemService = IOIteratorNext(serialPortIterator)) && !modemFound)
    {
        CFTypeRef	bsdPathAsCFString;

		// Get the callout device's path (/dev/cu.xxxxx). The callout device should almost always be
		// used: the dialin device (/dev/tty.xxxxx) would be used when monitoring a serial port for
		// incoming calls, e.g. a fax listener.

		bsdPathAsCFString = IORegistryEntryCreateCFProperty(modemService,
                                                            CFSTR(kIOCalloutDeviceKey),
                                                            kCFAllocatorDefault,
                                                            0);
        if (bsdPathAsCFString)
        {
            Boolean result;

            // Convert the path from a CFString to a C (NUL-terminated) string for use
			// with the POSIX open() call.

			result = CFStringGetCString(bsdPathAsCFString,
                                        bsdPath,
                                        maxPathSize,
                                        kCFStringEncodingUTF8);
            CFRelease(bsdPathAsCFString);

            if (result)
			{
				pNames[iCurrentNameIndex] = calloc(1,strlen(bsdPath)+1) ;
				strncpy(pNames[iCurrentNameIndex],bsdPath,strlen(bsdPath)+1);
				iCurrentNameIndex++ ;
            }
        }
        // Release the io_service_t now that we are done with it.
		(void) IOObjectRelease(modemService);

    }
    return iCurrentNameIndex ;
}

int FindSerialPortNames(char** pNames, int iMaxNames)
{
	int iActiveNameCount = 0 ;
    kern_return_t	kernResult; // on PowerPC this is an int (4 bytes)

    io_iterator_t	serialPortIterator;

    kernResult = FindSerialPorts(&serialPortIterator);

    iActiveNameCount = GetSerialPortPath(serialPortIterator, pNames, iMaxNames, MAXPATHLEN);

    IOObjectRelease(serialPortIterator);	// Release the iterator.
	return iActiveNameCount ;
}

bool ValidateSerialPortName(char* pPortName, int iMaxNamestoSearch)
{
	char* paPortNames[iMaxNamestoSearch] ;
	int iPortNameCount ;
	int iPortIndex ;
	bool bPortFound = false ;
	char* pPortSubName = index(pPortName,':') ;
	// name is always valid if opetion is set to 'none'
	if (0 == strcasecmp(pPortName,"NONE"))
		return true ;
	// if this name done not have a leading descriptor with a 'serial:', 'GPS:', etc, use the whole name
	if (NULL == pPortSubName)
		pPortSubName = pPortName ;
	else
		pPortSubName++ ;

	memset(paPortNames,0x00,sizeof(paPortNames)) ;
	iPortNameCount = FindSerialPortNames(&paPortNames[0],iMaxNamestoSearch) ;
	for ( iPortIndex=0;iPortIndex<iPortNameCount;iPortIndex++)
	{
		//stripoff leading 'serial:', etc based on iColonIndex
		int iStrCompresult = strcmp(paPortNames[iPortIndex],pPortSubName)  ;
		if (false == bPortFound )
			bPortFound = (bool) ( 0 == iStrCompresult) ;
		free(paPortNames[iPortIndex]) ;
	}
	return bPortFound ;
}


/**
 * Returns the width of the monitor in millimetres
 */
int GetMacMonitorSize()
{
    CGSize displayPhysicalSize = CGDisplayScreenSize(CGMainDisplayID()); // mm
    return displayPhysicalSize.width;
}

#endif            //__WXOSX__
