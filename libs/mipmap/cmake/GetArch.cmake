# Set ARCH in parent using cmake probes and various heuristics.
# Based on code from nohal

function(GetArch)
  # Set up ARCH
  if (NOT OCPN_TARGET_TUPLE STREQUAL "")
    list(GET OCPN_TARGET_TUPLE 2 ARCH)
  elseif (WIN32)
    # Should really be i386 since we are on win32. However, it's x86_64 for now,
    # see #2027
    set (ARCH "x86_64")
  else ()
    # Defaults:
    set (ARCH "x86_64")
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
      if (CMAKE_SIZEOF_VOID_P MATCHES "8")
        set (ARCH "arm64")
      else ()
        set (ARCH "armhf")
      endif ()
    else (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
      set (ARCH ${CMAKE_SYSTEM_PROCESSOR})
    endif ()
    if (ARCH STREQUAL "arm64")
      if (OCPN_FLATPAK)
        set(ARCH "aarch64")
      elseif (EXISTS /etc/redhat-release)
        set (ARCH "aarch64")
      elseif (EXISTS /etc/suse-release OR EXISTS /etc/SuSE-release)
        set (ARCH "aarch64")
      endif ()
    endif ()
  endif ()

  set(ARCH ${ARCH} PARENT_SCOPE)
endfunction (GetArch)
