#ifndef UseR_ColoRs_h
#define UseR_ColoRs_h

#include "color_handler.h"

namespace user_colors {

void Initialize();
void DeInitialize();

/** Return a hash table for given scheme name or nullptr if not found. */
wxColorHashMap* GetMapByScheme(const std::string& scheme_name);

ColorScheme GetColorScheme();

};  // namespace user_colors

#endif  // UseR_ColoRs_h
