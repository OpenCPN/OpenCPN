/***************************************************************************
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#ifndef __FONTMGR_H__
#define __FONTMGR_H__

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include "FontDesc.h"

class OCPNwxFontList;

/**
 * Manages the font list.
 *
 * Singleton.
 */
class FontMgr {
public:
  static FontMgr &Get();

  wxFont *GetFontLegacy(const wxString &TextElement, int user_default_size);

  void SetLocale(wxString &newLocale);
  /**
   * Gets a font object for a UI element.
   *
   * Each UI element (like "AISTargetAlert", "StatusBar") has a font
   * configuration. Returns existing font if found, otherwise creates a new one
   * using system defaults or user preferences.
   *
   * Supported TextElements: AISTargetAlert, AISTargetQuery, StatusBar, AIS
   * Target Name, ObjectQuery, RouteLegInfoRollover, ExtendedTideIcon,
   * CurrentValue, Console Legend, Console Value, AISRollover,
   * TideCurrentGraphRollover, Marks, ChartTexts, ToolTips, Dialog, Menu,
   * GridText
   *
   * @param TextElement UI element identifier (e.g., "AISTargetAlert",
   * "StatusBar")
   * @param requested_font_size Requested font size in points, 0 to use
   * system/user default
   * @deprecated requested_font_size This parameter is deprecated as it
   * overrides user font customization settings. Hard-coded font sizes make it
   * impossible for OpenCPN to honor user font preferences. Always use 0 to
   * respect user's font customization choices.
   * @return Pointer to the font to use
   */
  wxFont *GetFont(const wxString &TextElement, int requested_font_size = 0);
  /**
   * Gets the text color for a UI element.
   *
   * Looks up stored color for the element. Returns black if not found.
   *
   * @param TextElement UI element identifier
   * @return Text color to use, defaults to black if not found
   * @see [GetFont]
   */
  wxColour GetFontColor(const wxString &TextElement) const;
  /**
   * Gets the default text color for a UI element.
   *
   * Returns predefined defaults for certain elements like Console Legend
   * (green), Marks (black). Falls back to system window text color if no
   * special default. On macOS, always returns black to support light/dark mode.
   *
   * @param TextElement UI element identifier
   * @return Default text color for this element
   * @see [GetFont]
   */
  wxColour GetDefaultFontColor(const wxString &TextElement);
  /**
   * Sets the text color for a UI element.
   *
   * Updates stored color for the element in the current locale.
   *
   * @param TextElement UI element identifier
   * @param color New text color to set
   * @return True if element was found and color was set, false if not found
   * @see [GetFont]
   */
  bool SetFontColor(const wxString &TextElement, const wxColour color) const;

  /**
   * Gets the total number of font configurations currently loaded.
   * Each configuration defines the font and color for a specific UI element
   * like "AISTargetAlert" in a specific locale.
   * @return Number of font configurations
   * @see [m_fontlist]
   */
  int GetNumFonts(void) const;
  /**
   * Gets the locale-specific configuration key for a font at index i.
   *
   * Used to store and retrieve font configurations by combining the current
   * locale with a hash of the font's text element name (e.g. "AISTargetAlert",
   * "StatusBar").
   *
   * For example, given a text element "Dialog" in French locale "fr_FR", the
   * returned key might be "fr_FR-a7b9c123" where a7b9c123 is the hash of
   * "Dialog".
   *
   * @param i Font index between 0 and GetNumFonts()-1
   * @return Configuration string in format "locale-hash" used for storage
   * @see GetFontConfigKey() for the hash generation
   * @see s_locale for current locale tracking
   */
  const wxString &GetConfigString(int i) const;
  /**
   * Gets the UI element identifier string for the font at index i.
   *
   * Returns the human-readable identifier like "AISTargetAlert" or "StatusBar"
   * used to reference this font in the UI.
   *
   * @param i Font index between 0 and GetNumFonts()-1
   * @return UI element name for this font configuration
   */
  const wxString &GetDialogString(int i) const;

  /**
   * Gets the list of unique dialog strings.
   *
   * @param locale If provided, only returns strings for this locale.
   * @return Vector of unique dialog strings
   */
  wxArrayString GetDialogStrings(const wxString &locale = wxEmptyString) const;

