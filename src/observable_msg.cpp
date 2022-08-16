#include "observable_msg.h"

/* ObservableMsg implementation */

void ObservableMsg::notify(std::shared_ptr<const NavMsg> msg) {
  ObservedVar::notify(std::dynamic_pointer_cast<const void>(msg));
}
