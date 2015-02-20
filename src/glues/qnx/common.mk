NAME=GLUES_CM
EXTRA_SRCVPATH=../../../../source ../../../../source/libtess

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=usr/lib

LIBS+=GLES_CM

include $(MKFILES_ROOT)/qmacros.mk
define PINFO
PINFO DESCRIPTION=GLU 1.3 OpenGL ES Port
endef

SO_VERSION=1

include $(MKFILES_ROOT)/qtargets.mk

ifeq ($(origin NDEBUG),undefined)
CCFLAGS += -O0
else
CCFLAGS += -O3 -funroll-all-loops -Wall
endif
