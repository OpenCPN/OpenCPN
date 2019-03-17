/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2019 Alec Leamas                                        *
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
 */

/**  Handle downloading of files from remote urls. */

#ifndef DOWNLOADER_H__
#define DOWNLOADER_H__

#include <string>
#include <ostream>


/** Default downloader, usable in a CLI context.*/
class Downloader {

    public:
        Downloader(std::string url);

        /** Download url into stream, return false on errors. */
        bool download(std::ostream* stream);

        /**
         * Download url into path. If path is empty, set it to a
         * temporary filename used. Return false on errors.
         */
        bool download(std::string& path);

        /** Last error code, a CURLE return code. */
        int last_errorcode();

        /** Last Curl error message. */
        std::string last_error();

        /** Called when given bytes has been transferred from remote. */
        virtual void on_chunk(const char* buff, unsigned bytes);

    protected:
        /** Try to get remote filesize, return 0 on failure. */
        long get_filesize();

        std::string url;
        std::ostream* stream;
        std::string error_msg;
        int errorcode;
};


#endif // DOWNLOADER_H__
