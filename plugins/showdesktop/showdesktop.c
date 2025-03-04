/*
 * Copyright (c) 2005-2007 Jasper Huijsmans <jasper@xfce.org>
 * Copyright (C) 2007-2010 Nick Schermer <nick@xfce.org>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libbladeutil/libbladeutil.h>
#include <common/bar-private.h>
#include <common/bar-utils.h>

#include "showdesktop.h"



static void     show_desktop_plugin_screen_changed          (GtkWidget              *widget,
                                                             GdkScreen              *previous_screen);
static void     show_desktop_plugin_construct               (BladeBarPlugin        *bar_plugin);
static void     show_desktop_plugin_free_data               (BladeBarPlugin        *bar_plugin);
static gboolean show_desktop_plugin_size_changed            (BladeBarPlugin        *bar_plugin,
                                                             gint                    size);
static void     show_desktop_plugin_toggled                 (GtkToggleButton        *button,
                                                             ShowDesktopPlugin      *plugin);
static gboolean show_desktop_plugin_button_release_event    (GtkToggleButton        *button,
                                                             GdkEventButton         *event,
                                                             ShowDesktopPlugin      *plugin);
static void     show_desktop_plugin_showing_desktop_changed (WnckScreen             *wnck_screen,
                                                             ShowDesktopPlugin      *plugin);



struct _ShowDesktopPluginClass
{
  BladeBarPluginClass __parent__;
};

struct _ShowDesktopPlugin
{
  BladeBarPlugin __parent__;

  /* the toggle button */
  GtkWidget  *button;

  /* the wnck screen */
  WnckScreen *wnck_screen;
};



/* define the plugin */
BLADE_BAR_DEFINE_PLUGIN (ShowDesktopPlugin, show_desktop_plugin)



static void
show_desktop_plugin_class_init (ShowDesktopPluginClass *klass)
{
  BladeBarPluginClass *plugin_class;

  plugin_class = BLADE_BAR_PLUGIN_CLASS (klass);
  plugin_class->construct = show_desktop_plugin_construct;
  plugin_class->free_data = show_desktop_plugin_free_data;
  plugin_class->size_changed = show_desktop_plugin_size_changed;
}



static void
show_desktop_plugin_init (ShowDesktopPlugin *plugin)
{
  GtkWidget *button, *image;

  plugin->wnck_screen = NULL;

  /* monitor screen changes */
  g_signal_connect (G_OBJECT (plugin), "screen-changed",
      G_CALLBACK (show_desktop_plugin_screen_changed), NULL);

  /* create the toggle button */
  button = plugin->button = blade_bar_create_toggle_button ();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (plugin), button);
  gtk_widget_set_name (button, "showdesktop-button");
  g_signal_connect (G_OBJECT (button), "toggled",
      G_CALLBACK (show_desktop_plugin_toggled), plugin);
  g_signal_connect (G_OBJECT (button), "button-release-event",
      G_CALLBACK (show_desktop_plugin_button_release_event), plugin);
  blade_bar_plugin_add_action_widget (BLADE_BAR_PLUGIN (plugin), button);
  gtk_widget_show (button);

  image = blade_bar_image_new_from_source ("user-desktop");
  gtk_container_add (GTK_CONTAINER (button), image);
  gtk_widget_show (image);
}



static void
show_desktop_plugin_construct (BladeBarPlugin *bar_plugin)
{
  blade_bar_plugin_set_small (bar_plugin, TRUE);
}



static void
show_desktop_plugin_screen_changed (GtkWidget *widget,
                                    GdkScreen *previous_screen)
{
  ShowDesktopPlugin *plugin = XFCE_SHOW_DESKTOP_PLUGIN (widget);
  WnckScreen        *wnck_screen;
  GdkScreen         *screen;

  bar_return_if_fail (XFCE_IS_SHOW_DESKTOP_PLUGIN (widget));

  /* get the new wnck screen */
  screen = gtk_widget_get_screen (widget);
  wnck_screen = wnck_screen_get (gdk_screen_get_number (screen));
  bar_return_if_fail (WNCK_IS_SCREEN (wnck_screen));

  /* leave when the wnck screen did not change */
  if (plugin->wnck_screen == wnck_screen)
    return;

  /* disconnect signals from an existing wnck screen */
  if (plugin->wnck_screen != NULL)
    g_signal_handlers_disconnect_by_func (G_OBJECT (plugin->wnck_screen),
        show_desktop_plugin_showing_desktop_changed, plugin);

  /* set the new wnck screen */
  plugin->wnck_screen = wnck_screen;
  g_signal_connect (G_OBJECT (wnck_screen), "showing-desktop-changed",
      G_CALLBACK (show_desktop_plugin_showing_desktop_changed), plugin);

  /* toggle the button to the current state or update the tooltip */
  if (G_UNLIKELY (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (plugin->button)) !=
        wnck_screen_get_showing_desktop (wnck_screen)))
    show_desktop_plugin_showing_desktop_changed (wnck_screen, plugin);
  else
    show_desktop_plugin_toggled (GTK_TOGGLE_BUTTON (plugin->button), plugin);
}



