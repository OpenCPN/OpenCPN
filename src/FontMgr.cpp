/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
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

#include <locale>

#include <wx/gdicmn.h>
#include <wx/tokenzr.h>

#include "FontMgr.h"
#include "OCPNPlatform.h"

class  OCPNwxFontList: public wxGDIObjListBase
{
public:
    wxFont *FindOrCreateFont(int pointSize,
                             wxFontFamily family,
                             wxFontStyle style,
                             wxFontWeight weight,
                             bool underline = false,
                             const wxString& face = wxEmptyString,
                             wxFontEncoding encoding = wxFONTENCODING_DEFAULT);
    void FreeAll( void );
    
private:
    bool isSame(wxFont *font, int pointSize, wxFontFamily family,
                wxFontStyle style,
                wxFontWeight weight,
                bool underline,
                const wxString& facename,
                wxFontEncoding encoding);
};


extern wxString g_locale;
extern OCPNPlatform     *g_Platform;

wxString s_locale;
int g_default_font_size;

FontMgr * FontMgr::instance = NULL;

FontMgr & FontMgr::Get()
{
    if (!instance)
        instance = new FontMgr;
    return *instance;
}

void FontMgr::Shutdown()
{
    if (instance)
    {
        delete instance;
        instance = NULL;
    }
}

FontMgr::FontMgr()
    : m_wxFontCache(NULL)
    ,m_fontlist(NULL)
    , pDefFont(NULL)
{
    //    Create the list of fonts
    m_fontlist = new FontList;
    m_fontlist->DeleteContents( true );

    s_locale = g_locale;
    
    //    Get a nice generic font as default
    pDefFont = FindOrCreateFont( 12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, FALSE,
            wxString( _T ( "" ) ), wxFONTENCODING_SYSTEM );

}

FontMgr::~FontMgr()
{
    m_fontlist->Clear();
    delete m_fontlist;
    
    delete m_wxFontCache;
}

wxColour FontMgr::GetFontColor( const wxString &TextElement ) const
{
    //    Look thru the font list for a match
    MyFontDesc *pmfd;
    wxNode *node = (wxNode *) ( m_fontlist->GetFirst() );
    while( node ) {
        pmfd = (MyFontDesc *) node->GetData();
        if( pmfd->m_dialogstring == TextElement ) {
            if(pmfd->m_configstring.BeforeFirst('-') == s_locale)
                return pmfd->m_color;
        }
        node = node->GetNext();
    }

    return wxColour( 0, 0, 0 );
}

wxString FontMgr::GetFontConfigKey( const wxString &description )
{
    // Create the configstring by combining the locale with
    // a hash of the font description. Hash is used because the i18n
    // description can contain characters that mess up the config file.

    wxString configkey;
    configkey = s_locale;
    configkey.Append( _T("-") );

    using namespace std;
    locale loc;
    const collate<char>& coll = use_facet<collate<char> >( loc );
//    char cFontDesc[101];
//    wcstombs( cFontDesc, description.c_str(), 100 );
//    cFontDesc[100] = 0;

    wxCharBuffer abuf = description.ToUTF8();
    
    int fdLen = strlen( abuf );

    configkey.Append(
            wxString::Format( _T("%08lx"),
                              coll.hash( abuf.data(), abuf.data() + fdLen ) ) );
    return configkey;
}

wxFont *FontMgr::GetFont( const wxString &TextElement, int user_default_size )
{
    //    Look thru the font list for a match
    MyFontDesc *pmfd;
    wxNode *node = (wxNode *) ( m_fontlist->GetFirst() );
    while( node ) {
        pmfd = (MyFontDesc *) node->GetData();
        if( pmfd->m_dialogstring == TextElement ) {
            if(pmfd->m_configstring.BeforeFirst('-') == s_locale)
                return pmfd->m_font;
        }
        node = node->GetNext();
    }

    // Found no font, so create a nice one and add to the list
    wxString configkey = GetFontConfigKey( TextElement );

    //    Now create a benign, always present native font
    //    with optional user requested default size
    
    //    Get the system default font.
    wxFont sys_font = *wxNORMAL_FONT;
    int sys_font_size = sys_font.GetPointSize();
    wxString FaceName = sys_font.GetFaceName();
    
#ifdef __OCPN__ANDROID__
    sys_font_size = 18;
    FaceName = _T("Roboto");
#endif    
    

    int new_size;
    if( 0 == user_default_size )
        new_size = sys_font_size;
    else
        new_size = user_default_size;

    wxString nativefont = GetSimpleNativeFont( new_size, FaceName );
    wxFont *nf = wxFont::New( nativefont );
    
    wxColor color( *wxBLACK );

    MyFontDesc *pnewfd = new MyFontDesc( TextElement, configkey, nf, color );
    m_fontlist->Append( pnewfd );

    return pnewfd->m_font;

}

