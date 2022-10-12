#ifndef _JSON_EVT_H__
#define _JSON_EVT_H__

#include <memory>
#include <string>

#include <wx/event.h>

#include "observable_evtvar.h"

/** Handle sending of wxJson messages to all plugins. */
class JsonEvent {
public:
static JsonEvent& getInstance() { static JsonEvent instance; return instance; }

void notify(const std::string& message, std::shared_ptr<void> msg_root) {
    event.notify(msg_root, message, 0);
}

ObservedVarListener GetListener(wxEvtHandler* handler, wxEventType ev) {
    return event.GetListener(handler, ev) ;
}

private:
EventVar event;
};

#endif

