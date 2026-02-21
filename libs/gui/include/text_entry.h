#include <functional>
#include <string>

#include <wx/panel.h>
/**
 * A label, a text entry and an EditButton allows for user text input
 * Depending in state it looks like
 * \image html ./text-entry-1.png
 * or
 * \image html ./text-entry-2.png
 *
 *  The leftmost label is fixed, possibly empty.
 *
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

  const int kEditBtnId;  ///< Use with FindWindowById to get EditButton button
  const int kTextId;    ///< Use with FindWindowById to get wxStaticText label
  const int kTextEntryId;  ///< Use with FindWindowById to get wxTextCtrl entry

private:
  void OnEditClick();

  std::string m_text;
  bool m_is_editing;
  std::function<void()> m_on_update;
};
