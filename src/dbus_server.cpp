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
 ***************************************************************************/

/**
 * \file
 *
 * Implement dbus_server.h using the new GDbus API
 *
 * \see https://docs.gtk.org/glib/
 * \see
 * https://github.com/bratsche/glib/blob/master/gio/tests/gdbus-example-server.c
 */

#include <iostream>            // Debug junk
#include <chrono>
#include <thread>

#include "dbus_server.h"

/** Event handler context */
struct DbusCtx {
  GMainLoop* loop;
  DbusServer* handler;
  DbusCtx(GMainLoop* l, DbusServer* h) : loop(l), handler(h) {}
};

static const GDBusInterfaceVTable interface_vtable = {HandleMethodCall};

/**
 * GDBusConnection is  a singleton and there seem to be no way to change
 * the user_data once provided to g_bus_own_name(). This variable is a band
 * aid and also a memory leak. However, in a live system there is only one
 * instance, so the impact should be limited.
 */
static gpointer static_user_data = 0;

static void OnNameAcquired(GDBusConnection*, const gchar* /* name */,
                           gpointer user_data) {
  auto ctx = static_cast<DbusCtx*>(user_data);
  ctx->handler->SetMainInstance(true);
}

static void OnNameReleased(GDBusConnection*, const gchar* /* name */,
                           gpointer user_data) {
  auto ctx = static_cast<DbusCtx*>(user_data);
  ctx->handler->SetMainInstance(false);
}

/** Bus acquired: register services. Fail silently if not the name owner. */
static void OnBusAcquired(GDBusConnection* connection, const gchar* /* name */,
                          gpointer user_data) {
  auto ctx = static_cast<DbusCtx*>(user_data);
  g_dbus_connection_register_object(
      connection, kDbusObject, ctx->handler->introspection_data->interfaces[0],
      &interface_vtable, user_data, 0 /* user_data_free_func */, 0);
}

static void HandleMethodCall(GDBusConnection*, const gchar* /* sender */,
                             const gchar* /* Object path */,
                             const gchar* /* interface name */,
                             const gchar* method_name, GVariant* parameters,
                             GDBusMethodInvocation* invocation,
                             gpointer /* user_data */) {
  // gpointer udata = g_dbus_method_invocation_get_user_data(invocation);
  auto ctx = static_cast<DbusCtx*>(static_user_data);
  if (0 == g_strcmp0(method_name, "Ping")) {
    GVariant* args[2];
    args[0] = g_variant_new_uint32(21614);
    args[1] = g_variant_new_boolean(true);
    g_dbus_method_invocation_return_value(invocation,
                                          g_variant_new_tuple(args, 2));
  } else if (0 == g_strcmp0(method_name, "Raise")) {
    if (ctx) ctx->handler->on_raise.Notify();
    g_dbus_method_invocation_return_value(invocation, 0);
  } else if (0 == g_strcmp0(method_name, "Quit")) {
    if (ctx) ctx->handler->on_quit.Notify();
    g_dbus_method_invocation_return_value(invocation, 0);
  } else if (ctx && 0 == g_strcmp0(method_name, "Open")) {
    const gchar* path;
    g_variant_get(parameters, "(&s)", &path);
    bool ok = ctx->handler->open_file_cb(std::string(path));
    GVariant* params[1];
    params[0] = g_variant_new_boolean(ok);
    g_dbus_method_invocation_return_value(invocation,
                                          g_variant_new_tuple(params, 1));

  } else if (ctx && g_strcmp0(method_name, "GetRestEndpoint") == 0) {
    GVariant* params[1];
    params[0] =
        g_variant_new_string(ctx->handler->get_rest_api_endpoint_cb().c_str());
    g_dbus_method_invocation_return_value(invocation,
                                          g_variant_new_tuple(params, 1));
  }
}

DbusServer& DbusServer::GetInstance() {
  static DbusServer* server = nullptr; 
  if (!server) server = new DbusServer();
  return *server;
}

DbusServer::DbusServer()
    : LocalServerApi(),
      introspection_data(0),
      m_is_main_instance(false),
      m_owner_id(0) {
  introspection_data = g_dbus_node_info_new_for_xml(introspection_xml, 0);
  g_assert(introspection_data != 0);

  auto flags = static_cast<GBusNameOwnerFlags>(
      G_BUS_NAME_OWNER_FLAGS_NONE | G_BUS_NAME_OWNER_FLAGS_DO_NOT_QUEUE);

  auto ctx = new DbusCtx(0, this);
  auto deleter = [](gpointer p) { delete static_cast<DbusCtx*>(p); };
  static_user_data = ctx;
  m_owner_id = g_bus_own_name(G_BUS_TYPE_SESSION, kDbusName, flags,
                              OnBusAcquired, OnNameAcquired, OnNameReleased,
                              ctx, deleter);
}

DbusServer::~DbusServer() {
  if (m_owner_id) g_bus_unown_name(m_owner_id);
  g_dbus_node_info_unref(introspection_data);
}


void DbusServer::WaitUntilValid() {
    using namespace std::literals::chrono_literals;
    int i = 0;
    do {
      wxYield();
      std::this_thread::sleep_for(5ms);
    } while (!m_is_main_instance &&  i++ < 200);
    wxYield();
}



