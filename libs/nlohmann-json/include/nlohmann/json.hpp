// https://github.com/nlohmann/json/issues/1408
#ifdef _MSC_VER
#undef snprintf
#endif

// See "Implicit Conversions" in https://github.com/nlohmann/json/README.md
#define JSON_USE_IMPLICIT_CONVERSIONS 0

#include "../../json/single_include/nlohmann/json.hpp"
