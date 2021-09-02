#include "semantic_vers.h"
#include <iostream>

// c++   -I../include -I../build/include  testvers.cpp semantic_vers.cpp
// a.out 1.4.6 7.8.9+deadbee

int main(int argc, char** argv) {
  SemanticVersion v1;
  SemanticVersion v2;
  if (argc > 1) {
    auto tmp = SemanticVersion::parse(argv[1]);
    v1 = tmp;
    std::cout << "First version: " << v1 << "\n";
  }
  if (argc > 2) {
    auto tmp = SemanticVersion::parse(argv[2]);
    v2 = tmp;
    std::cout << "Second version: " << v2 << "\n";
  } else {
    exit(0);
  }
  std::cout << "1 > 2"
            << " " << (v1 > v2 ? "True" : "False") << "\n";
  std::cout << "1 < 2"
            << " " << (v1 < v2 ? "True" : "False") << "\n";
  std::cout << "1 == 2"
            << " " << (v1 == v2 ? "True" : "False") << "\n";
  std::cout << "1 != 2"
            << " " << (v1 != v2 ? "True" : "False") << "\n";
  std::cout << "1 <= 2"
            << " " << (v1 <= v2 ? "True" : "False") << "\n";
  std::cout << "1 >= 2"
            << " " << (v1 >= v2 ? "True" : "False") << "\n";
}
