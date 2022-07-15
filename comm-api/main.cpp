// CFLAGS="-I/usr/lib64/wx/include/gtk3-unicode-3.0 -I/usr/include/wx-3.0 -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__ -pthread -I../include -I../libs/wxJSON/include/"
// gcc -c $CFLAGS main.cpp
#include <iostream>

#include "app_msg.h"
#include "transport.h"
#include "drivers.h"

int main(int argc, char** argv) {
  std::cout << "hello\n";
}
