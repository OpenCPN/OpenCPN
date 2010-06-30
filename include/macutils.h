/*
 *  macutils.h
 *  openCPNOSx
 *
 *  Created by Richard Smith on 7/5/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

// begin rms
#ifndef MACUTILS_H_INCLUDED_H__
#define MACUTILS_H_INCLUDED_H__
#ifdef __WXOSX__
#define MAX_SERIAL_PORTS 10

extern "C" int FindSerialPortNames(char** pNames, int iMaxNames) ;
extern "C" bool ValidateSerialPortName(const char* pPortName, int iMaxNamesToSearch) ;

#endif // __WXOSX__
// end rms

#endif // MACUTILS_H_INCLUDED_H__