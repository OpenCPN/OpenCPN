#include "s52_plib_utils.h"

static class _DefaultUtils : public S52PlibUtils {

    /** Default implementation: return unscaled font. */
    wxFont* GetFont(wxFont*  font, double scale) override { return font; }
    wxFont*  GetDefaultFont( wxString label, int Ptsize) override {return nullptr;}

} DefaultUtils;

static S52PlibUtils* s_the_utils = &DefaultUtils;

S52PlibUtils* GetS52Utils() { return s_the_utils; }
void SetUtils(S52PlibUtils* utils) { s_the_utils = utils; }
