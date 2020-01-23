CXXFLAGS = -std=gnu++0x -Wall
LDLIBS = -lstdc++
.PHONY: all clean test
all: example-client example-client-cpp11
clean:
	-rm  example-client example-client-cpp11 *.o
	cd test; $(MAKE) clean
test:
	cd test; $(MAKE) test
testserver: node_modules
	node example-server.js
node_modules:
	npm install
example-client-cpp11: example-client-cpp11.o easywsclient.o
example-client-cpp11.o: example-client-cpp11.cpp easywsclient.hpp
example-client: example-client.o easywsclient.o
example-client.o: example-client.cpp easywsclient.hpp
easywsclient.o: easywsclient.cpp easywsclient.hpp