static void
show_desktop_plugin_free_data (BladeBarPlugin *bar_plugin)
{
  ShowDesktopPlugin *plugin = XFCE_SHOW_DESKTOP_PLUGIN (bar_plugin);

  /* disconnect screen changed signal */
  g_signal_handlers_disconnect_by_func (G_OBJECT (plugin),
     show_desktop_plugin_screen_changed, NULL);

  /* disconnect handle */
  if (plugin->wnck_screen != NULL)
    g_signal_handlers_disconnect_by_func (G_OBJECT (plugin->wnck_screen),
        show_desktop_plugin_showing_desktop_changed, plugin);
}



static gboolean
show_desktop_plugin_size_changed (BladeBarPlugin *bar_plugin,
                                  gint             size)
{
  bar_return_val_if_fail (XFCE_IS_SHOW_DESKTOP_PLUGIN (bar_plugin), FALSE);

  /* keep the button squared */
  size /= blade_bar_plugin_get_nrows (bar_plugin);
  gtk_widget_set_size_request (GTK_WIDGET (bar_plugin), size, size);

  return TRUE;
}



static void
show_desktop_plugin_toggled (GtkToggleButton   *button,
                             ShowDesktopPlugin *plugin)
{
  gboolean     active;
  const gchar *text;

  bar_return_if_fail (XFCE_IS_SHOW_DESKTOP_PLUGIN (plugin));
  bar_return_if_fail (GTK_IS_TOGGLE_BUTTON (button));
  bar_return_if_fail (WNCK_IS_SCREEN (plugin->wnck_screen));

  /* toggle the desktop */
  active = gtk_toggle_button_get_active (button);
  if (active != wnck_screen_get_showing_desktop (plugin->wnck_screen))
    wnck_screen_toggle_showing_desktop (plugin->wnck_screen, active);

  if (active)
    text = _("Restore the minimized windows");
  else
    text = _("Minimize all open windows and show the desktop");

  gtk_widget_set_tooltip_text (GTK_WIDGET (button), text);
  bar_utils_set_atk_info (GTK_WIDGET (button), _("Show Desktop"), text);
}



static gboolean
show_desktop_plugin_button_release_event (GtkToggleButton   *button,
                                          GdkEventButton    *event,
                                          ShowDesktopPlugin *plugin)
{
  WnckWorkspace *active_ws;
  GList         *windows, *li;
  WnckWindow    *window;

  bar_return_val_if_fail (XFCE_IS_SHOW_DESKTOP_PLUGIN (plugin), FALSE);
  bar_return_val_if_fail (WNCK_IS_SCREEN (plugin->wnck_screen), FALSE);

  if (event->button == 2)
    {
      active_ws = wnck_screen_get_active_workspace (plugin->wnck_screen);
      windows = wnck_screen_get_windows (plugin->wnck_screen);

      for (li = windows; li != NULL; li = li->next)
        {
          window = WNCK_WINDOW (li->data);

          if (wnck_window_get_workspace (window) != active_ws)
            continue;

          /* toggle the shade state */
          if (wnck_window_is_shaded (window))
            wnck_window_unshade (window);
          else
            wnck_window_shade (window);
        }
    }

  return FALSE;
}



static void
show_desktop_plugin_showing_desktop_changed (WnckScreen        *wnck_screen,
                                             ShowDesktopPlugin *plugin)
{
  bar_return_if_fail (XFCE_IS_SHOW_DESKTOP_PLUGIN (plugin));
  bar_return_if_fail (WNCK_IS_SCREEN (wnck_screen));
  bar_return_if_fail (plugin->wnck_screen == wnck_screen);

  /* update button to user action */
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (plugin->button),
      wnck_screen_get_showing_desktop (wnck_screen));
}
