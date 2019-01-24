/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>

#include <wx/log.h>
#include <wx/time.h>


/**
 * Customized logger class writing on a file providing:
 *   - Millisecond timestamps
 *   - Consistent tagging WARNING, ERROR, MESSAGE etc.
 *   - Filename:line info.
 */
class OcpnLog: public wxLog
{
    public:
        static const wxLogLevel LOG_BADLEVEL;

        /** Create logger appending to given filename. */
        OcpnLog(const char* path);

        virtual ~OcpnLog();

        void DoLogRecord(wxLogLevel level,
                         const wxString& msg,
                         const wxLogRecordInfo& info) override;

        static wxLogLevel str2level(const char* string);

    protected:
        std::ofstream log;

};


#endif // LOGGER_H
