
# Set ARCH and LIB_INSTALL_DIR  and PACKAGE_RECS in parent 
# using cmake probes and various heuristics.

#Red Hat:   /etc/redhat-release
#Slackware: /etc/slackware-version
#Slamd64:   /etc/slamd64-version
#Gentoo: /etc/gentoo-release
#Ubuntu: /etc/debian_version

# Based on code from nohal
function(GetArch)
    if (NOT WIN32)
    # default
    set (ARCH "i386")
    set (LIB_INSTALL_DIR "lib")
    if (EXISTS /etc/debian_version)
      set (PACKAGE_FORMAT "DEB")
      set (PACKAGE_RECS "xcalib,xdg-utils")
      set (LIB_INSTALL_DIR "lib")
      if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
        if (CMAKE_SIZEOF_VOID_P MATCHES "8")
          set (ARCH "arm64")
        else ()
          set (ARCH "armhf")
        endif ()
      else (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
        if (CMAKE_SIZEOF_VOID_P MATCHES "8")
          set (ARCH "amd64")
        else ()
          set (ARCH "i386")
        endif ()
      endif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
      set(TENTATIVE_PREFIX "/usr/local")
    endif (EXISTS /etc/debian_version)
    if (EXISTS /etc/redhat-release)
      set (PACKAGE_FORMAT "TBZ2")
      if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
        if (CMAKE_SIZEOF_VOID_P MATCHES "8")
          set (ARCH "aarch64")
        else ()
          set (ARCH "armhf")
        endif ()
      else (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
        if (CMAKE_SIZEOF_VOID_P MATCHES "8")
          set (ARCH "x86_64")
          set (LIB_INSTALL_DIR "lib64")
        else ()
          set (ARCH "i386")
          set (LIB_INSTALL_DIR "lib")
        endif ()
      endif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
    endif (EXISTS /etc/redhat-release)
    if (EXISTS /etc/suse-release OR EXISTS /etc/SuSE-release)
      set (PACKAGE_FORMAT "TBZ2")
      if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
        if (CMAKE_SIZEOF_VOID_P MATCHES "8")
          set (ARCH "aarch64")
        else ()
          set (ARCH "armhf")
        endif ()
      else (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
        if (cMAKE_SIZEOF_VOID_P MATCHES "8")
          set (ARCH "x86_64")
          set (LIB_INSTALL_DIR "lib")
          # In recent openSUSE versions (as of 2016), lib64 is mostly 
          # used when there are both 32-bit and 64-bit versions, although 
          # not limited to that. This CMake variable only affects the location
          # of OpenCPN plugins at installation time, and nothing more than that.
          # At run time, the plugin directory is determined by 
          # wxStandardPaths::GetPluginsDir(), which returns "lib", so this can 
          # be considered canonical.
        else ()
          set (ARCH "i386")
          set (LIB_INSTALL_DIR "lib")
        endif ()
      endif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
    endif (EXISTS /etc/suse-release OR EXISTS /etc/SuSE-release)
    if (EXISTS /etc/gentoo-release)
      set (LIB_INSTALL_DIR "lib${LIB_SUFFIX}")
    endif ()
    if (APPLE)
      if (CMAKE_SIZEOF_VOID_P MATCHES "8")
        set(ARCH "x86_64")
      endif ()
    endif (APPLE)
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
