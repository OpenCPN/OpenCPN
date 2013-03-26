#ifndef __HYPERLINK_H__
#define __HYPERLINK_H__

#include <wx/string.h>
#include <wx/list.h>

class Hyperlink // toh, 2009.02.14
{
      public:
            wxString DescrText;
            wxString Link;
            wxString Type;
};

WX_DECLARE_LIST(Hyperlink, HyperlinkList);// establish class as list member

#endif
