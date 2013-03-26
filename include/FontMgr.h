#ifndef __FONTMGR_H__
#define __FONTMGR_H__

#include "FontDesc.h"

class FontMgr
{
public:
      FontMgr();
      ~FontMgr();

      wxFont *GetFont(const wxString &TextElement, int default_size = 0);
      wxColour GetFontColor( const wxString &TextElement );

      int GetNumFonts(void);
      wxString *GetConfigString(int i);
      wxString *GetDialogString(int i);
      wxString *GetNativeDesc(int i);
      wxString GetFullConfigDesc ( int i );
      static wxString GetFontConfigKey( const wxString &description );

      void LoadFontNative(wxString *pConfigString, wxString *pNativeDesc);
      bool SetFont(wxString &TextElement, wxFont *pFont, wxColour color);

private:
      wxString GetSimpleNativeFont(int size);

      FontList          *m_fontlist;
      wxFont            *pDefFont;

};

#endif
