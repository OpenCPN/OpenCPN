#ifndef S52_PLIB_UTILS_H
#define S52_PLIB_UTILS_H

#include <wx/font.h>

/** Interface implemented by calling code */
class S52PlibUtils {
public:
    /** Get a scaled font based on given; created if not already existing. */
    virtual wxFont* GetFont(wxFont*  font, double scale) = 0;
    virtual wxFont* GetDefaultFont( wxString label, int Ptsize) = 0;
};

S52PlibUtils* GetS52Utils();

void SetUtils(S52PlibUtils* utils);
#endif  // S52_PLIB_UTILS_H
