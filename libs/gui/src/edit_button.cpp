#include <cassert>
#include <cstring>
#include <functional>

#include <wx/bitmap.h>
#include <wx/image.h>

#if wxCHECK_VERSION(3, 2, 0) || defined(ocpnUSE_wxBitmapBundle)
#include <wx/bmpbndl.h>
#else
#include <wx/sstream.h>
#include <wxSVG/svg.h>
#endif

#include "edit_button.h"
#include "svg_icons.h"

#ifdef ANDROID
wxBitmap loadAndroidSVG(const char* svg, unsigned int width,
                        unsigned int height);
#endif


EditButton::EditButton(wxWindow* parent, int id, std::function<void()> on_click)
    : wxButton(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize,
               wxBU_EXACTFIT | wxBU_BOTTOM),
      m_on_click(std::move(on_click)) {
  Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { m_on_click(); });
  SetIcon(false);
}

void EditButton::SetIcon(bool is_editing) {
  using namespace gui_icons;
  char buffer[2048];  // Needs to be larger than any svg icon...
  std::strcpy(buffer, is_editing ? kCheckmark : kEditPen);
#ifndef ANDROID
#if wxCHECK_VERSION(3, 2, 0) || defined(ocpnUSE_wxBitmapBundle)
  auto icon_size = wxSize(GetCharHeight(), GetCharHeight());
  auto bundle = wxBitmapBundle::FromSVG(buffer, icon_size);
  assert(bundle.IsOk() && "Cannot load svg icon");
  SetBitmap(bundle);
#else
  wxStringInputStream wis(buffer);
  wxSVGDocument svg_doc(wis);
  wxImage image = svg_doc.Render(GetCharHeight(), GetCharHeight());
  assert(wxBitmap(image).IsOk() && "Cannot load svg icon");
  SetBitmap(wxBitmap(image));
#endif
#else
  wxBitmap  bm = loadAndroidSVG(buffer, GetCharHeight(), GetCharHeight());
  SetBitmap(bm);
#endif
}