wxString FontMgr::GetSimpleNativeFont( int size, wxString face )
{
    //    Now create a benign, always present native string
    wxString nativefont;

    // this should work for all platforms
    nativefont = wxFont(size, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, face)
    .GetNativeFontInfoDesc();
    
#if 0
    //    For those platforms which have no native font description string format
    nativefont.Printf( _T ( "%d;%d;%d;%d;%d;%d;%s;%d" ),
            0,                                 // version
            size, wxFONTFAMILY_DEFAULT, (int) wxFONTSTYLE_NORMAL, (int) wxFONTWEIGHT_NORMAL, false,
            "", (int) wxFONTENCODING_DEFAULT );

//    If we know of a detailed description string format, use it.
#ifdef __WXGTK__
    nativefont.Printf ( _T ( "Fixed %2d" ), size );
#endif

#ifdef __WXX11__
    nativefont = _T ( "0;-*-fixed-*-*-*-*-*-120-*-*-*-*-iso8859-1" );
#endif

#ifdef __WXMSW__
//      nativefont = _T ( "0;-11;0;0;0;400;0;0;0;0;0;0;0;0;MS Sans Serif" );

    wxFont sys_font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    sys_font.SetPointSize( size + 1 );

    int size_px = sys_font.GetPixelSize().GetHeight();

    nativefont.Printf( _T("%d;%ld;%ld;%ld;%ld;%ld;%d;%d;%d;%d;%d;%d;%d;%d;"), 0, // version, in case we want to change the format later
            size_px,             //lf.lfHeight
            0,                   //lf.lfWidth,
            0,                   //lf.lfEscapement,
            0,                   //lf.lfOrientation,
            400,                 //lf.lfWeight,
            0,                   //lf.lfItalic,
            0,                   //lf.lfUnderline,
            0,                   //lf.lfStrikeOut,
            0,                   //lf.lfCharSet,
            0,                   //lf.lfOutPrecision,
            0,                   //lf.lfClipPrecision,
            0,                   //lf.lfQuality,
            0 );                    //lf.lfPitchAndFamily,

    nativefont.Append( sys_font.GetFaceName() );

#endif
#endif
    return nativefont;
}

bool FontMgr::SetFont(const wxString &TextElement, wxFont *pFont, wxColour color)
{
    //    Look thru the font list for a match
    MyFontDesc *pmfd;
    wxNode *node = (wxNode *) ( m_fontlist->GetFirst() );
    while( node ) {
        pmfd = (MyFontDesc *) node->GetData();
        if( pmfd->m_dialogstring == TextElement ) {
            if(pmfd->m_configstring.BeforeFirst('-') == g_locale) {
                
            // Todo Think about this
            //

//      Cannot delete the present font, since it may be in use elsewhere
//      This WILL leak....but only on font changes

//              delete pmfd->m_font;                            // purge any old value

                pmfd->m_font = pFont;
                pmfd->m_nativeInfo = pFont->GetNativeFontInfoDesc();
                pmfd->m_color = color;

                return true;
            }
        }
        node = node->GetNext();
    }

    return false;
}

int FontMgr::GetNumFonts( void ) const
{
    return m_fontlist->GetCount();
}

const wxString & FontMgr::GetConfigString( int i ) const
{
    MyFontDesc * pfd = (MyFontDesc *) ( m_fontlist->Item( i )->GetData() );
    return pfd->m_configstring;
}

const wxString & FontMgr::GetDialogString( int i ) const
{
    MyFontDesc *pfd = (MyFontDesc *) ( m_fontlist->Item( i )->GetData() );
    return pfd->m_dialogstring;
}

const wxString & FontMgr::GetNativeDesc( int i ) const
{
    MyFontDesc *pfd = (MyFontDesc *) ( m_fontlist->Item( i )->GetData() );
    return pfd->m_nativeInfo;
}

wxString FontMgr::GetFullConfigDesc( int i ) const
{
    MyFontDesc *pfd = (MyFontDesc *) ( m_fontlist->Item( i )->GetData() );
    wxString ret = pfd->m_dialogstring;
    ret.Append( _T ( ":" ) );
    ret.Append( pfd->m_nativeInfo );
    ret.Append( _T ( ":" ) );

    wxString cols( _T("rgb(0,0,0)") );
    if( pfd->m_color.IsOk() ) cols = pfd->m_color.GetAsString( wxC2S_CSS_SYNTAX );

    ret.Append( cols );
    return ret;
}

