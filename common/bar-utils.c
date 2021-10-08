/*
 * Copyright (C) 2009-2010 Nick Schermer <nick@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include <libbladeui/libbladeui.h>
#include <blxo/blxo.h>

#include <common/bar-private.h>
#include <common/bar-utils.h>



static void
bar_utils_help_button_clicked (GtkWidget       *button,
                                 BladeBarPlugin *bar_plugin)
{
  GtkWidget *toplevel;

  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (bar_plugin));
  bar_return_if_fail (GTK_IS_WIDGET (button));

  toplevel = gtk_widget_get_toplevel (button);
  bar_utils_show_help (GTK_WINDOW (toplevel),
      blade_bar_plugin_get_name (bar_plugin),
      NULL);
}



GtkBuilder *
bar_utils_builder_new (BladeBarPlugin  *bar_plugin,
                         const gchar      *buffer,
                         gsize             length,
                         GObject         **dialog_return)
{
  GError     *error = NULL;
  GtkBuilder *builder;
  GObject    *dialog, *button;

  bar_return_val_if_fail (BLADE_IS_BAR_PLUGIN (bar_plugin), NULL);

  builder = gtk_builder_new ();
  if (gtk_builder_add_from_string (builder, buffer, length, &error))
    {
      dialog = gtk_builder_get_object (builder, "dialog");
      if (G_LIKELY (dialog != NULL))
        {
          g_object_weak_ref (G_OBJECT (dialog),
                             (GWeakNotify) g_object_unref, builder);
          blade_bar_plugin_take_window (bar_plugin, GTK_WINDOW (dialog));

          blade_bar_plugin_block_menu (bar_plugin);
          g_object_weak_ref (G_OBJECT (dialog),
                             (GWeakNotify) blade_bar_plugin_unblock_menu,
                             bar_plugin);

          button = gtk_builder_get_object (builder, "close-button");
          if (G_LIKELY (button != NULL))
            g_signal_connect_swapped (G_OBJECT (button), "clicked",
                G_CALLBACK (gtk_widget_destroy), dialog);

          button = gtk_builder_get_object (builder, "help-button");
          if (G_LIKELY (button != NULL))
            g_signal_connect (G_OBJECT (button), "clicked",
                G_CALLBACK (bar_utils_help_button_clicked), bar_plugin);

          if (G_LIKELY (dialog_return != NULL))
            *dialog_return = dialog;

          return builder;
        }
      else
        {
          g_set_error_literal (&error, 0, 0, "No widget with the name \"dialog\" found");
        }
    }

  g_critical ("Faild to construct the builder for plugin %s-%d: %s.",
              blade_bar_plugin_get_name (bar_plugin),
              blade_bar_plugin_get_unique_id (bar_plugin),
              error->message);
  g_error_free (error);
  g_object_unref (G_OBJECT (builder));

  return NULL;
}



void
bar_utils_show_help (GtkWindow   *parent,
                       const gchar *page,
                       const gchar *offset)
{
  xfce_dialog_show_help (parent, "blade-bar", page, offset);
}



gboolean
bar_utils_grab_available (void)
{
  GdkScreen     *screen;
  GdkWindow     *root;
  GdkGrabStatus  grab_pointer = GDK_GRAB_FROZEN;
  GdkGrabStatus  grab_keyboard = GDK_GRAB_FROZEN;
  gboolean       grab_succeed = FALSE;
  guint          i;
  GdkEventMask   pointer_mask = GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
                                | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
                                | GDK_POINTER_MOTION_MASK;

  screen = xfce_gdk_screen_get_active (NULL);
  root = gdk_screen_get_root_window (screen);

  /* don't try to get the grab for longer then 1/4 second */
  for (i = 0; i < (G_USEC_PER_SEC / 100 / 4); i++)
    {
      grab_keyboard = gdk_keyboard_grab (root, TRUE, GDK_CURRENT_TIME);
      if (grab_keyboard == GDK_GRAB_SUCCESS)
        {
          grab_pointer = gdk_pointer_grab (root, TRUE, pointer_mask,
                                           NULL, NULL, GDK_CURRENT_TIME);
          if (grab_pointer == GDK_GRAB_SUCCESS)
            {
              grab_succeed = TRUE;
              break;
            }
        }

      g_usleep (100);
    }

  /* release the grab so the gtk_menu_popup() can take it */
  if (grab_pointer == GDK_GRAB_SUCCESS)
    gdk_pointer_ungrab (GDK_CURRENT_TIME);
  if (grab_keyboard == GDK_GRAB_SUCCESS)
    gdk_keyboard_ungrab (GDK_CURRENT_TIME);

  if (!grab_succeed)
    {
      g_printerr (PACKAGE_NAME ": Unable to get keyboard and mouse "
                  "grab. Menu popup failed.\n");
    }

  return grab_succeed;
}



void
bar_utils_set_atk_info (GtkWidget   *widget,
                          const gchar *name,
                          const gchar *description)
{
  static gboolean  initialized = FALSE;
  static gboolean  atk_enabled = TRUE;
  AtkObject       *object;

  bar_return_if_fail (GTK_IS_WIDGET (widget));

  if (atk_enabled)
    {
      object = gtk_widget_get_accessible (widget);

      if (!initialized)
        {
          initialized = TRUE;
          atk_enabled = GTK_IS_ACCESSIBLE (object);

          if (!atk_enabled)
            return;
        }

      if (name != NULL)
        atk_object_set_name (object, name);

      if (description != NULL)
        atk_object_set_description (object, description);
    }
}
