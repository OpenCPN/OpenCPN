#[[
Copyright (C) 2025 by NoCodeHummel
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.
#]]

# OpenCPN UI components library
# with dependency on Android utils.
cmake_minimum_required(VERSION 3.15.0)
set(CMAKE_CXX_STANDARD 17)

if (TARGET ocpn::guilib)
    return ()
endif ()

# Sources
set(SRC
    include/button_switch.h
    include/dialog_alert.h
    include/dialog_base.h
    include/dialog_footer.h
    include/dialog_input.h
    include/field_switch.h
    include/field_text.h
    include/edit_button.h
    include/form_grid.h
    include/svg_icons.h
    include/text_entry.h
    include/ui_utils.h
    src/button_switch.cpp
    src/dialog_alert.cpp
    src/dialog_base.cpp
    src/edit_button.cpp
    src/dialog_input.cpp
    src/field_switch.cpp
    src/field_text.cpp
    src/form_grid.cpp
    src/svg_icons.cpp
    src/text_entry.cpp
    src/ui_utils.cpp
)

add_library(GUILIB STATIC ${SRC})
add_library(ocpn::guilib ALIAS GUILIB)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU")  # Apple is AppleClang
  target_compile_options(
    GUILIB PRIVATE
      -fvisibility=default -Wno-unknown-pragmas -fPIC
  )
endif ()

target_include_directories(GUILIB PRIVATE ${wxWidgets_INCLUDE_DIRS})
target_link_libraries(GUILIB PRIVATE ${wxWidgets_LIBRARIES})
if (TARGET ocpn::wxsvg)
   target_link_libraries(GUILIB PRIVATE ocpn::wxsvg)
endif ()

target_include_directories(
  GUILIB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
