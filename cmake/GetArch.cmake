
# Set ARCH and LIB_INSTALL_DIR  and PACKAGE_RECS in parent 
# using cmake probes and various heuristics.

#Red Hat:/etc/redhat-release
#Gentoo: /etc/gentoo-release
#Ubuntu: /etc/debian_version
#Suse:   /etc/suse-release or /etc/SuSE-release

# Based on code from nohal
function(GetArch)
  if (NOT WIN32)
    # default
    set (ARCH "x86_64")
    set (LIB_INSTALL_DIR "lib")
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
      if (CMAKE_SIZEOF_VOID_P MATCHES "8")
        set (ARCH "arm64")
      else ()
        set (ARCH "armhf")
      endif ()
    else (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
      set (ARCH ${CMAKE_SYSTEM_PROCESSOR})
    endif ()
    if (EXISTS /etc/debian_version)
      set (PACKAGE_FORMAT "DEB")
      set (PACKAGE_RECS "xcalib,xdg-utils")
      set (LIB_INSTALL_DIR "lib")
      set(TENTATIVE_PREFIX "/usr/local")
    elseif (EXISTS /etc/redhat-release)
      set (LIB_INSTALL_DIR "lib64")
      if (ARCH STREQUAL "arm64")
        set (ARCH "aarch64")
      endif()
    elseif (EXISTS /etc/suse-release OR EXISTS /etc/SuSE-release)
      if (ARCH STREQUAL "arm64")
         set (ARCH "aarch64")
      endif()
    elseif (EXISTS /etc/gentoo-release)
      set (LIB_INSTALL_DIR "lib${LIB_SUFFIX}")
    endif ()
  else (NOT WIN32)
    # On WIN32 probably CMAKE_SIZEOF_VOID_P EQUAL 8, but we don't use it at all now...
    set (ARCH "i386")
  endif (NOT WIN32)
  set (LIB_INSTALL_DIR ${LIB_INSTALL_DIR} PARENT_SCOPE)
  set (ARCH ${ARCH} PARENT_SCOPE)
  set (PACKAGE_FORMAT ${PACKAGE_FORMAT} PARENT_SCOPE)
  set (PACKAGE_RECS ${PACKAGE_RECS} PARENT_SCOPE)
  set (TENTATIVE_PREFIX ${TENTATIVE_PREFIX} PARENT_SCOPE)
endfunction(GetArch)
