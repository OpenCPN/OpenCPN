##---------------------------------------------------------------------------
## Author:      Dave Register
## DEB config from: antonm - Anton Martchukov <anton@martchukov.com>
##***************************************************************************
## *   Copyright (C) 2010 by David S. Register                               *
## *   This program is free software; you can redistribute it and/or modify  *
## *   it under the terms of the GNU General Public License as published by  *
## *   the Free Software Foundation; either version 2 of the License, or     *
## *   (at your option) any later version.                                   *
## *                                                                         *
## *   This program is distributed in the hope that it will be useful,       *
## *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
## *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
## *   GNU General Public License for more details.                          *
## *                                                                         *
## *   You should have received a copy of the GNU General Public License     *
## *   along with this program; if not, write to the                         *
## *   Free Software Foundation, Inc.,                                       *
## *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
## ***************************************************************************

# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)
MESSAGE (STATUS "*** Staging to build PlugIns ***")

# These variables are inherited by all plugins
SET(PARENT opencpn)
SET(PREFIX_DATA share)
SET(PREFIX_PLUGINS ${PREFIX_LIB}/${PARENT})

SET(STANDALONE "BUNDLED")

FILE(GLOB PLUGINS *_pi)

FOREACH(PLUGIN ${PLUGINS})
     add_subdirectory (${PLUGIN})
     MESSAGE(STATUS "*** Added plugin: ${PLUGIN}")
ENDFOREACH(PLUGIN)

OPTION(OCPN_BUILD_SAMPLE "Build sample plugin" OFF)

IF(OCPN_BUILD_SAMPLE)
    FILE(GLOB PLUGINS_SAMPLE *_pi_sample)

    FOREACH(PLUGIN ${PLUGINS_SAMPLE})
        add_subdirectory (${PLUGIN})
     MESSAGE(STATUS "*** Added sample plugin: ${PLUGIN}")
    ENDFOREACH(PLUGIN)
ENDIF(OCPN_BUILD_SAMPLE)

