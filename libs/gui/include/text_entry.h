#include <functional>
#include <string>

#include <wx/panel.h>
/**
 *
 * A label, a text entry and an EditButton allows for user text input
 *
 *              -------------------------------    -----
 *  Label:      |                             |    |   |
 *              -------------------------------    -----
 *
 *  The rightmost button is either a check mark or a pen icon, the entry
 *  is either a regular wxTextCtrl or a read-only text label depending
 *  on state. The label is fixed, possibly empty
 */
class TextEntryPanel : public wxPanel {
public:
  /** Create a new TextEntryPanel
   * @param parent Containing window.
   * @param label Left-most label, possibly empty.
   * @param text Initial value of the text entry.
   * @param on_update Callback invoked when user updates the text entry
   * @param width Width of entry in characters.
   */
  TextEntryPanel(wxWindow* parent, const std::string& label, std::string& text,
                 std::function<void()> on_update, int width = 40);

private:
  void OnEditClick();

  const int kEditBtnId;
  const int kTextId;
  const int kTextEntryId;

  std::string m_text;
  bool m_is_editing;
  std::function<void()> m_on_update;
};
