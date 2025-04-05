#include <functional>

#include <wx/button.h>
#include <wx/window.h>

/**
 *
 *  Two state button showing either an 'edit'(pen) or 'done' (checkmark)
 *  icon.
 */
class EditButton : public wxButton {
public:
  /**
   * Create a new instance.
   * @param parent Containing window.
   * @param id Window id, possibly wxID_ANY
   * @param on_click Callback invoked when user clicks on button.
   */
  EditButton(wxWindow* parent, int id, std::function<void()> on_click);

  /**
   * Set icon to either pen or checkmark.
   * @param is_editing If true set icon to checkmark, else set it ri pen.
   */
  void SetIcon(bool is_editing);

private:
  std::function<void()> m_on_click;
};
