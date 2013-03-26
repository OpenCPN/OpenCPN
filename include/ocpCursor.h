#ifndef __OCPCURSOR_H__
#define __OCPCURSOR_H__

#include <wx/cursor.h>

class wxCursorRefData;
class wxString;

class ocpCursor : public wxCursor
{
      public:

            ocpCursor(const wxString& cursorName, long type, int hotSpotX=0, int hotSpotY=0);
            ocpCursor(const char **xpm_data, long type, int hotSpotX=0, int hotSpotY=0);
};

#endif
