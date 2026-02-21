/**************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Implement font_mgr.h -- font list manager
 */

#include <algorithm>
#include <locale>
#include <set>

#include <wx/gdicmn.h>
#include <wx/tokenzr.h>

#include "gl_headers.h"  // Must come before anything using GL stuff

#include "model/config_vars.h"
#include "font_mgr.h"
#include "ocpn_platform.h"
#include "ocpn_plugin.h"

struct font_cache_record {
  wxFont *font;
  int pointsize_req;
  wxFontStyle style_req;
  wxFontWeight weight_req;
  bool underline_req;
};

class OCPNwxFontList {
public:
  ~OCPNwxFontList() { FreeAll(); }
  wxFont *FindOrCreateFont(int pointSize, wxFontFamily family,
                           wxFontStyle style, wxFontWeight weight,
                           bool underline = false,
                           const wxString &face = wxEmptyString,
                           wxFontEncoding encoding = wxFONTENCODING_DEFAULT);
  void FreeAll();

private:
  bool isCached(font_cache_record &record, int pointSize, wxFontFamily family,
                wxFontStyle style, wxFontWeight weight, bool underline,
                const wxString &facename, wxFontEncoding encoding);

  std::vector<font_cache_record> m_fontVector;
};

/**
 * Static copy of effective UI locale.
 *
 * Used by FontMgr to support locale-specific font configurations. Allows for
 * different fonts per language.
 * Updated whenever UI locale changes via platform ChangeLocale().
 * @see g_locale for main locale setting
 */
static wxString s_locale;

FontMgr *FontMgr::instance = NULL;

FontMgr &FontMgr::Get() {
  if (!instance) instance = new FontMgr;
  return *instance;
}

void FontMgr::Shutdown() {
  if (instance) {
    delete instance;
    instance = NULL;
  }
}

FontMgr::FontMgr() : m_wxFontCache(NULL), m_fontlist(NULL), pDefFont(NULL) {
  //    Create the list of fonts
  m_fontlist = new FontList;

  s_locale = g_locale;

  //    Get a nice generic font as default
  pDefFont = FindOrCreateFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                              wxFONTWEIGHT_BOLD, FALSE, wxString(""),
                              wxFONTENCODING_SYSTEM);
}

FontMgr::~FontMgr() {
  for (auto it = m_fontlist->begin(); it != m_fontlist->end(); it++) delete *it;
  m_fontlist->clear();
  delete m_fontlist;

  delete m_wxFontCache;
}

void FontMgr::SetLocale(wxString &newLocale) { s_locale = newLocale; }

wxColour FontMgr::GetFontColor(const wxString &TextElement) const {
  MyFontDesc *pmfd = GetFontDesc(TextElement);
  return pmfd ? pmfd->m_color : wxColour(0, 0, 0);
}

bool FontMgr::SetFontColor(const wxString &TextElement,
                           const wxColour color) const {
  MyFontDesc *pmfd = GetFontDesc(TextElement);
  if (pmfd) {
    pmfd->m_color = color;
    return true;
  }
  return false;
}

wxString FontMgr::GetFontConfigKey(const wxString &description) {
  // Create the configstring by combining the locale with
  // a hash of the font description. Hash is used because the i18n
  // description can contain characters that mess up the config file.

  wxString configkey;
  configkey = s_locale;
  configkey.Append("-");

  using namespace std;
  locale loc;
  const collate<char> &coll = use_facet<collate<char> >(loc);
  //    char cFontDesc[101];
  //    wcstombs( cFontDesc, description.c_str(), 100 );
  //    cFontDesc[100] = 0;

  wxCharBuffer abuf = description.ToUTF8();

  int fdLen = strlen(abuf);

  configkey.Append(
      wxString::Format("%08lx", coll.hash(abuf.data(), abuf.data() + fdLen)));
  return configkey;
}

int FontMgr::GetSystemFontSize() {
  static int sys_font_size = 0;
  if (!sys_font_size) {
    wxFont sys_font = *wxNORMAL_FONT;
    sys_font_size = sys_font.GetPointSize();
  }
  return sys_font_size;
}

wxString FontMgr::GetSystemFontFaceName() {
  static wxString sys_font_facename;
  if (sys_font_facename.IsEmpty()) {
    wxFont sys_font = *wxNORMAL_FONT;
    sys_font_facename = sys_font.GetFaceName();
  }
  return sys_font_facename;
}

