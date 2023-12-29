/***************************************************************************
 *   Copyright (C) 2023 Alec Leamas                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#ifndef DBUS_HANDLER_H__
#define DBUS_HANDLER_H__

#include <functional>
#include <thread>

#include <gio/gio.h>

#include "instance_check.h"
#include "local_api.h"
#include "observable_evtvar.h"

/**
 * \file
 *
 * DBus interface header file.
 */

/** The name registered on the session bus. */
static const char* const kDbusName = "org.opencpn.OpenCPN";

/** Object representing a running desktop OpenCPN instance. */
static const char* const kDbusObject = "/org/opencpn/OpenCPN";

/** Dbus interface name, duplicated in introspection_xml. */
static const char* const kDbusInterface = "opencpn.desktop";

/** Interface description i. e., all methods we support + parameters */
static const gchar introspection_xml[] = R"""(
<node>
  <interface name='opencpn.desktop'>
    <annotation name='org.gtk.GDBus.Annotation' value='OnInterface'/>
    <annotation name='org.gtk.GDBus.Annotation' value='AlsoOnInterface'/>
    <method name='Ping'>
      <annotation name='org.gtk.GDBus.Annotation' value='OnMethod'/>
      <arg type='u' name='level' direction='out'/>
      <arg type='b' name='stat' direction='out'/>
    </method>
    <method name='Raise'>
      <annotation name='org.gtk.GDBus.Annotation' value='OnMethod'/>
      <!-- In the GUI case, raise application to top. -->
    </method>
    <method name='Open'>
      <annotation name='org.gtk.GDBus.Annotation' value='OnMethod'/>
      <arg type='s' name='level' direction='in'/>
      <arg type='b' name='stat' direction='out'/>
    </method>
    <method name='GetRestEndpoint'>
      <annotation name='org.gtk.GDBus.Annotation' value='OnMethod'/>
      <arg type='s' name='endpoint' direction='out'/>
    </method>
     <method name='Quit'>
      <annotation name='org.gtk.GDBus.Annotation' value='OnMethod'/>
    </method>
   </interface>
</node>
)""";

/** Handle incoming messages. */
static void HandleMethodCall(GDBusConnection* connection, const gchar* sender,
                             const gchar* object_path,
                             const gchar* interface_name,
                             const gchar* method_name, GVariant* parameters,
                             GDBusMethodInvocation* invocation,
                             gpointer user_data);

/**
 * Basic interface to low-level DBus library.
 *
 * The server is started by the first call to GetInstance(). The
 * server bith acts as a regular LocalApiServer and as an
 * InstanceCheckinstance check.
 */
class DbusServer : public LocalServerApi, public InstanceCheck {
public:

  static DbusServer& GetInstance();

  /** Clear current singleton instance and disconnect from session bus */
  static void Disconnect();

  DbusServer(DbusServer&) = delete;
  void operator=(const DbusServer&) = delete;

  virtual ~DbusServer();

  /**
   * Wait until ready. There is no signal for "ready", so the strategy is
   * to just wait for some time, allowing the underlying layers to perform
   * the dbus handshake. If we have claimed the bus name after this
   * time we are a server, else a client.
   *
   * TODO: Some clever algorithm to define processor speed which
   * could be used to determine the wait time. On my fast development
   * machine 200 us is enough in native Linux, but Flatpak requires
   * longer time.
   *
   * TODO: Some kind of lazy evalution to avoid delays?
   */
  void WaitUntilValid() override;

  /** Callback context. */
  GDBusNodeInfo* introspection_data;

  /**
   * Check if this instance is the only one.
   * @return true if ctor was able to claim kDbusName, else false.
   */
  bool IsMainInstance() { return m_is_main_instance; }

  void SetMainInstance(bool value) { m_is_main_instance = value; }

  DbusServer* UnitTestInstance() { return new DbusServer(); }

protected:
  DbusServer();

private:
  static DbusServer* s_instance;
  bool m_is_main_instance;
  unsigned m_owner_id;
};

#endif  // DBUS_HANDLER_H__
