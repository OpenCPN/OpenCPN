/***************************************************************************
 *   Copyright (C) 2006 by dsr   *
 *   dsr@mshome.net   *
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
 ***************************************************************************/


#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdio.h>
#include <assert.h>
#include <pwd.h>
#include <stdbool.h>


int main(int argc, char *argv[])
{
    int option, debuglevel;

    int inF, ouF, bytes;
    char line[512];

    while ((option = getopt(argc, argv, "D:SBUV")) != -1)
    {
        switch (option) {
            case 'B':
            {
                 inF = open("/proc/tty/driver/usb-serial", O_RDONLY);
                 if (inF == -1)
            		inF = open("/proc/tty/driver/usbserial", O_RDONLY);
 
		 if(inF != -1) 
		 {
	                 if((ouF = open("/var/tmp/usbserial", O_WRONLY | O_CREAT | O_TRUNC, 0777)) != -1)
        	         {
                 		while((bytes = read(inF, line, sizeof(line))) > 0)
                     			write(ouF, line, bytes);
                                close(ouF);
			 }
		         close(inF);
                 }
                 break;
            }

            case 'S':
            {
                 inF = open("/proc/tty/driver/serial", O_RDONLY);
 
		 if(inF != -1) 
		 {
	                 if((ouF = open("/var/tmp/serial", O_WRONLY | O_CREAT | O_TRUNC, 0777)) != -1)
        	         {
                 		while((bytes = read(inF, line, sizeof(line))) > 0)
                     			write(ouF, line, bytes);
                                close(ouF);
			 }
		         close(inF);
                 }
                 break;
             }

            case 'U':
            {
                /*  Kill the helper files  */
                unlink("/var/tmp/usbserial");
                unlink("/var/tmp/serial");
		break;
            }

            case 'D':
            {
                debuglevel = (int) strtol(optarg, 0, 0);
                break;
            }
             
            case 'V':
            {
                (void)printf("ocpnhelper %s\n", VERSION);
                exit(0);
            }

            case 'h': case '?':
            default:
            { 
//                usage();
                exit(0);
            }
        }
    }


    return 0;
}