bool FontMgr::IsDefaultFontEntry(const MyFontDesc *font_desc) const {
  return font_desc && font_desc->m_is_default;
  /*
  if (!font_desc || !font_desc->m_font) return false;

  int font_size = font_desc->m_font->GetPointSize();
  return (g_default_font_size && font_size == g_default_font_size) ||
         (!g_default_font_size && font_size == GetSystemFontSize());
  */
}

/*  Support Legacy (Prior to O5.12) plugin API interface */
wxFont *FontMgr::GetFontLegacy(const wxString &TextElement,
                               int user_default_size) {
  //    Look thru the font list for a match
  for (auto node = m_fontlist->begin(); node != m_fontlist->end(); ++node) {
    MyFontDesc *pmfd = *node;
    if (pmfd->m_dialogstring == TextElement) {
      if (pmfd->m_configstring.BeforeFirst('-') == s_locale)
        return pmfd->m_font;
    }
  }
  return GetFont(TextElement, user_default_size);
}

wxFont *FontMgr::GetFont(const wxString &TextElement, int requested_font_size) {
  // Look thru the font list for a match
  for (auto node = m_fontlist->begin(); node != m_fontlist->end(); ++node) {
    MyFontDesc *pmfd = *node;
    // Check if the font matches both the text element and the requested size.
    if (pmfd->m_dialogstring == TextElement &&
        (pmfd->m_configstring.BeforeFirst('-') == s_locale)) {
      if (requested_font_size == 0 && IsDefaultFontEntry(pmfd)) {
        // Caller did not specify a font size, so return the default font.
        // The user may have customized the default font in Options->User Fonts,
        // else the system default font is used.
        return pmfd->m_font;
      } else if (requested_font_size != 0 &&
                 pmfd->m_font->GetPointSize() == requested_font_size) {
        return pmfd->m_font;
      }
    }
  }

  // Found no font, so create a nice one and add to the list
  wxString configkey = GetFontConfigKey(TextElement);

  //    Now create a benign, always present native font
  //    with optional user requested default size

  int new_size;
  if (0 == requested_font_size) {
    new_size = g_default_font_size ? g_default_font_size : GetSystemFontSize();
  } else
    new_size = requested_font_size;

  wxString face_name = g_default_font_facename.Length()
                           ? g_default_font_facename
                           : GetSystemFontFaceName();

  wxString nativefont = GetSimpleNativeFont(new_size, face_name);
  wxFont *nf = wxFont::New(nativefont);

  wxColor color = GetDefaultFontColor(TextElement);

  bool is_default = (requested_font_size == 0);
  MyFontDesc *pnewfd =
      new MyFontDesc(TextElement, configkey, nf, color, is_default);
  m_fontlist->push_back(pnewfd);

  return pnewfd->m_font;
}

MyFontDesc *FontMgr::GetFontDesc(const wxString &TextElement) const {
  // First try to find the default font entry
  for (auto node = m_fontlist->begin(); node != m_fontlist->end(); ++node) {
    MyFontDesc *pmfd = *node;
    if (pmfd->m_dialogstring == TextElement && pmfd->m_is_default &&
        pmfd->m_configstring.BeforeFirst('-') == s_locale) {
      return pmfd;
    }
  }

  // No default entry found, fall back to first matching entry
  for (auto node = m_fontlist->begin(); node != m_fontlist->end(); ++node) {
    MyFontDesc *pmfd = *node;
    if (pmfd->m_dialogstring == TextElement &&
        pmfd->m_configstring.BeforeFirst('-') == s_locale) {
      return pmfd;
    }
  }
  return NULL;
}

wxColour FontMgr::GetDefaultFontColor(const wxString &TextElement) {
  wxColor defaultColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);

  // Special cases here
  if (TextElement.IsSameAs(_("Console Legend")))
    defaultColor = wxColour(0, 255, 0);
  else if (TextElement.IsSameAs(_("Console Value")))
    defaultColor = wxColour(0, 255, 0);
  else if (TextElement.IsSameAs(_("Marks")))
    defaultColor = wxColour(0, 0, 0);
  else if (TextElement.IsSameAs(_("RouteLegInfoRollover")))
    defaultColor = wxColour(0, 0, 0);
  else if (TextElement.IsSameAs(_("AISRollover")))
    defaultColor = wxColour(0, 0, 0);
  else if (TextElement.IsSameAs(_("ExtendedTideIcon")))
    defaultColor = wxColour(0, 0, 0);
  else if (TextElement.IsSameAs(_("ChartTexts")))
    defaultColor = wxColour(0, 0, 0);
  else if (TextElement.IsSameAs(_("AIS Target Name")))
    defaultColor = wxColour(0, 0, 0);
