#ifndef __FONTDESC_H__
#define __FONTDESC_H__

#include <wx/string.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/list.h>

class MyFontDesc
{
public:

      MyFontDesc(wxString DialogString, wxString ConfigString, wxFont *pFont, wxColour color);
      ~MyFontDesc();

      wxString    m_dialogstring;
      wxString    m_configstring;
      wxString    m_nativeInfo;
      wxFont      *m_font;
      wxColour    m_color;
};


WX_DECLARE_LIST(MyFontDesc, FontList);

#endif
