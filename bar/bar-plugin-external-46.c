/*
 * Copyright (C) 2009-2010 Nick Schermer <nick@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef HAVE_MATH_H
#include <math.h>
#endif

#include <blxo/blxo.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <libbladeutil/libbladeutil.h>

#include <common/bar-private.h>
#include <common/bar-debug.h>

#include <libbladebar/libbladebar.h>
#include <libbladebar/blade-bar-plugin-provider.h>

#include <bar/bar-module.h>
#include <bar/bar-plugin-external-46.h>
#include <bar/bar-plugin-external.h>
#include <bar/bar-window.h>
#include <bar/bar-dialogs.h>



static void       bar_plugin_external_46_finalize       (GObject              *object);
static gboolean   bar_plugin_external_46_client_event   (GtkWidget            *widget,
                                                           GdkEventClient       *event);
static gchar    **bar_plugin_external_46_get_argv       (BarPluginExternal  *external,
                                                           gchar               **arguments);
static void       bar_plugin_external_46_set_properties (BarPluginExternal  *external,
                                                           GSList               *properties);
static gboolean   bar_plugin_external_46_remote_event   (BarPluginExternal  *external,
                                                           const gchar          *name,
                                                           const GValue         *value,
                                                           guint                *handle);



struct _BarPluginExternal46Class
{
  BarPluginExternalClass __parent__;
};

struct _BarPluginExternal46
{
  BarPluginExternal __parent__;

  gchar *background_image;
};



static GdkAtom bar_atom = GDK_NONE;



G_DEFINE_TYPE (BarPluginExternal46, bar_plugin_external_46, BAR_TYPE_PLUGIN_EXTERNAL)



static void
bar_plugin_external_46_class_init (BarPluginExternal46Class *klass)
{
  GObjectClass             *gobject_class;
  GtkWidgetClass           *gtkwidget_class;
  BarPluginExternalClass *plugin_external_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = bar_plugin_external_46_finalize;

  gtkwidget_class = GTK_WIDGET_CLASS (klass);
  gtkwidget_class->client_event = bar_plugin_external_46_client_event;

  plugin_external_class = BAR_PLUGIN_EXTERNAL_CLASS (klass);
  plugin_external_class->get_argv = bar_plugin_external_46_get_argv;
  plugin_external_class->set_properties = bar_plugin_external_46_set_properties;
  plugin_external_class->remote_event = bar_plugin_external_46_remote_event;

  bar_atom = gdk_atom_intern_static_string (_BAR_CLIENT_EVENT_ATOM);
}



static void
bar_plugin_external_46_init (BarPluginExternal46 *external)
{
  external->background_image = NULL;
}



static void
bar_plugin_external_46_finalize (GObject *object)
{
  BarPluginExternal46 *external = BAR_PLUGIN_EXTERNAL_46 (object);

  g_free (external->background_image);

  (*G_OBJECT_CLASS (bar_plugin_external_46_parent_class)->finalize) (object);
}



static gboolean
bar_plugin_external_46_client_event (GtkWidget      *widget,
                                       GdkEventClient *event)
{
  BarPluginExternal *external = BAR_PLUGIN_EXTERNAL (widget);
  gint                 provider_signal;

  if (event->message_type == bar_atom)
    {
      provider_signal = event->data.s[0];

      switch (provider_signal)
        {
        case PROVIDER_SIGNAL_SHOW_CONFIGURE:
          external->show_configure = TRUE;
          break;

        case PROVIDER_SIGNAL_SHOW_ABOUT:
          external->show_about = TRUE;
          break;

        default:
          /* other signals are handled in bar-applications.c */
          blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (external),
                                                  provider_signal);
          break;
        }

      return FALSE;
    }

  return TRUE;
}



static gchar **
bar_plugin_external_46_get_argv (BarPluginExternal  *external,
                                   gchar               **arguments)
{
  BarPluginExternal46  *external46 = BAR_PLUGIN_EXTERNAL_46 (external);
  guint                  i, argc = PLUGIN_ARGV_ARGUMENTS;
  gchar                 **argv;

  bar_return_val_if_fail (BAR_IS_PLUGIN_EXTERNAL_46 (external), NULL);
  bar_return_val_if_fail (BAR_IS_MODULE (external->module), NULL);
  bar_return_val_if_fail (GTK_IS_SOCKET (external), NULL);

  /* add the number of arguments to the argc count */
  if (G_UNLIKELY (arguments != NULL))
    argc += g_strv_length (arguments);

  /* setup the basic argv */
  argv = g_new0 (gchar *, argc + 1);
  argv[PLUGIN_ARGV_0] = g_strdup (bar_module_get_filename (external->module));
  argv[PLUGIN_ARGV_FILENAME] = g_strdup (""); /* unused, for wrapper only */
  argv[PLUGIN_ARGV_UNIQUE_ID] = g_strdup_printf ("%d", external->unique_id);;
  argv[PLUGIN_ARGV_SOCKET_ID] = g_strdup_printf ("%u", gtk_socket_get_id (GTK_SOCKET (external)));;
  argv[PLUGIN_ARGV_NAME] = g_strdup (bar_module_get_name (external->module));
  argv[PLUGIN_ARGV_DISPLAY_NAME] = g_strdup (bar_module_get_display_name (external->module));
  argv[PLUGIN_ARGV_COMMENT] = g_strdup (bar_module_get_comment (external->module));

  if (external46->background_image != NULL)
    argv[PLUGIN_ARGV_BACKGROUND_IMAGE] = g_strdup (external46->background_image);
  else
    argv[PLUGIN_ARGV_BACKGROUND_IMAGE] = g_strdup ("");

  /* append the arguments */
  if (G_UNLIKELY (arguments != NULL))
    {
      for (i = 0; arguments[i] != NULL; i++)
        argv[i + PLUGIN_ARGV_ARGUMENTS] = g_strdup (arguments[i]);
    }

  return argv;
}



