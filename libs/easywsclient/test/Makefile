include gtest.mk

CXXFLAGS = -std=gnu++1y -Wall -I..
LDLIBS +=
.PHONY: all clean
test: easywsclient.t
	node testServer.js & sleep 1 && ./easywsclient.t
clean:
	-rm  easywsclient.t *.o
vpath %.cpp ../
vpath %.hpp ../
easywsclient.t: easywsclient.t.o easywsclient.o gtest-all.o
easywsclient.o: easywsclient.cpp easywsclient.hpp
