/***************************************************************************
 *   Copyright (C) 2024  Alec Leamas                                       *
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
 **************************************************************************/

/**
 * \file linux_watch_daemon.h
 * Listen for Linux DBus  events like suspend/resume and new devices to notify
 * SystemEvents
 */

#include "model/usb_watch_daemon.h"

// Definitions for "New device" event. Could be any device, but in real life
// this is only USB.
static const char* const kDevSender = "org.freedesktop.systemd1";
static const char* const kDevInterface = "org.freedesktop.systemd1.Manager";
static const char* const kDevMember = "UnitNew";

// Definitions for suspend/resume event
// Link:
// https://www.freedesktop.org/software/systemd/man/latest/org.freedesktop.login1.html#Signals
static const char* const kResSender = "org.freedesktop.login1";
static const char* const kResInterface = "org.freedesktop.login1.Manager";
static const char* const kResMember = "PrepareForSleep";

static void dev_signal_cb(GDBusConnection* connection, const gchar* sender,
                          const gchar* object_path, const gchar* interface,
                          const gchar* signal, GVariant* parameters,
                          gpointer user_data) {
  // printf("%s: %s.%s %s\n", object_path, interface, signal,
  //        g_variant_print(parameters, TRUE));
  auto watch_daemon = static_cast<LinuxUsbWatchDaemon*>(user_data);
  watch_daemon->m_sys_events.evt_new_device.Notify();
}

static void prepare_for_sleep_cb(GDBusConnection* connection,
                                 const gchar* sender, const gchar* object_path,
                                 const gchar* interface, const gchar* signal,
                                 GVariant* parameters, gpointer user_data) {
  gboolean suspending;
  g_variant_get(parameters, "(b)", &suspending);
  auto watch_daemon = static_cast<LinuxUsbWatchDaemon*>(user_data);
  if (!suspending) watch_daemon->m_sys_events.evt_resume.Notify();

  // printf("Resume callback, arg: %s", suspending ? "true" : "false");
}

UsbWatchDaemon& UsbWatchDaemon::GetInstance() {
  static LinuxUsbWatchDaemon watch_daemon(SystemEvents::GetInstance());
  return watch_daemon;
}

LinuxUsbWatchDaemon::~LinuxUsbWatchDaemon() { Stop(); }

void LinuxUsbWatchDaemon::DoStart() {
  g_main_context_push_thread_default(m_worker_context);
  g_main_loop_run(m_main_loop);
  g_main_context_pop_thread_default(m_worker_context);
}

void LinuxUsbWatchDaemon::Start() {
  int filter_id_dev;
  int filter_id_res;
  GError* err = 0;

  if (m_thread.joinable()) return;  // already running
  m_conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, 0, &err);
  filter_id_dev = g_dbus_connection_signal_subscribe(
      m_conn, kDevSender, kDevInterface, kDevMember, 0, 0,
      G_DBUS_SIGNAL_FLAGS_NONE, dev_signal_cb, this, 0);
  filter_id_res = g_dbus_connection_signal_subscribe(
      m_conn, kResSender, kResInterface, kResMember, 0, 0,
      G_DBUS_SIGNAL_FLAGS_NONE, prepare_for_sleep_cb, 0, 0);
  m_worker_context = g_main_context_new();
  m_main_loop = g_main_loop_new(m_worker_context, false);
  m_thread = std::thread([&]{ DoStart(); });
}

void LinuxUsbWatchDaemon::Stop() {
  if (!m_thread.joinable()) return;  // Already stopped
  g_main_loop_unref(m_main_loop);
  g_main_loop_quit(m_main_loop);
  m_thread.join();
}
