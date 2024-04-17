#!/usr/bin/env python


import dbus
import dbus.service
import dbus.mainloop.glib

import sys
import threading
import time

from gi.repository import GObject as gobject
from gi.repository import GLib

def sleep_and_exit(main_loop):
    time.sleep(50/1000)
    main_loop.quit()

class TestException(dbus.DBusException):
    _dbus_error_name = 'org.opencpn.OpenCPN.TestException'


class TestService(dbus.service.Object):

    def __init__(self, bus, object_path, main_loop, max_commands):
        super().__init__(bus, object_path)
        self._main_loop = main_loop
        self._max_commands = max_commands

    def maybe_exit_later(self):
        if self._max_commands == 0:
            self._main_loop.quit()
        elif self._max_commands == 1: 
            t = threading.Thread(target=sleep_and_exit,
                                 args=(self._main_loop,))
            t.start()
        elif self._max_commands > 0: 
            self._max_commands -= 1

    @dbus.service.method("opencpn.desktop",
                         in_signature='', out_signature='')
    def Raise(self):
        self.maybe_exit_later()
        return None

    @dbus.service.method("opencpn.desktop",
                         in_signature='', out_signature='')
    def Quit(self):
        self.maybe_exit_later()
        return None

    @dbus.service.method("opencpn.desktop",
                         in_signature='s', out_signature='b')
    def Open(self, path):
        self.maybe_exit_later()
        return (True,)

    @dbus.service.method("opencpn.desktop",
                         in_signature='', out_signature='s')
    def GetRestEndpoint(self):
        self.maybe_exit_later()
        return ("0.0.0.0/1025")

    @dbus.service.method("opencpn.desktop",
                         in_signature='', out_signature='ub')
    def Ping(self):
        self.maybe_exit_later()
        return (21614, True)


if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    max_commands = -1 if len(sys.argv) < 2 else int(sys.argv[1])

    session_bus = dbus.SessionBus()
    name = dbus.service.BusName('org.opencpn.OpenCPN', session_bus)

    main_loop = GLib.MainLoop()
    obj = TestService(session_bus, '/org/opencpn/OpenCPN', main_loop,
                      max_commands)
    main_loop.run()