#ifdef __WXMAC__
  // Override, to adjust for light/dark mode
  return wxColour(0, 0, 0);
#endif

  return defaultColor;
}

wxString FontMgr::GetSimpleNativeFont(int size, wxString face) {
  //    Now create a benign, always present native string
  wxString nativefont;

  // this should work for all platforms
  nativefont = wxFont(size, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                      wxFONTWEIGHT_NORMAL, false, face)
                   .GetNativeFontInfoDesc();

  return nativefont;
}

bool FontMgr::SetFont(const wxString &TextElement, wxFont *pFont,
                      wxColour color) {
  //    Look thru the font list for a match
  MyFontDesc *pmfd = GetFontDesc(TextElement);
  // Todo Think about this
  //
  //      Cannot delete the present font, since it may be in use elsewhere
  //      This WILL leak....but only on font changes
  //              delete pmfd->m_font;                            // purge
  //              any old value
  if (pmfd) {
    pmfd->m_font = pFont;
    pmfd->m_nativeInfo = pFont->GetNativeFontInfoDesc();
    pmfd->m_color = color;

    return true;
  }
  return false;
}

int FontMgr::GetNumFonts() const { return m_fontlist->size(); }

const wxString &FontMgr::GetConfigString(int i) const {
  auto it = m_fontlist->begin();
  std::advance(it, i);
  return (*it)->m_configstring;
}

const wxString &FontMgr::GetDialogString(int i) const {
  auto it = m_fontlist->begin();
  std::advance(it, i);
  return (*it)->m_dialogstring;
}

wxArrayString FontMgr::GetDialogStrings(const wxString &locale) const {
  std::set<wxString> uniqueStrings;

  for (auto node = m_fontlist->begin(); node != m_fontlist->end(); ++node) {
    MyFontDesc *pmfd = *node;
    if (locale.IsEmpty() || pmfd->m_configstring.BeforeFirst('-') == locale) {
      uniqueStrings.insert(pmfd->m_dialogstring);
    }
  }
  wxArrayString strings;
  strings.reserve(uniqueStrings.size());  // Pre-allocate for efficiency
  for (const auto &str : uniqueStrings) {
    strings.Add(str);
  }

  return strings;
}

const wxString &FontMgr::GetNativeDesc(int i) const {
  auto it = m_fontlist->begin();
  std::advance(it, i);
  return (*it)->m_nativeInfo;
}

wxString FontMgr::GetFullConfigDesc(int i) const {
  auto it = m_fontlist->begin();
  std::advance(it, i);
  MyFontDesc *pfd = *it;
  wxString ret = pfd->m_dialogstring;
  ret.Append(":");
  ret.Append(pfd->m_nativeInfo);
  ret.Append(":");

  wxString cols("rgb(0,0,0)");
  if (pfd->m_color.IsOk()) cols = pfd->m_color.GetAsString(wxC2S_CSS_SYNTAX);

  ret.Append(cols);
  return ret;
}

MyFontDesc *FontMgr::FindFontByConfigString(wxString pConfigString) {
  //    Search for a match in the list
  for (auto node = m_fontlist->begin(); node != m_fontlist->end(); ++node) {
    MyFontDesc *pmfd = *node;
    if (pmfd->m_configstring == pConfigString) {
      return pmfd;
    }
  }
  return NULL;
}

void FontMgr::LoadFontNative(wxString *pConfigString, wxString *pNativeDesc) {
  //    Parse the descriptor string

  wxStringTokenizer tk(*pNativeDesc, ":");
  wxString dialogstring = tk.GetNextToken();
  wxString nativefont = tk.GetNextToken();

  wxString c = tk.GetNextToken();
  wxColour color(c);  // from string description

  //    Search for a match in the list
  auto node = m_fontlist->begin();
  for (; node != m_fontlist->end(); ++node) {
    MyFontDesc *pmfd = *node;
    if (pmfd->m_configstring == *pConfigString) {
      if (pmfd->m_configstring.BeforeFirst('-') == g_locale) {
        pmfd->m_nativeInfo = nativefont;
        wxFont *nf = pmfd->m_font->New(pmfd->m_nativeInfo);
        pmfd->m_font = nf;
        break;
      }
    }
  }

  //    Create and add the font to the list
  if (node == m_fontlist->end()) {
    wxFont *nf0 = new wxFont();

#ifdef __ANDROID__
    wxFont *nf = new wxFont(nativefont);
#else
    wxFont *nf = nf0->New(nativefont);
#endif

    double font_size = nf->GetPointSize();
    wxString s = nf->GetNativeFontInfoDesc();

    //    Scrub the native font string for bad unicode conversion
#ifdef __WXMSW__
    wxString face = nf->GetFaceName();
    const wxChar *t = face.c_str();
    if (*t > 255) {
      delete nf;
      wxString substitute_native = GetSimpleNativeFont(12, "");
      nf = nf0->New(substitute_native);
    }
#endif
    delete nf0;

    MyFontDesc *pnewfd =
        new MyFontDesc(dialogstring, *pConfigString, nf, color);
    m_fontlist->push_back(pnewfd);
  }
}

