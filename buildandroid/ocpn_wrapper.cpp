/////////////////////////////////////////////////////////////////////////////
// Name:        minimal.cpp
// Purpose:     Minimal wxWidgets sample
// Author:      David S Register
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id: minimal.cpp 65971 2010-11-01 12:30:38Z PMO $
// Copyright:   (c) Julian Smart
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif


#include "chart1.h"
//#include "myapp.h"

IMPLEMENT_APP_NO_MAIN(MyApp);
//IMPLEMENT_APP(MyApp);
//IMPLEMENT_WX_THEME_SUPPORT;

int main(int argc, char *argv[])
{
    wxEntryStart( argc, argv );
    wxTheApp->CallOnInit();
    wxTheApp->OnRun();

    return 0;
}
