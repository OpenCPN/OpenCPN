// gcc $(pkg-config --cflags --libs gio-2.0)  watch_daemon.cpp
#include <stdio.h>
#include <stdlib.h>
#include <gio/gio.h>

static void signal_cb(GDBusConnection *connection, const gchar *sender_name,
                      const gchar *object_path, const gchar *interface_name,
                      const gchar *signal_name, GVariant *parameters,
                      gpointer user_data) {
  printf("%s: %s.%s %s\n", object_path, interface_name, signal_name,
         g_variant_print(parameters, TRUE));
}

int main(int argc, char *argv[]) {
  GDBusConnection *c;
  GMainLoop *loop;
  int filter_id;
  GError *err = NULL;

  c = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
  loop = g_main_loop_new(NULL, 0);
  filter_id = g_dbus_connection_signal_subscribe(
      c, "org.freedesktop.systemd1", "org.freedesktop.systemd1.Manager",
      "UnitNew", NULL, NULL, G_DBUS_SIGNAL_FLAGS_NONE, signal_cb, NULL, NULL);
  g_main_loop_run(loop);
  g_main_loop_unref(loop);

  exit(0);
}