wxFont *FontMgr::FindOrCreateFont(int point_size, wxFontFamily family,
                                  wxFontStyle style, wxFontWeight weight,
                                  bool underline, const wxString &facename,
                                  wxFontEncoding encoding) {
  if (m_wxFontCache == 0) m_wxFontCache = new OCPNwxFontList;
  return m_wxFontCache->FindOrCreateFont(point_size, family, style, weight,
                                         underline, facename, encoding);
}

bool OCPNwxFontList::isCached(font_cache_record &record, int pointSize,
                              wxFontFamily family, wxFontStyle style,
                              wxFontWeight weight, bool underline,
                              const wxString &facename,
                              wxFontEncoding encoding) {
  if (record.pointsize_req == pointSize && record.style_req == style &&
      record.weight_req == weight && record.underline_req == underline) {
    bool same;

    wxFont *font = record.font;
    // empty facename matches anything at all: this is bad because
    // depending on which fonts are already created, we might get back
    // a different font if we create it with empty facename, but it is
    // still better than never matching anything in the cache at all
    // in this case
    if (!facename.empty()) {
      const wxString &fontFace = font->GetFaceName();

      // empty facename matches everything
      same = !fontFace || fontFace == facename;
    } else {
      same = font->GetFamily() == family;
    }
    if (same && (encoding != wxFONTENCODING_DEFAULT)) {
      // have to match the encoding too
      same = font->GetEncoding() == encoding;
    }
    return same;
  }
  return false;
}

wxFont *OCPNwxFontList::FindOrCreateFont(int pointSize, wxFontFamily family,
                                         wxFontStyle style, wxFontWeight weight,
                                         bool underline,
                                         const wxString &facename,
                                         wxFontEncoding encoding) {
  // from wx source code
  // In all ports but wxOSX, the effective family of a font created using
  // wxFONTFAMILY_DEFAULT is wxFONTFAMILY_SWISS so this is what we need to
  // use for comparison.
  //
  // In wxOSX the original wxFONTFAMILY_DEFAULT seems to be kept and it uses
  // a different font than wxFONTFAMILY_SWISS anyhow so we just preserve it.
#ifndef __WXOSX__
  if (family == wxFONTFAMILY_DEFAULT) family = wxFONTFAMILY_SWISS;
#endif  // !__WXOSX__

  wxFont *font;
  for (size_t i = 0; i < m_fontVector.size(); i++) {
    font_cache_record record = m_fontVector[i];
    if (isCached(record, pointSize, family, style, weight, underline, facename,
                 encoding))
      return record.font;
  }

  // font not found, create the new one
  // Support scaled HDPI displays automatically

  font = NULL;
  wxFont fontTmp(OCPN_GetDisplayContentScaleFactor() * pointSize, family, style,
                 weight, underline, facename, encoding);
  if (fontTmp.IsOk()) {
    font = new wxFont(fontTmp);
    font_cache_record record;
    record.font = font;
    record.pointsize_req = pointSize;
    record.style_req = style;
    record.weight_req = weight;
    record.underline_req = underline;
    m_fontVector.push_back(record);
  }

  return font;
}

void OCPNwxFontList::FreeAll() {
  wxFont *font;
  for (size_t i = 0; i < m_fontVector.size(); i++) {
    font_cache_record record = m_fontVector[i];
    font = record.font;
    delete font;
  }
  m_fontVector.clear();
}