  /**
   * Gets the native font descriptor string for the font at index i.
   *
   * Returns platform-specific complete font specification string that can be
   * used to recreate the exact font. Contains size, family, style, weight and
   * other attributes. Format varies by platform.
   *
   * @param i Font index between 0 and GetNumFonts()-1
   * @return Native font description string
   */
  const wxString &GetNativeDesc(int i) const;
  /**
   * Gets description of font at index i.
   * @param i Font index between 0 and GetNumFonts()-1
   * @return String in format "elementname:nativedesc:color"
   */
  wxString GetFullConfigDesc(int i) const;
  /**
   * Creates configuration key from UI element name by combining locale with
   * hash.
   *
   * @param description UI element name to hash (e.g. "AISTargetAlert")
   * @return String in format "locale-hash" for storage.
   */
  static wxString GetFontConfigKey(const wxString &description);

  /**
   * Gets array of plugin-defined font configuration keys.
   *
   * Plugins can add their own UI elements requiring font configuration beyond
   * the standard elements defined in FontCandidates[] (like "AISTargetAlert",
   * "StatusBar"). These plugin-specific keys allow plugins to participate in
   * OpenCPN's font management system.
   *
   * @return Reference to array containing plugin-defined font element
   * identifiers
   */
  wxArrayString &GetAuxKeyArray() { return m_AuxKeyArray; }
  /**
   * Adds new plugin-defined font configuration key.
   *
   * Allows plugins to register their UI elements for font configuration.
   * Each element must have a unique identifier.
   *
   * @param key New plugin UI element identifier to add
   * @return True if key was added, false if already exists
   */
  bool AddAuxKey(wxString key);

  /**
   * Loads font settings from a string descriptor.
   *
   * @param[in] pConfigString Lookup key in "locale-hash" format to find font
   * config
   * @param[in] pNativeDesc Font settings to load, format:
   * "TextElement:NativeFontString:rgb(r,g,b)" where:
   *                        - TextElement: UI element identifier (e.g.
   * "AISTargetAlert")
   *                        - NativeFontString: Platform-specific font
   * description
   *                        - rgb(r,g,b): Text color in CSS RGB
   *
   * Updates the font configuration in m_fontlist matching pConfigString with
   * the settings from pNativeDesc. Creates new entry if not found.
   */
  void LoadFontNative(wxString *pConfigString, wxString *pNativeDesc);
  /**
   * Sets the default font properties for a UI element.
   *
   * Updates the font and color of the default font entry for the given
   * TextElement. Creates a copy of the provided font to avoid memory management
   * issues. Only affects the default font entry - specific size variants are
   * unchanged.
   *
   * @param TextElement UI element identifier (e.g., "AISTargetAlert")
   * @param pFont New font to use
   * @param color New text color
   * @return true if default font entry was found and updated, false if not
   * found
   */
  bool SetFont(const wxString &TextElement, wxFont *pFont, wxColour color);
  /**
   * Cleans up stale font entries after a locale change.
   *
   * The function preserves customized font settings while removing
   * outdated entries that might reference old translations.
   */
  void ScrubList();
  /**
   * Finds font descriptor by its configuration key.
   *
   * @param pConfigString Key in "locale-hash" format
   * @return Matching font descriptor or NULL if not found
   */
  MyFontDesc *FindFontByConfigString(wxString pConfigString);

  /**
   * Creates or finds a matching font in the font cache.
   *
   * If a font with the specified characteristics exists in the cache, returns
   * that font. Otherwise, creates a new font, adds it to the cache and returns
   * it.
   *
   * @param point_size Font size in points.
   * @param family Font family like wxFONTFAMILY_SWISS.
   * @param style Font style like wxFONTSTYLE_NORMAL.
   * @param weight Font weight like wxFONTWEIGHT_NORMAL.
   * @param underline True to underline the font.
   * @param facename Name of the font face, empty for default.
   * @param encoding Font encoding, wxFONTENCODING_DEFAULT for default.
   * @return Pointer to the cached wxFont object.
   */
  wxFont *FindOrCreateFont(int point_size, wxFontFamily family,
                           wxFontStyle style, wxFontWeight weight,
                           bool underline = false,
                           const wxString &facename = wxEmptyString,
                           wxFontEncoding encoding = wxFONTENCODING_DEFAULT);
  // For wxWidgets 2.8 compatability
  wxFont *FindOrCreateFont(int pointSize, int family, int style, int weight,
                           bool underline = false,
                           const wxString &face = wxEmptyString,
                           wxFontEncoding encoding = wxFONTENCODING_DEFAULT) {
    return FindOrCreateFont(pointSize, (wxFontFamily)family, (wxFontStyle)style,
                            (wxFontWeight)weight, underline, face, encoding);
  }