void FontMgr::LoadFontNative( wxString *pConfigString, wxString *pNativeDesc )
{
    //    Parse the descriptor string

    wxStringTokenizer tk( *pNativeDesc, _T ( ":" ) );
    wxString dialogstring = tk.GetNextToken();
    wxString nativefont = tk.GetNextToken();

    wxString c = tk.GetNextToken();
    wxColour color( c );            // from string description

    //    Search for a match in the list
    MyFontDesc *pmfd;
    wxNode *node = (wxNode *) ( m_fontlist->GetFirst() );

    while( node ) {
        pmfd = (MyFontDesc *) node->GetData();
        if( pmfd->m_configstring == *pConfigString ) {
            if(pmfd->m_configstring.BeforeFirst('-') == g_locale) {
                pmfd->m_nativeInfo = nativefont;
                wxFont *nf = pmfd->m_font->New( pmfd->m_nativeInfo );
                pmfd->m_font = nf;
                break;
            }
        }
        node = node->GetNext();
    }

    //    Create and add the font to the list
    if( !node ) {

        wxFont *nf0 = new wxFont();
        
#ifdef __OCPN__ANDROID__
        wxFont *nf = new wxFont( nativefont );
#else
        wxFont *nf = nf0->New( nativefont );
#endif
        
        double font_size = nf->GetPointSize();
        wxString s = nf->GetNativeFontInfoDesc();

        //    Scrub the native font string for bad unicode conversion
#ifdef __WXMSW__
        wxString face = nf->GetFaceName();
        const wxChar *t = face.c_str();
        if( *t > 255 ) {
            delete nf;
            wxString substitute_native = GetSimpleNativeFont( 12, _T("") );
            nf = nf0->New( substitute_native );
        }
#endif
        delete nf0;

        MyFontDesc *pnewfd = new MyFontDesc( dialogstring, *pConfigString, nf, color );
        m_fontlist->Append( pnewfd );

    }
}

wxFont* FontMgr::FindOrCreateFont( int point_size, wxFontFamily family, 
                    wxFontStyle style, wxFontWeight weight, bool underline,
                    const wxString &facename,
                    wxFontEncoding encoding)
{
    if (m_wxFontCache == 0)
        m_wxFontCache = new OCPNwxFontList;
    return m_wxFontCache->FindOrCreateFont( point_size, family, style, weight,
        underline, facename, encoding);
}        

bool OCPNwxFontList::isSame(wxFont *font, int pointSize, wxFontFamily family,
                             wxFontStyle style,
                             wxFontWeight weight,
                             bool underline,
                             const wxString& facename,
                             wxFontEncoding encoding)
{
    if (
         font->GetPointSize () == pointSize &&
         font->GetStyle () == style &&
         font->GetWeight () == weight &&
         font->GetUnderlined () == underline )
    {
        bool same;

        // empty facename matches anything at all: this is bad because
        // depending on which fonts are already created, we might get back
        // a different font if we create it with empty facename, but it is
        // still better than never matching anything in the cache at all
        // in this case
        if ( !facename.empty() )
        {
            const wxString& fontFace = font->GetFaceName();

            // empty facename matches everything
            same = !fontFace || fontFace == facename;
        }
        else 
        {
            same = font->GetFamily() == family;
        }
        if ( same && (encoding != wxFONTENCODING_DEFAULT) )
        {
            // have to match the encoding too
            same = font->GetEncoding() == encoding;
        }
        return same;
    }
    return false;
}

wxFont *OCPNwxFontList::FindOrCreateFont(int pointSize,
                                     wxFontFamily family,
                                     wxFontStyle style,
                                     wxFontWeight weight,
                                     bool underline,
                                     const wxString& facename,
                                     wxFontEncoding encoding)
{
    // from wx source code
    // In all ports but wxOSX, the effective family of a font created using
    // wxFONTFAMILY_DEFAULT is wxFONTFAMILY_SWISS so this is what we need to
    // use for comparison.
    //
    // In wxOSX the original wxFONTFAMILY_DEFAULT seems to be kept and it uses
    // a different font than wxFONTFAMILY_SWISS anyhow so we just preserve it.
#ifndef __WXOSX__
    if ( family == wxFONTFAMILY_DEFAULT )
        family = wxFONTFAMILY_SWISS;
#endif // !__WXOSX__

    wxFont *font;
    wxList::compatibility_iterator node;
    for (node = list.GetFirst(); node; node = node->GetNext())
    {
        font = (wxFont *)node->GetData();
        if (isSame(font, pointSize, family, style, weight, underline, facename, encoding))
            return font;
    }

    // font not found, create the new one
    font = NULL;
    wxFont fontTmp(pointSize, family, style, weight, underline, facename, encoding);
    if (fontTmp.IsOk())
    {
        font = new wxFont(fontTmp);
        list.Append(font);
        
        // double check the font really roundtrip
        //  Removed after verification.
        //wxASSERT(isSame(font, pointSize, family, style, weight, underline, facename, encoding));
    }

    return font;
}

