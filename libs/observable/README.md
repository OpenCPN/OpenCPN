\page observable wxWidgets Observable Framework

\section Introduction

This is a basic notify-listen framework on top of wxWidgets.  It requires
C++11 and wxWidgets 3.0+.

Library is thread-safe in the sense that Notify() can be invoked from
asynchronous worker threads. However, actual work performed by Listen()
must be done in the main thread.

The listening mechanism makes it possible for a listener to receive an event
on certain condtions.  The event is based on wxCommandEvent and can thus
optionally carry a string, a number and/or a pointer to the listening side.
It has been extended to also be able to carry a `shared_ptr<void>` which
basically makes it possible to transfer any shared_ptr from the notifying
side to the listeners.

There are three out of the box use cases:

- The EventVar mechanism depends on a variable which is visible on both
  the listener and notifying side. It is described in the
  _observable_evtvar.h_ header.

- The ConfigVar mechanism allows for listening to configuraion
  variable changes, and to notify about such changes. It is described
  in the _observable_confvar.h_ header

- The GlobalVar variable mechanism (sic!) allows for listening to global
  variable changes and to notify about such changes. It is described in
  the _observable_globvar.h_ header.

The framework is developed in the opencpn project [1].

\section Why

The basic event handling in wxWidgets has some limitations which this
framework addresses:

- The party generating an event needs to know which window to send it to.
  This is problematic for example when low level code needs to generate an
  event and send it to a GUI which it really should not be aware of.
  Framework handles this using the classic observable pattern.

- If a GUI starts listening to some event it means that other parties start
  sending events to a window id. This fails if the listening window goes out
  of scope. Framework handles this using listeners which does the right thing
  when they go out of scope.

- In a message context, listening is the same as receiving messages. In many
  cases this means that several parties needs to receive a single message,
  something best sorted out using `std::shared_ptr`. Framework supports this
  by using a new event type which can carry also a `std::shared_ptr`.

\section Extending

The framework can be extended to handle other types. The basic
condition is that anything which should use Listen() and Notify()
should implement the KeyProvider interface. This is just the ability
to return a unique, stable key.

Stated otherwise, the key used when invoking Listen() should be the
same as when doing Notify(). Examples includes:

* The event variable described above uses an auto generated random key,
  this works since both sides knows about the same event variable.

* The config variable uses a key derived from the configuration value name.
  This works since both sides knows the name (path) of the configuration
  variable.

In the OpenCPN project the framework is extended to handle messages. Each
message implements the KeyProvider interface which generates a key based on
the message "type". This makes it possible to listen for specific message
types as well as notifying when such a message is received.

\section Copyright

This work is copyright (c) Alec Leamas 2021-2022. It is distributed under
the GPL licence, version 2 or later. See the source files for details.


[1] https://github.com/opencpn/OpenCPN