  /**
   * Resets the font configuration for a UI element back to system defaults.
   *
   * This function:
   * 1. Creates a font with system default properties.
   * 2. Updates the font descriptor in the font list to use this default font.
   * 3. Resets the text color to the default for this element.
   *
   * @param TextElement The UI element identifier (e.g. "AISTargetAlert").
   * @return true if the font was reset successfully, false if element not
   * found.
   */
  bool ResetFontToDefault(const wxString &TextElement);

  static void Shutdown();

private:  // private for singleton
  FontMgr();
  ~FontMgr();
  FontMgr(const FontMgr &) {}
  FontMgr &operator=(const FontMgr &) { return *this; }

  /**
   * Creates a standard native font description string.
   *
   * This function generates a platform-independent font description string that
   * is compatible with wxFont's native font description format. The resulting
   * string can be used to create a new wxFont using wxFont::New().
   *
   * The font is created with default properties:
   * - Family: wxFONTFAMILY_DEFAULT
   * - Style: wxFONTSTYLE_NORMAL
   * - Weight: wxFONTWEIGHT_NORMAL
   * - No underline
   *
   * @param size The point size for the font
   * @param face The face name to use. If empty, system default face will be
   * used
   * @return Native font description string that can be used with wxFont::New()
   */
  wxString GetSimpleNativeFont(int size, wxString face);

  /**
   * Determines if a font descriptor represents the default font entry.
   *
   * A font entry is considered default if its size matches either:
   * - The user-specified default size (g_default_font_size) if set, or
   * - The system default font size if no user default is specified
   *
   * This is used to distinguish between default font entries and
   * entries with specific requested sizes for the same TextElement.
   *
   * @param font_desc Pointer to the font descriptor to check
   * @return true if this is a default font entry, false otherwise
   */
  bool IsDefaultFontEntry(const MyFontDesc *font_desc) const;
  /**
   * Gets the system default font size.
   *
   * This method lazily initializes and caches the system font size from
   * wxNORMAL_FONT. Used to determine if a font entry represents the system
   * default when no user default size (g_default_font_size) is specified.
   *
   * @return The point size of the system default font
   */
  static int GetSystemFontSize();
  /**
   * Gets the system default font face name.
   *
   * This method lazily initializes and caches the system font face name from
   * wxNORMAL_FONT. Used when creating new fonts if no user default face name
   * (g_default_font_facename) is specified.
   *
   * @return The face name of the system default font
   */
  static wxString GetSystemFontFaceName();

  /**
   * Gets the default font descriptor for a given TextElement.
   *
   * First tries to find the default font entry (m_is_default=true) for the
   * current locale. If not found, falls back to the first matching entry
   * for backward compatibility.
   *
   * @param TextElement The UI element identifier (e.g., "AISTargetAlert")
   * @return Pointer to the font descriptor, or NULL if not found
   */
  MyFontDesc *GetFontDesc(const wxString &TextElement) const;

  static FontMgr *instance;

  /**
   * Cache mapping font specifications to wxFont objects.
   *
   * Cache keyed by combined font properties (size, family, style etc).
   * Reuses identical wxFont instances to reduce memory usage.
   * Internal cache for FindOrCreateFont(), does not handle text element
   * mapping.
   */
  OCPNwxFontList *m_wxFontCache;
  /**
   * High-level list mapping text elements to font configurations.
   *
   * Maps text element IDs (e.g. "AISTargetAlert") to complete font settings:
   * - Font (via font description string)
   * - Text color
   * - Locale binding
   * Used by GetFont() to provide configured fonts for UI elements.
   */
  FontList *m_fontlist;
  /** Default wxFont used when no specific font is found. System default in
   * normal weight */
  wxFont *pDefFont;
  /** Array of plugin-registered UI element identifiers that supplement standard
   * elements */
  wxArrayString m_AuxKeyArray;
};

#endif
