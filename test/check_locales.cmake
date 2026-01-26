# ~~~
# Summary:     Configure locale_conf.h header
# License:     GPLv3+
# Copyright (c) 2026 Alec Leamas
#
# Using the files LOCALES genererated by check_locales.cpp  configure
# locale_conf.h so it reflects available test locales.
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.


file(READ ${LOCALES} locales)

string(FIND "${locales}" "en_US" result)
if ("${result}" EQUAL -1)
  file(WRITE ${HEADER_DIR}/locale_conf.h "// en_US not found\n")
else ()
  file(WRITE ${HEADER_DIR}/locale_conf.h "#define HAS_EN_US\n")
  message(STATUS "Found en_US.UTF8 locale")
endif ()

string(FIND "${locales}" "sv_SE" result)
if ("${result}" EQUAL -1)
  file(WRITE ${HEADER_DIR}/locale_conf.h "// sv_SE not found\n")
else ()
  file(APPEND ${HEADER_DIR}/locale_conf.h "#define HAS_SV_SE\n")
  message(STATUS "Found sv_SE.UTF8 locale")
endif ()