static wxString FontCandidates[] = {_("AISTargetAlert"),
                                    _("AISTargetQuery"),
                                    _("StatusBar"),
                                    _("AIS Target Name"),
                                    _("ObjectQuery"),
                                    _("RouteLegInfoRollover"),
                                    _("ExtendedTideIcon"),
                                    _("CurrentValue"),
                                    _("Console Legend"),
                                    _("Console Value"),
                                    _("AISRollover"),
                                    _("TideCurrentGraphRollover"),
                                    _("Marks"),
                                    _("ChartTexts"),
                                    _("ToolTips"),
                                    _("Dialog"),
                                    _("Menu"),
                                    _("GridText"),
                                    "END_OF_LIST"};

void FontMgr::ScrubList() {
  wxString now_locale = g_locale;
  wxArrayString string_array;

  //  Build the composite candidate array
  wxArrayString candidateArray;
  unsigned int i = 0;

  // The fixed, static list
  while (true) {
    wxString candidate = FontCandidates[i];
    if (candidate == "END_OF_LIST") {
      break;
    }

    candidateArray.Add(candidate);
    i++;
  }

  //  The Aux Key array
  for (unsigned int i = 0; i < m_AuxKeyArray.GetCount(); i++) {
    candidateArray.Add(m_AuxKeyArray[i]);
  }

  for (unsigned int i = 0; i < candidateArray.GetCount(); i++) {
    wxString candidate = candidateArray[i];

    //  For each font identifier string in the FontCandidate array...

    //  In the current locale, walk the loaded list looking for a translation
    //  that is correct, according to the currently load .mo file.
    //  If found, add to a temporary array

    wxString trans = wxGetTranslation(candidate);

    for (auto node = m_fontlist->begin(); node != m_fontlist->end(); node++) {
      MyFontDesc *pmfd = *node;
      wxString tlocale = pmfd->m_configstring.BeforeFirst('-');
      if (tlocale == now_locale) {
        if (trans == pmfd->m_dialogstring) {
          string_array.Add(pmfd->m_dialogstring);
        }
      }
    }
  }

  // now we have an array of correct translations
  // Walk the loaded list again.
  // If a list item's translation is not in the "good" array, mark it for
  // removal

  for (auto node = m_fontlist->begin(); node != m_fontlist->end(); node++) {
    MyFontDesc *pmfd = *node;
    wxString tlocale = pmfd->m_configstring.BeforeFirst('-');
    if (tlocale == now_locale) {
      bool bfound = false;
      for (unsigned int i = 0; i < string_array.GetCount(); i++) {
        if (string_array[i] == pmfd->m_dialogstring) {
          bfound = true;
          break;
        }
      }
      if (!bfound) {  // mark for removal
        pmfd->m_dialogstring = "";
        pmfd->m_configstring = "";
      }
    }
  }

  //  Remove the marked list items
  auto node = m_fontlist->begin();
  while (node != m_fontlist->end()) {
    MyFontDesc *pmfd = *node;
    if (pmfd->m_dialogstring == "") {
      auto found = std::find(m_fontlist->begin(), m_fontlist->end(), pmfd);
      if (found != m_fontlist->end()) m_fontlist->erase(found);
      node = m_fontlist->begin();
    } else {
      ++node;
    }
  }

  //  And finally, for good measure, make sure that everything in the candidate
  //  array has a valid entry in the list
  i = 0;
  while (true) {
    wxString candidate = FontCandidates[i];
    if (candidate == "END_OF_LIST") {
      break;
    }

    GetFont(wxGetTranslation(candidate));
    i++;
  }
}

bool FontMgr::AddAuxKey(wxString key) {
  for (unsigned int i = 0; i < m_AuxKeyArray.GetCount(); i++) {
    if (m_AuxKeyArray[i] == key) return false;
  }
  m_AuxKeyArray.Add(key);
  return true;
}

bool FontMgr::ResetFontToDefault(const wxString &TextElement) {
  // Create default font with system settings
  int size = g_default_font_size ? g_default_font_size : GetSystemFontSize();
  wxString face = g_default_font_facename.Length() ? g_default_font_facename
                                                   : GetSystemFontFaceName();
  wxString native = GetSimpleNativeFont(size, face);
  wxFont *defaultFont = wxFont::New(native);

  // Find and update the font descriptor
  MyFontDesc *desc = GetFontDesc(TextElement);
  if (desc) {
    // Update font properties
    desc->m_font = defaultFont;
    desc->m_nativeInfo = native;
    desc->m_color = GetDefaultFontColor(TextElement);
    desc->m_is_default = true;
    return true;
  }

  return false;
}
