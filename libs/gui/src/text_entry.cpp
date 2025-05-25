#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "edit_button.h"
#include "text_entry.h"

/** Return window with given id (assumed to exist) casted to T*. */
template <typename T>
T* GetWindowById(int id) {
  return dynamic_cast<T*>(wxWindow::FindWindowById(id));
};

TextEntryPanel::TextEntryPanel(wxWindow* parent, const std::string& label,
                               std::string& text,
                               std::function<void()> on_update, int width)
    : wxPanel(parent, wxID_ANY),
      kEditBtnId(wxWindow::NewControlId()),
      kTextId(wxWindow::NewControlId()),
      kTextEntryId(wxWindow::NewControlId()),
      m_text(text),
      m_is_editing(false),
      m_on_update(std::move(on_update)) {
  auto flags = wxSizerFlags(0).Border();
  auto hbox = new wxBoxSizer(wxHORIZONTAL);
  auto size = wxSize(width * GetCharWidth(), GetCharHeight());
  hbox->Add(new wxStaticText(this, wxID_ANY, label), flags);
  auto entry =
      new wxTextCtrl(this, kTextEntryId, text, wxDefaultPosition, size);
  entry->SetValue(text);
  entry->Hide();
  hbox->Add(entry, flags);

  hbox->Add(1, 1, 1);  // Expanding space.
  std::string _label(text);
  if (_label.empty()) _label = _("(Empty)");
  hbox->Add(new wxStaticText(this, kTextId, _label), flags);
  auto button = new EditButton(this, kEditBtnId, [&] { OnEditClick(); });
  hbox->Add(button, flags);

  SetSizer(hbox);
  Fit();
  Show();
}

void TextEntryPanel::OnEditClick() {
  auto entry = GetWindowById<wxTextCtrl>(kTextEntryId);
  auto text = GetWindowById<wxStaticText>(kTextId);
  auto button = GetWindowById<EditButton>(kEditBtnId);
  if (m_is_editing) {
    m_text = entry->GetValue();
    m_on_update();
  } else {
    entry->SetValue(m_text);
  }
  text->SetLabel(entry->GetValue());
  m_is_editing = !m_is_editing;
  button->SetIcon(m_is_editing);
  text->Show(!m_is_editing);
  entry->Show(m_is_editing);
  auto parent = GetGrandParent() ? GetGrandParent() : GetParent();
  parent->Fit();
}
