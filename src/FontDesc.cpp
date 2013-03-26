#include "FontDesc.h"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(FontList);

MyFontDesc::MyFontDesc( wxString DialogString, wxString ConfigString, wxFont *pFont,
        wxColour color )
{
    m_dialogstring = DialogString;
    m_configstring = ConfigString;

    m_nativeInfo = pFont->GetNativeFontInfoDesc();

    m_font = pFont;
    m_color = color;
}

MyFontDesc::~MyFontDesc()
{
    delete m_font;
}