static void
bar_plugin_external_46_set_properties (BarPluginExternal *external,
                                         GSList              *properties)
{
  BarPluginExternal46 *external46 = BAR_PLUGIN_EXTERNAL_46 (external);
  GSList                *li;
  GdkEventClient         event;
  PluginProperty        *property;
  GdkColor               color = { 0, };
  GdkWindow             *window;

  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL_46 (external));
  bar_return_if_fail (bar_atom != GDK_NONE);
  bar_return_if_fail (BAR_IS_MODULE (external->module));

  if (!GTK_WIDGET_REALIZED (external))
    return;

  event.type = GDK_CLIENT_EVENT;
  bar_return_if_fail (GDK_IS_WINDOW (GTK_WIDGET (external)->window));
  event.window = GTK_WIDGET (external)->window;
  event.send_event = TRUE;
  event.message_type = bar_atom;
  event.data_format = 16;

  gdk_error_trap_push ();

  for (li = properties; li != NULL; li = li->next)
    {
      property = li->data;

      event.data.s[0] = property->type;

      switch (property->type)
        {
        case PROVIDER_PROP_TYPE_SET_SIZE:
        case PROVIDER_PROP_TYPE_SET_MODE:
        case PROVIDER_PROP_TYPE_SET_NROWS:
        case PROVIDER_PROP_TYPE_SET_SCREEN_POSITION:
          event.data.s[1] = g_value_get_int (&property->value);
          break;

        case PROVIDER_PROP_TYPE_SET_BACKGROUND_ALPHA:
          event.data.s[1] = g_value_get_double (&property->value) * 100.00;
          break;

        case PROVIDER_PROP_TYPE_SET_LOCKED:
        case PROVIDER_PROP_TYPE_SET_SENSITIVE:
          event.data.s[1] = g_value_get_boolean (&property->value);
          break;

        case PROVIDER_PROP_TYPE_SET_BACKGROUND_COLOR:
          if (gdk_color_parse (g_value_get_string (&property->value), &color))
            {
              event.data.s[1] = color.red;
              event.data.s[2] = color.green;
              event.data.s[3] = color.blue;
              event.data.s[4] = 0;
            }
          break;

        case PROVIDER_PROP_TYPE_SET_BACKGROUND_IMAGE:
          bar_plugin_external_46_set_background_image (external46, g_value_get_string (&property->value));

          /* ask to restart the child */
          bar_plugin_external_restart (external);

          /* stop handling other properties */
          goto bailout;

        case PROVIDER_PROP_TYPE_ACTION_REMOVED:
        case PROVIDER_PROP_TYPE_ACTION_SAVE:
        case PROVIDER_PROP_TYPE_ACTION_QUIT:
        case PROVIDER_PROP_TYPE_ACTION_QUIT_FOR_RESTART:
        case PROVIDER_PROP_TYPE_ACTION_BACKGROUND_UNSET:
        case PROVIDER_PROP_TYPE_ACTION_SHOW_CONFIGURE:
        case PROVIDER_PROP_TYPE_ACTION_SHOW_ABOUT:
          event.data.s[1] = 0;
          break;

        default:
          g_critical ("Received unknown plugin property %u for %s-%d",
                      property->type, bar_module_get_name (external->module),
                      external->unique_id);
          goto bailout;
        }

      window = gtk_socket_get_plug_window (GTK_SOCKET (external));
      bar_return_if_fail (GDK_IS_WINDOW (window));
      gdk_event_send_client_message ((GdkEvent *) &event, GDK_WINDOW_XID (window));
    }

  bailout:

  gdk_flush ();

  if (gdk_error_trap_pop () != 0)
    {
      g_critical ("Failed to send client messages for %s-%d",
                  bar_module_get_name (external->module),
                  external->unique_id);
    }
}



static gboolean
bar_plugin_external_46_remote_event (BarPluginExternal *external,
                                       const gchar         *name,
                                       const GValue        *value,
                                       guint               *handle)
{
  bar_return_val_if_fail (BAR_IS_PLUGIN_EXTERNAL_46 (external), TRUE);
  bar_return_val_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (external), TRUE);

  g_warning ("Plugin %s is compiled as an Xfce 4.6 binary. It needs to be "
             "ported to the new library plugin framework to be able to use "
             "remote events.", bar_module_get_name (external->module));

  return TRUE;
}



GtkWidget *
bar_plugin_external_46_new (BarModule  *module,
                              gint          unique_id,
                              gchar       **arguments)
{
  bar_return_val_if_fail (BAR_IS_MODULE (module), NULL);
  bar_return_val_if_fail (unique_id != -1, NULL);

  return g_object_new (BAR_TYPE_PLUGIN_EXTERNAL_46,
                       "module", module,
                       "unique-id", unique_id,
                       "arguments", arguments, NULL);
}



void
bar_plugin_external_46_set_background_image (BarPluginExternal46 *external,
                                               const gchar           *image)
{
  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (external));

  g_free (external->background_image);
  external->background_image = g_strdup (image);
}
