#ifndef _COMM_POINTER_MSG_H__
#define _COMM_POINTER_MSG_H__

#include <sstream>

#include "wx/event.h"

/**
 * Container  which carries a shared_ptr through wxWidgets event handling
 * which only allows simple objects which can be represented by a void*.
 */
template<typename T>
class PointerMsg {
public:
  PointerMsg(std::shared_ptr<T> p) : ptr(p){};

  /** Retrieve the pointer contained in an instance and delete instance. */
  static std::shared_ptr<T> get_pointer(wxCommandEvent ev) {
    auto msg = static_cast<PointerMsg<T>*>(ev.GetClientData());
    auto ptr = std::move(msg->ptr);
    delete msg;
    return ptr;
  };

private:
  PointerMsg() = delete;
  std::shared_ptr<T> operator=(std::shared_ptr<T>&) = delete;
  std::shared_ptr<T> ptr;
};


#endif  // _COMM_POINTER_MSG_H__
