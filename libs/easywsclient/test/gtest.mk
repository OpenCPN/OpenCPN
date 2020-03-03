# gtest.mk
# Uses the source code version of GoogleTest, assumed to be installed at /usr/src/gtest.
GTEST_PATH ?= /usr/src/gtest
GTEST_OBJS = gtest-all.o
LDLIBS += -lpthread
LDLIBS += -lstdc++
vpath %.cc $(GTEST_PATH)/src

GTEST_FOUND = $(shell ls -d $(GTEST_PATH)/src/gtest-all.cc)
ifeq (,$(GTEST_FOUND))
  $(info )
  $(info Error:)
  $(info GoogleTest sources were not found at the path:)
  $(info GTEST_PATH=$(GTEST_PATH))
  $(info Suggested corrective action: sudo apt-get install libgtest-dev)
  $(info )
  $(error Please setup GTEST_PATH so that gtest-all.cc can be found.)
endif

.PHONY: all gtest_clean
all:
gtest_clean:
	-rm gtest-all.o
gtest-all.o: CXXFLAGS += -I$(GTEST_PATH)