void OCPNwxFontList::FreeAll( void )
{
    wxFont *font;
    wxList::compatibility_iterator node;
    for (node = list.GetFirst(); node; node = node->GetNext())
    {
        font = (wxFont *)node->GetData();
        delete font;
    }
}

wxString FontCandidates[] = {
    _T("AISTargetAlert"), 
    _T("AISTargetQuery"),
    _T("StatusBar"),
    _T("AIS Target Name" ),
    _T("ObjectQuery"),
    _T("RouteLegInfoRollover"),
    _T("ExtendedTideIcon"),
    _T("CurrentValue"),
    _T("Console Legend"),
    _T("Console Value"),
    _T("AISRollover"),
    _T("TideCurrentGraphRollover"),
    _T("Marks"),
    _T("ChartTexts"),
    _T("ToolTips"),
    _T("Dialog"),
    _T("Menu"),
    _T("END_OF_LIST")
};


void FontMgr::ScrubList( )
{
    wxString now_locale = g_locale;
    wxArrayString string_array;
    
    //  Build the composite candidate array
    wxArrayString candidateArray;
    bool done = false;
    unsigned int i = 0;
    
    // The fixed, static list
    while( ! done ){
        wxString candidate = FontCandidates[i];
        if(candidate == _T("END_OF_LIST") ) {
            done = true;
            break;
        }
        
        candidateArray.Add(candidate);
        i++;
    }
        
    //  The Aux Key array    
    for(unsigned int i=0 ; i <  m_AuxKeyArray.GetCount() ; i++){
        candidateArray.Add(m_AuxKeyArray[i]);
    }
    
    
    for(unsigned int i = 0; i < candidateArray.GetCount() ; i++ ){
        wxString candidate = candidateArray[i];
        
        //  For each font identifier string in the FontCandidate array...
        
        //  In the current locale, walk the loaded list looking for a translation
        //  that is correct, according to the currently load .mo file.
        //  If found, add to a temporary array
        
        wxString trans = wxGetTranslation(candidate);
        
        MyFontDesc *pmfd;
        wxNode *node = (wxNode *) ( m_fontlist->GetFirst() );
        while( node ) {
            pmfd = (MyFontDesc *) node->GetData();
            wxString tlocale = pmfd->m_configstring.BeforeFirst('-');
            if( tlocale == now_locale) {
                if(trans == pmfd->m_dialogstring){
                    string_array.Add(pmfd->m_dialogstring);
                }
            }
 
            node = node->GetNext();
        }
    }        

    // now we have an array of correct translations    
    // Walk the loaded list again.
    // If a list item's translation is not in the "good" array, mark it for removal
    
    MyFontDesc *pmfd;
    wxNode *node = (wxNode *) ( m_fontlist->GetFirst() );
    while( node ) {
        pmfd = (MyFontDesc *) node->GetData();
        wxString tlocale = pmfd->m_configstring.BeforeFirst('-');
        if( tlocale == now_locale) {
            bool bfound = false;
            for(unsigned int i=0 ; i < string_array.GetCount() ; i++){
                if( string_array[i] == pmfd->m_dialogstring){
                    bfound = true;
                    break;
                }
            }
            if(!bfound){        // mark for removal
                pmfd->m_dialogstring = _T("");
                pmfd->m_configstring = _T("");
            }
        }
        
        node = node->GetNext();
    }
    
    //  Remove the marked list items
    node = (wxNode *) ( m_fontlist->GetFirst() );
    while( node ) {
        pmfd = (MyFontDesc *) node->GetData();
        if( pmfd->m_dialogstring == _T("") ) {
            bool bd = m_fontlist->DeleteObject(pmfd);
            if(bd)
                node = (wxNode *) ( m_fontlist->GetFirst() );
        }
        else
            node = node->GetNext();
        
    }
 
    //  And finally, for good measure, make sure that everything in the candidate array has a valid entry in the list
    done = false;
    i = 0;
    while( ! done ){
        wxString candidate = FontCandidates[i];
        if(candidate == _T("END_OF_LIST") ) {
            done = true;
            break;
        }

        GetFont( wxGetTranslation(candidate), g_default_font_size );
     
        i++;
    }
 
     
}

bool FontMgr::AddAuxKey( wxString key )
{
    for(unsigned int i=0 ; i <  m_AuxKeyArray.GetCount() ; i++){
        if(m_AuxKeyArray[i] == key)
            return false;
    }
    m_AuxKeyArray.Add(key);
    return true;
}

