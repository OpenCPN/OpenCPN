/***************************************************************************
 *   Copyright (C) 2023  Alec Leamas                                       *
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

#include <gio/gio.h>

#include "dbus_client.h"

static GDBusProxy* GetProxy() {
  // session_bus_up();
  // 0std::this_thread::sleep_for(50ms);
  GError* error = 0;
  GDBusConnection* conn = g_bus_get_sync(G_BUS_TYPE_SESSION, 0, &error);
  g_assert_no_error(error);
  //  /* we shouldn't have a name owner nor any cached properties //
  //  g_assert_cmpstr (g_dbus_proxy_get_name_owner (p), ==, NULL);
  //  g_assert (g_dbus_proxy_get_cached_property_names (p) == NULL);
  error = 0;
  GDBusProxy* p = g_dbus_proxy_new_sync(conn, G_DBUS_PROXY_FLAGS_NONE,
                                        0  /* GDBusInterfaceInfo* */,
                                        kDbusName, kDbusObject, kDbusInterface,
                                        0  /* GCancellable */,
                                        &error);
  g_assert_no_error(error);
  return p;
}

LocalApiResult DbusLocalClient::SendRaise() {
  auto proxy = GetProxy();
  if (!proxy) return LocalApiResult(false, "Cannot create proxy");
  GError* error = 0;
  GVariant* result = g_dbus_proxy_call_sync (proxy,
                                             "Raise",
                                             0 /* parameters */,
                                             G_DBUS_CALL_FLAGS_NONE,
                                             -1 /* timeout msec */,
                                             0 /* cancellable */,
                                             &error);
  const std::string message(error ? error->message : "");
  bool ok(error == 0 && g_variant_is_container(result) 
          && g_variant_n_children(result) == 0);
  if (error) g_clear_error(&error);
  g_variant_unref(result);
  g_object_unref(proxy);
  return LocalApiResult(ok, message);
}

LocalApiResult DbusLocalClient::SendQuit() {
  auto proxy = GetProxy();
  if (!proxy) return LocalApiResult(false, "Cannot create proxy");
  GError* error = 0;
  GVariant* result = g_dbus_proxy_call_sync (proxy,
                                             "Quit",
                                             0 /* parameters */,
                                             G_DBUS_CALL_FLAGS_NONE,
                                             -1 /* timeout msec */,
                                             0 /* cancellable */,
                                             &error);
  const std::string message(error ? error->message : "");
  bool ok(error == 0 && g_variant_is_container(result) 
          && g_variant_n_children(result) == 0);
  if (error) g_clear_error(&error);
  g_variant_unref(result);
  g_object_unref(proxy);
  return LocalApiResult(ok, message);
}

LocalApiResult DbusLocalClient::SendOpen(const char* path) {
  auto proxy = GetProxy();
  if (!proxy) return LocalApiResult(false, "Cannot create proxy");
  GError* error = 0;
  GVariant* result = g_dbus_proxy_call_sync (proxy,
                                             "Open",
                                             g_variant_new("(s)", path),
                                             G_DBUS_CALL_FLAGS_NONE,
                                             -1 /* timeout msec */,
                                             0 /* cancellable */,
                                             &error);
  const std::string message(error ? error->message : "");
  bool ok(error == 0 && g_variant_is_container(result) 
          && g_variant_n_children(result) == 1);
  gboolean result_code = false;
  if (ok) {
    GVariant* result_value = g_variant_get_child_value(result, 0);
    result_code = g_variant_get_boolean(result_value);
    g_variant_unref(result_value);
  }
  if (error) g_clear_error(&error);
  g_variant_unref(result);
  g_object_unref(proxy);
  if (!ok) return LocalApiResult(false, "Error invoking DBus server command.");
  if (result_code) return LocalApiResult(true, path);
  return LocalApiResult(false, "Error opening file");
}

LocalApiResult DbusLocalClient::GetRestEndpoint() {
  auto proxy = GetProxy();
  if (!proxy) return LocalApiResult(false, "Cannot create proxy");
  GError* error = 0;
  GVariant* result = g_dbus_proxy_call_sync (proxy,
                                             "GetRestEndpoint",
                                             0 /*arguments */,
                                             G_DBUS_CALL_FLAGS_NONE,
                                             -1 /* timeout msec */,
                                             0 /* cancellable */,
                                             &error);
  const std::string message(error ? error->message : "");
  bool ok(error == 0 && g_variant_is_container(result) 
          && g_variant_n_children(result) == 1);
  std::string result_str;
  if (ok) {
    GVariant* result_value = g_variant_get_child_value(result, 0);
    gsize length;
    const gchar* s = g_variant_get_string(result_value, &length);
    result_str = std::string(s, length);
    g_variant_unref(result_value);
  }
  if (error) g_clear_error(&error);
  g_variant_unref(result);
  g_object_unref(proxy);
  if (ok)
    return LocalApiResult(true, result_str.c_str());
  else 
    return LocalApiResult(false, "Error invoking DBus server command.");
}
