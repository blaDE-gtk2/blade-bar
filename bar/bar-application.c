/*
 * Copyright (C) 2008-2010 Nick Schermer <nick@xfce.org>
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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <blxo/blxo.h>
#include <glib/gstdio.h>
#include <blconf/blconf.h>
#include <libbladeutil/libbladeutil.h>
#include <libbladeui/libbladeui.h>

#ifdef GDK_WINDOWING_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif

#include <common/bar-private.h>
#include <common/bar-blconf.h>
#include <common/bar-debug.h>
#include <common/bar-utils.h>
#include <libbladebar/libbladebar.h>
#include <libbladebar/blade-bar-plugin-provider.h>

#include <bar/bar-dbus-service.h>
#include <bar/bar-base-window.h>
#include <bar/bar-plugin-external-46.h>
#include <bar/bar-window.h>
#include <bar/bar-application.h>
#include <bar/bar-itembar.h>
#include <bar/bar-module-factory.h>
#include <bar/bar-preferences-dialog.h>
#include <bar/bar-item-dialog.h>
#include <bar/bar-dialogs.h>
#include <bar/bar-plugin-external.h>

#define AUTOSAVE_INTERVAL (10 * 60)
#define MIGRATE_BIN       HELPERDIR G_DIR_SEPARATOR_S "migrate"



static void      bar_application_finalize           (GObject                *object);
static gboolean  bar_application_autosave_timer     (gpointer                user_data);
static void      bar_application_plugin_move        (GtkWidget              *item,
                                                       BarApplication       *application);
static gboolean  bar_application_plugin_insert      (BarApplication       *application,
                                                       BarWindow            *window,
                                                       const gchar            *name,
                                                       gint                    unique_id,
                                                       gchar                 **arguments,
                                                       gint                    position);
static void      bar_application_dialog_destroyed   (GtkWindow              *dialog,
                                                       BarApplication       *application);
static void      bar_application_drag_data_received (BarWindow            *window,
                                                       GdkDragContext         *context,
                                                       gint                    x,
                                                       gint                    y,
                                                       GtkSelectionData       *selection_data,
                                                       guint                   info,
                                                       guint                   drag_time,
                                                       GtkWidget              *itembar);
static gboolean  bar_application_drag_motion        (GtkWidget              *window,
                                                       GdkDragContext         *context,
                                                       gint                    x,
                                                       gint                    y,
                                                       guint                   drag_time,
                                                       BarApplication       *application);
static gboolean  bar_application_drag_drop          (GtkWidget              *window,
                                                       GdkDragContext         *context,
                                                       gint                    x,
                                                       gint                    y,
                                                       guint                   drag_time,
                                                       BarApplication       *application);
static void      bar_application_drag_leave         (GtkWidget              *window,
                                                       GdkDragContext         *context,
                                                       guint                   drag_time,
                                                       BarApplication       *application);



enum
{
  PROP_0,
  PROP_ITEMS_CHANGED
};

struct _BarApplicationClass
{
  GObjectClass __parent__;
};

struct _BarApplication
{
  GObject  __parent__;

  /* the plugin factory */
  BarModuleFactory *factory;

  /* blconf channel */
  BlconfChannel      *blconf;

  /* internal list of all the bar windows */
  GSList             *windows;

  /* internal list of opened dialogs */
  GSList             *dialogs;

  /* autosave timer for plugins */
  guint               autosave_timer_id;

#ifdef GDK_WINDOWING_X11
  guint               wait_for_wm_timeout_id;
#endif

  /* drag and drop data */
  guint               drop_data_ready : 1;
  guint               drop_occurred : 1;
  guint               drop_desktop_files : 1;
  guint               drop_index;
};

#ifdef GDK_WINDOWING_X11
typedef struct
{
  BarApplication *application;

  Display          *dpy;
  Atom             *atoms;
  guint             atom_count;
  guint             have_wm : 1;
  guint             counter;
}
WaitForWM;
#endif

enum
{
  TARGET_PLUGIN_NAME,
  TARGET_PLUGIN_WIDGET,
  TARGET_TEXT_URI_LIST
};

static const GtkTargetEntry drag_targets[] =
{
  { "blade-bar/plugin-widget",
    GTK_TARGET_SAME_APP, TARGET_PLUGIN_WIDGET }
};

static const GtkTargetEntry drop_targets[] =
{
  { "blade-bar/plugin-name",
    GTK_TARGET_SAME_APP, TARGET_PLUGIN_NAME },
  { "blade-bar/plugin-widget",
    GTK_TARGET_SAME_APP, TARGET_PLUGIN_WIDGET },
  { "text/uri-list", 0, TARGET_TEXT_URI_LIST }
};



G_DEFINE_TYPE (BarApplication, bar_application, G_TYPE_OBJECT)



static void
bar_application_class_init (BarApplicationClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = bar_application_finalize;
}



static void
bar_application_init (BarApplication *application)
{
  GError *error = NULL;
  gint    configver;

  application->windows = NULL;
  application->dialogs = NULL;
  application->drop_desktop_files = FALSE;
  application->drop_data_ready = FALSE;
  application->drop_occurred = FALSE;

  /* get the blconf channel (singleton) */
  application->blconf = bar_properties_get_channel (G_OBJECT (application));

  /* check if we need to migrate configuration */
  configver = blconf_channel_get_int (application->blconf, "/configver", -1);
  if (G_UNLIKELY (configver < BLADE_BAR_CONFIG_VERSION))
    {
      if (!g_spawn_command_line_sync (MIGRATE_BIN, NULL, NULL, NULL, &error))
        {
          xfce_dialog_show_error (NULL, error, _("Failed to launch the migration application"));
          g_error_free (error);
        }
    }

  /* check if we need to force all plugins to run external */
  if (blconf_channel_get_bool (application->blconf, "/force-all-external", FALSE))
    bar_module_factory_force_all_external ();

  /* get a factory reference so it never unloads */
  application->factory = bar_module_factory_get ();

  /* start the autosave timer for plugins */
  application->autosave_timer_id = g_timeout_add_seconds (60 * 10,
      bar_application_autosave_timer, application);
}



static void
bar_application_finalize (GObject *object)
{
  BarApplication *application = BAR_APPLICATION (object);

  bar_return_if_fail (application->dialogs == NULL);

  if (application->autosave_timer_id != 0)
    {
      g_source_remove (application->autosave_timer_id);

      /* save plugins */
      bar_application_autosave_timer (application);
    }

#ifdef GDK_WINDOWING_X11
  /* stop autostart timeout */
  if (application->wait_for_wm_timeout_id != 0)
    g_source_remove (application->wait_for_wm_timeout_id);
#endif

  /* destroy all bars */
  g_slist_foreach (application->windows, (GFunc) gtk_widget_destroy, NULL);
  g_slist_free (application->windows);

  g_object_unref (G_OBJECT (application->factory));

  /* this is a good reference if all the objects are released */
  bar_debug (BAR_DEBUG_APPLICATION, "finalized");

  (*G_OBJECT_CLASS (bar_application_parent_class)->finalize) (object);
}



static gboolean
bar_application_autosave_timer (gpointer user_data)
{
  BarApplication *application = BAR_APPLICATION (user_data);

  /* emit a save signal for the plugins */
  bar_application_save (application, SAVE_PLUGIN_PROVIDERS);

  return TRUE;
}



static void
bar_application_blconf_window_bindings (BarApplication *application,
                                          BarWindow      *window,
                                          gboolean          save_properties)
{
  gchar               *property_base;
  const BarProperty  properties[] =
  {
    { "position-locked", G_TYPE_BOOLEAN },
    { "autohide-behavior", G_TYPE_UINT },
    { "span-monitors", G_TYPE_BOOLEAN },
    { "mode", G_TYPE_UINT },
    { "size", G_TYPE_UINT },
    { "nrows", G_TYPE_UINT },
    { "length", G_TYPE_UINT },
    { "length-adjust", G_TYPE_BOOLEAN },
    { "enter-opacity", G_TYPE_UINT },
    { "leave-opacity", G_TYPE_UINT },
    { "background-alpha", G_TYPE_UINT },
    { "background-style", G_TYPE_UINT },
    { "background-color", GDK_TYPE_COLOR },
    { "background-image", G_TYPE_STRING },
    { "output-name", G_TYPE_STRING },
    { "position", G_TYPE_STRING },
    { "disable-struts", G_TYPE_BOOLEAN },
    { NULL }
  };

  bar_return_if_fail (BLCONF_IS_CHANNEL (application->blconf));

  /* create the property base */
  property_base = g_strdup_printf ("/bars/bar-%d", bar_window_get_id (window));

  /* migrate old autohide property */
  bar_window_migrate_autohide_property (window, application->blconf, property_base);

  /* bind all the properties */
  bar_properties_bind (application->blconf, G_OBJECT (window),
                         property_base, properties, save_properties);

  /* set locking for this bar */
  bar_window_set_locked (window,
      blconf_channel_is_property_locked (application->blconf, property_base));

  g_free (property_base);
}



static void
bar_application_load_real (BarApplication *application)
{
  BarWindow  *window;
  guint         i, j, n_bars;
  gchar         buf[50];
  gchar        *name;
  gint          unique_id;
  GdkScreen    *screen;
  GPtrArray    *array;
  const GValue *value;
  gchar        *output_name;
  gint          screen_num;
  GdkDisplay   *display;
  GValue        val = { 0, };
  GPtrArray    *bars;
  gint          bar_id;
  gboolean      save_changed_ids = FALSE;

  bar_return_if_fail (BAR_IS_APPLICATION (application));
  bar_return_if_fail (BLCONF_IS_CHANNEL (application->blconf));

  display = gdk_display_get_default ();

  if (blconf_channel_get_property (application->blconf, "/bars", &val)
      && (G_VALUE_HOLDS_UINT (&val)
          || G_VALUE_HOLDS (&val, BAR_PROPERTIES_TYPE_VALUE_ARRAY)))
    {
      if (G_VALUE_HOLDS_UINT (&val))
        {
          n_bars = g_value_get_uint (&val);
          bars = NULL;
        }
      else
        {
          bars = g_value_get_boxed (&val);
          n_bars = bars->len;
        }

      /* walk all the bar in the configuration */
      for (i = 0; i < n_bars; i++)
        {
          screen = NULL;

          /* get the bar id */
          if (bars != NULL)
            {
              /* get the id from the array */
              value = g_ptr_array_index (bars, i);
              bar_assert (value != NULL);
              bar_id = g_value_get_int (value);
            }
          else
            {
              /* use the list position if /bars is an uint */
              bar_id = i;
            }

          /* start the bar directly on the correct screen */
          g_snprintf (buf, sizeof (buf), "/bars/bar-%d/output-name", bar_id);
          output_name = blconf_channel_get_string (application->blconf, buf, NULL);
          if (output_name != NULL
              && strncmp (output_name, "screen-", 7) == 0
              && sscanf (output_name, "screen-%d", &screen_num) == 1)
            {
              if (screen_num < gdk_display_get_n_screens (display))
                screen = gdk_display_get_screen (display, screen_num);
            }
          g_free (output_name);

          /* create a new window */
          window = bar_application_new_window (application, screen, bar_id, FALSE);

          /* walk all the plugins on the bar */
          g_snprintf (buf, sizeof (buf), "/bars/bar-%d/plugin-ids", bar_id);
          array = blconf_channel_get_arrayv (application->blconf, buf);
          if (array == NULL)
            continue;

          for (j = 0; j < array->len; j++)
            {
              /* get the plugin id */
              value = g_ptr_array_index (array, j);
              bar_assert (value != NULL);
              unique_id = g_value_get_int (value);

              /* get the plugin name */
              g_snprintf (buf, sizeof (buf), "/plugins/plugin-%d", unique_id);
              name = blconf_channel_get_string (application->blconf, buf, NULL);

              /* append the plugin to the bar */
              if (unique_id < 1 || name == NULL
                  || !bar_application_plugin_insert (application, window,
                                                       name, unique_id, NULL, -1))
                {
                  /* plugin could not be loaded, remove it from the channel */
                  g_snprintf (buf, sizeof (buf), "/bars/plugin-%d", unique_id);
                  if (blconf_channel_has_property (application->blconf, buf))
                    blconf_channel_reset_property (application->blconf, buf, TRUE);

                  /* show warnings */
                  g_message ("Plugin \"%s-%d\" was not found and has been "
                             "removed from the configuration", name, unique_id);

                  /* save configuration change after loading */
                  save_changed_ids = TRUE;
                }

              g_free (name);
            }

          blconf_array_free (array);
        }

      /* free blconf array or uint */
      g_value_unset (&val);
    }

  /* create empty window if everything else failed */
  if (G_UNLIKELY (application->windows == NULL))
    bar_application_new_window (application, NULL, -1, TRUE);

  if (save_changed_ids)
    bar_application_save (application, SAVE_PLUGIN_IDS);
}



#ifdef GDK_WINDOWING_X11
static gboolean
bar_application_wait_for_window_manager (gpointer data)
{
  WaitForWM *wfwm = data;
  guint      i;
  gboolean   have_wm = TRUE;

  for (i = 0; i < wfwm->atom_count; i++)
    {
      if (XGetSelectionOwner (wfwm->dpy, wfwm->atoms[i]) == None)
        {
          bar_debug (BAR_DEBUG_APPLICATION, "window manager not ready on screen %d", i);

          have_wm = FALSE;
          break;
        }
    }

  wfwm->have_wm = have_wm;

  /* abort if a window manager is found or 5 seconds expired */
  return wfwm->counter++ < 20 * 5 && !wfwm->have_wm;
}



static void
bar_application_wait_for_window_manager_destroyed (gpointer data)
{
  WaitForWM        *wfwm = data;
  BarApplication *application = wfwm->application;

  application->wait_for_wm_timeout_id = 0;

  if (!wfwm->have_wm)
    {
      g_printerr (G_LOG_DOMAIN ": No window manager registered on screen 0. "
                  "To start the bar without this check, run with --disable-wm-check.\n");
    }
  else
    {
      bar_debug (BAR_DEBUG_APPLICATION, "found window manager after %d tries",
                   wfwm->counter);
    }

  g_free (wfwm->atoms);
  XCloseDisplay (wfwm->dpy);
  g_slice_free (WaitForWM, wfwm);

  /* start loading the bars, hopefully a window manager is found, but it
   * probably also works fine without... */
  GDK_THREADS_ENTER ();
  bar_application_load_real (application);
  GDK_THREADS_LEAVE ();
}
#endif



static void
bar_application_plugin_move_drag_data_get (GtkWidget        *item,
                                             GdkDragContext   *drag_context,
                                             GtkSelectionData *selection_data,
                                             guint             info,
                                             guint             drag_time,
                                             BarApplication *application)
{
  /* set some data, we never use this, but GTK_DEST_DEFAULT_ALL
   * used in the item dialog requires this */
  gtk_selection_data_set (selection_data,
                          selection_data->target, 8,
                          (const guchar *) "0", 1);
}



static void
bar_application_plugin_move_drag_end (GtkWidget        *item,
                                        GdkDragContext   *context,
                                        BarApplication *application)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (item));
  bar_return_if_fail (BAR_IS_APPLICATION (application));

  /* disconnect this signal */
  g_signal_handlers_disconnect_by_func (G_OBJECT (item),
      G_CALLBACK (bar_application_plugin_move_drag_end), application);
  g_signal_handlers_disconnect_by_func (G_OBJECT (item),
      G_CALLBACK (bar_application_plugin_move_drag_data_get), application);

  /* unblock autohide */
  bar_application_windows_blocked (application, FALSE);
}



static void
bar_application_plugin_move (GtkWidget        *item,
                               BarApplication *application)
{
  GtkTargetList  *target_list;
  const gchar    *icon_name;
  GdkDragContext *context;
  BarModule    *module;
  GtkIconTheme   *theme;

  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (item));
  bar_return_if_fail (BAR_IS_APPLICATION (application));

  /* block autohide */
  bar_application_windows_blocked (application, TRUE);

  /* create drag context */
  target_list = gtk_target_list_new (drag_targets, G_N_ELEMENTS (drag_targets));
  context = gtk_drag_begin (item, target_list, GDK_ACTION_MOVE, 1, NULL);
  gtk_target_list_unref (target_list);

  /* set the drag context icon name */
  module = bar_module_get_from_plugin_provider (BLADE_BAR_PLUGIN_PROVIDER (item));
  icon_name = bar_module_get_icon_name (module);
  theme = gtk_icon_theme_get_for_screen (gtk_widget_get_screen (item));
  if (!blxo_str_is_empty (icon_name)
      && gtk_icon_theme_has_icon (theme, icon_name))
    gtk_drag_set_icon_name (context, icon_name, 0, 0);
  else
    gtk_drag_set_icon_default (context);

  /* signal to make the window sensitive again on a drag end */
  g_signal_connect (G_OBJECT (item), "drag-end",
      G_CALLBACK (bar_application_plugin_move_drag_end), application);
  g_signal_connect (G_OBJECT (item), "drag-data-get",
      G_CALLBACK (bar_application_plugin_move_drag_data_get), application);
}



static void
bar_application_plugin_delete_config (BarApplication *application,
                                        const gchar      *name,
                                        gint              unique_id)
{
  gchar *property;
  gchar *filename, *path;

  bar_return_if_fail (BAR_IS_APPLICATION (application));
  bar_return_if_fail (!blxo_str_is_empty (name));
  bar_return_if_fail (unique_id != -1);

  /* remove the blconf property */
  property = g_strdup_printf (BAR_PLUGIN_PROPERTY_BASE, unique_id);
  if (blconf_channel_has_property (application->blconf, property))
    blconf_channel_reset_property (application->blconf, property, TRUE);
  g_free (property);

  /* lookup the rc file */
  filename = g_strdup_printf (BAR_PLUGIN_RC_RELATIVE_PATH, name, unique_id);
  path = xfce_resource_lookup (XFCE_RESOURCE_CONFIG, filename);
  g_free (filename);

  /* unlink the rc file */
  if (G_LIKELY (path != NULL))
    g_unlink (path);
  g_free (path);
}



static void
bar_application_plugin_remove (GtkWidget *widget,
                                 gpointer   user_data)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (widget));

  /* ask the plugin to cleanup when we destroy a bar window */
  blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (widget),
                                          PROVIDER_SIGNAL_REMOVE_PLUGIN);
}



static void
bar_application_plugin_provider_signal (BladeBarPluginProvider       *provider,
                                          BladeBarPluginProviderSignal  provider_signal,
                                          BarApplication              *application)
{
  GtkWidget   *itembar;
  BarWindow *window;
  gint         unique_id;
  gchar       *name;

  bar_return_if_fail (BAR_IS_APPLICATION (application));
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  window = BAR_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (provider)));
  bar_return_if_fail (BAR_IS_WINDOW (window));

  switch (provider_signal)
    {
    case PROVIDER_SIGNAL_MOVE_PLUGIN:
      /* check the window locking, not that of the provider, because
       * the users might have worked around that and both should be identical */
      if (!bar_window_get_locked (window))
        bar_application_plugin_move (GTK_WIDGET (provider), application);
      break;

    case PROVIDER_SIGNAL_EXPAND_PLUGIN:
    case PROVIDER_SIGNAL_COLLAPSE_PLUGIN:
      itembar = gtk_bin_get_child (GTK_BIN (window));
      gtk_container_child_set (GTK_CONTAINER (itembar),
                               GTK_WIDGET (provider),
                               "expand",
                               provider_signal == PROVIDER_SIGNAL_EXPAND_PLUGIN,
                               NULL);
      break;

    case PROVIDER_SIGNAL_SHRINK_PLUGIN:
    case PROVIDER_SIGNAL_UNSHRINK_PLUGIN:
      itembar = gtk_bin_get_child (GTK_BIN (window));
      gtk_container_child_set (GTK_CONTAINER (itembar),
                               GTK_WIDGET (provider),
                               "shrink",
                               provider_signal == PROVIDER_SIGNAL_SHRINK_PLUGIN,
                               NULL);
      break;

    case PROVIDER_SIGNAL_SMALL_PLUGIN:
    case PROVIDER_SIGNAL_UNSMALL_PLUGIN:
      itembar = gtk_bin_get_child (GTK_BIN (window));
      gtk_container_child_set (GTK_CONTAINER (itembar),
                               GTK_WIDGET (provider),
                               "small",
                               provider_signal == PROVIDER_SIGNAL_SMALL_PLUGIN,
                               NULL);
      break;

    case PROVIDER_SIGNAL_LOCK_BAR:
      /* increase window's autohide counter */
      bar_window_freeze_autohide (window);
      break;

    case PROVIDER_SIGNAL_UNLOCK_BAR:
      /* decrease window's autohide counter */
      bar_window_thaw_autohide (window);
      break;

    case PROVIDER_SIGNAL_REMOVE_PLUGIN:
      /* check the window locking, not that of the provider, because
       * the users might have worked around that and both should be identical */
      if (!bar_window_get_locked (window))
        {
          /* give plugin the opportunity to cleanup special configuration */
          blade_bar_plugin_provider_removed (provider);

          /* store the provider's unique id and name (lost after destroy) */
          unique_id = blade_bar_plugin_provider_get_unique_id (provider);
          name = g_strdup (blade_bar_plugin_provider_get_name (provider));

          /* destroy the plugin */
          gtk_widget_destroy (GTK_WIDGET (provider));

          /* remove the plugin configuration */
          bar_application_plugin_delete_config (application, name, unique_id);
          g_free (name);

          /* save new ids */
          bar_application_save_window (application, window, SAVE_PLUGIN_IDS);
        }
      break;

    case PROVIDER_SIGNAL_ADD_NEW_ITEMS:
      /* open the items dialog, locking is handled in the object */
      bar_item_dialog_show (window);
      break;

    case PROVIDER_SIGNAL_BAR_PREFERENCES:
      /* open the bar preferences, locking is handled in the object */
      bar_preferences_dialog_show (window);
      break;

    case PROVIDER_SIGNAL_BAR_LOGOUT:
      /* logout */
      bar_application_logout ();
      break;

    case PROVIDER_SIGNAL_BAR_ABOUT:
      /* show the about dialog */
      bar_dialogs_show_about ();
      break;

    case PROVIDER_SIGNAL_BAR_HELP:
      /* try to launch help browser */
      bar_utils_show_help (NULL, NULL, NULL);
      break;

    case PROVIDER_SIGNAL_FOCUS_PLUGIN:
       /* focus the bar window (as part of focusing a widget within the plugin) */
       bar_window_focus (window);
       break;

    case PROVIDER_SIGNAL_SHOW_CONFIGURE:
    case PROVIDER_SIGNAL_SHOW_ABOUT:
      /* signals we can ignore, only for external plugins */
      break;

    default:
      g_critical ("Received unknown provider signal %d", provider_signal);
      break;
    }
}



static gboolean
bar_application_plugin_insert (BarApplication  *application,
                                 BarWindow       *window,
                                 const gchar       *name,
                                 gint               unique_id,
                                 gchar            **arguments,
                                 gint               position)
{
  GtkWidget *itembar, *provider;
  gint       new_unique_id;

  bar_return_val_if_fail (BAR_IS_APPLICATION (application), FALSE);
  bar_return_val_if_fail (BAR_IS_WINDOW (window), FALSE);
  bar_return_val_if_fail (name != NULL, FALSE);

  /* create a new bar plugin */
  provider = bar_module_factory_new_plugin (application->factory, name,
                                              gtk_window_get_screen (GTK_WINDOW (window)),
                                              unique_id, arguments, &new_unique_id);
  if (G_UNLIKELY (provider == NULL))
    return FALSE;

  /* make sure there is no bar configuration with this unique id when a
   * new plugin is created */
  if (G_UNLIKELY (unique_id == -1))
    bar_application_plugin_delete_config (application, name, new_unique_id);

  /* add signal to monitor provider signals */
  g_signal_connect (G_OBJECT (provider), "provider-signal",
      G_CALLBACK (bar_application_plugin_provider_signal), application);

  /* work around the problem that we need a background before
   * realizing for 4.6 bar plugins */
  if (BAR_BASE_WINDOW (window)->background_style == BAR_BG_STYLE_IMAGE
      && BAR_IS_PLUGIN_EXTERNAL_46 (provider))
    bar_plugin_external_set_background_image (BAR_PLUGIN_EXTERNAL (provider),
        BAR_BASE_WINDOW (window)->background_image);

  /* add the item to the bar */
  itembar = gtk_bin_get_child (GTK_BIN (window));
  bar_itembar_insert (BAR_ITEMBAR (itembar),
                        GTK_WIDGET (provider), position);

  /* send all the needed info about the bar to the plugin */
  bar_window_set_povider_info (window, provider, FALSE);

  /* show the plugin */
  gtk_widget_show (provider);

  return TRUE;
}



static void
bar_application_dialog_destroyed (GtkWindow        *dialog,
                                    BarApplication *application)
{
  bar_return_if_fail (GTK_IS_WINDOW (dialog));
  bar_return_if_fail (BAR_IS_APPLICATION (application));
  bar_return_if_fail (g_slist_find (application->dialogs, dialog) != NULL);

  /* remove the window from the list */
  application->dialogs = g_slist_remove (application->dialogs, dialog);

  /* unblock autohide if there are no open windows anymore */
  if (application->dialogs == NULL)
    bar_application_windows_blocked (application, FALSE);
}



static void
bar_application_drag_data_received (BarWindow      *window,
                                      GdkDragContext   *context,
                                      gint              x,
                                      gint              y,
                                      GtkSelectionData *selection_data,
                                      guint             info,
                                      guint             drag_time,
                                      GtkWidget        *itembar)
{
  BarApplication  *application;
  GtkWidget         *provider;
  gboolean           succeed = FALSE;
  gboolean           save_application = FALSE;
  const gchar       *name;
  guint              old_position;
  gchar            **uris;
  guint              i;
  gboolean           found;
  gint               n_items;
  gboolean           child_small;
  gboolean           child_expand;
  gboolean           child_shrink;
  GtkWidget         *parent_itembar;

  bar_return_if_fail (BAR_IS_WINDOW (window));
  bar_return_if_fail (GDK_IS_DRAG_CONTEXT (context));
  bar_return_if_fail (BAR_IS_ITEMBAR (itembar));

  application = bar_application_get ();

  /* we don't allow any kind of drops here when the bar is locked */
  if (bar_application_get_locked (application)
      || bar_window_get_locked (window))
    goto invalid_drag;

  if (!application->drop_data_ready)
    {
      bar_assert (!application->drop_desktop_files);

      if (info == TARGET_TEXT_URI_LIST)
        {
          /* look if the selection data contains atleast 1 desktop file */
          uris = gtk_selection_data_get_uris (selection_data);
          if (G_LIKELY (uris != NULL))
            {
              for (i = 0, found = FALSE; !found && uris[i] != NULL; i++)
                found = g_str_has_suffix (uris[i], ".desktop");
              g_strfreev (uris);
              application->drop_desktop_files = found;
            }
        }

      /* reset the state */
      application->drop_data_ready = TRUE;
    }

  /* check if the data was droppped */
  if (application->drop_occurred)
    {
      /* reset the state */
      application->drop_occurred = FALSE;

      switch (info)
        {
        case TARGET_PLUGIN_NAME:
          if (G_LIKELY (selection_data->length > 0))
            {
              /* create a new item with a unique id */
              name = (const gchar *) selection_data->data;
              succeed = bar_application_plugin_insert (application, window, name,
                                                         -1, NULL, application->drop_index);
            }
          break;

        case TARGET_PLUGIN_WIDGET:
          /* get the source widget */
          provider = gtk_drag_get_source_widget (context);

          /* debug check */
          bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

          /* check if we move to another itembar */
          parent_itembar = gtk_widget_get_parent (provider);
          if (parent_itembar == itembar)
            {
              /* get the current position on the itembar */
              old_position = bar_itembar_get_child_index (BAR_ITEMBAR (itembar),
                                                            provider);

              /* decrease the counter if we drop after the current position */
              if (application->drop_index > old_position)
                application->drop_index--;

              /* reorder the child if needed */
              if (old_position != application->drop_index)
                bar_itembar_reorder_child (BAR_ITEMBAR (itembar), provider, application->drop_index);
            }
          else
            {
              /* get properties from old itemsbar */
              gtk_container_child_get (GTK_CONTAINER (parent_itembar), provider,
                                       "expand", &child_expand,
                                       "shrink", &child_shrink,
                                       "small", &child_small, NULL);

              /* reparent the widget, this will also call remove and add for the itembar */
              gtk_widget_hide (provider);
              gtk_widget_reparent (provider, itembar);
              gtk_widget_show (provider);

              /* move the item to the correct position on the itembar */
              bar_itembar_reorder_child (BAR_ITEMBAR (itembar), provider, application->drop_index);

              /* restore child properties */
              gtk_container_child_set (GTK_CONTAINER (itembar), provider,
                                       "expand", child_expand,
                                       "shrink", child_shrink,
                                       "small", child_small, NULL);

              /* send all the needed bar information to the plugin */
              bar_window_set_povider_info (window, provider, TRUE);

              /* moved between bars, save everything */
              save_application = TRUE;
            }

          /* everything went fine */
          succeed = TRUE;
          break;

        case TARGET_TEXT_URI_LIST:
          if (G_LIKELY (application->drop_desktop_files))
            {
              /* pass all the uris to the launcher, it will filter out
               * the desktop files on it's own */
              uris = gtk_selection_data_get_uris (selection_data);
              if (G_LIKELY (uris != NULL))
                {
                  n_items = g_strv_length (uris);
                  if (xfce_dialog_confirm (NULL, GTK_STOCK_ADD, _("Create _Launcher"),
                                           _("This will create a new launcher plugin on the bar and inserts "
                                             "the dropped files as menu items."),
                                           ngettext ("Create new launcher from %d desktop file",
                                                     "Create new launcher from %d desktop files",
                                                     n_items),
                                           n_items))
                    {
                      /* create a new item with a unique id */
                      succeed = bar_application_plugin_insert (application, window, LAUNCHER_PLUGIN_NAME,
                                                                 -1, uris, application->drop_index);
                    }

                  g_strfreev (uris);
                }

              application->drop_desktop_files = FALSE;
            }
          break;

        default:
          bar_assert_not_reached ();
          break;
        }

      /* save the new plugin ids */
      if (G_LIKELY (save_application))
        bar_application_save (application, SAVE_PLUGIN_IDS);
      else if (succeed)
        bar_application_save_window (application, window, SAVE_PLUGIN_IDS);

      /* tell the peer that we handled the drop */
      gtk_drag_finish (context, succeed, FALSE, drag_time);
    }
  else
    {
      invalid_drag:
      gdk_drag_status (context, 0, drag_time);
    }

  g_object_unref (G_OBJECT (application));
}



static gboolean
bar_application_drag_motion (GtkWidget        *window,
                               GdkDragContext   *context,
                               gint              x,
                               gint              y,
                               guint             drag_time,
                               BarApplication *application)
{
  GdkAtom        target;
  GtkWidget     *itembar;
  GdkDragAction  drag_action = 0;

  bar_return_val_if_fail (BAR_IS_WINDOW (window), FALSE);
  bar_return_val_if_fail (GDK_IS_DRAG_CONTEXT (context), FALSE);
  bar_return_val_if_fail (BAR_IS_APPLICATION (application), FALSE);
  bar_return_val_if_fail (!application->drop_occurred, FALSE);

  /* don't allow anything when the window is locked */
  if (bar_window_get_locked (BAR_WINDOW (window)))
    goto not_a_drop_zone;

  /* determine the drag target */
  target = gtk_drag_dest_find_target (window, context, NULL);

  if (target == gdk_atom_intern_static_string ("text/uri-list"))
    {
      /* request the drop data on-demand (if we don't have it already) */
      if (!application->drop_data_ready)
        {
          /* request the drop data from the source */
          gtk_drag_get_data (window, context, target, drag_time);

          /* we cannot drop here (yet!) */
          return TRUE;
        }
      else if (application->drop_desktop_files)
        {
          /* there are valid uris in the drop data */
          drag_action = GDK_ACTION_COPY;
        }
    }
  else if (target == gdk_atom_intern_static_string ("blade-bar/plugin-name"))
    {
      /* insert a new plugin */
      drag_action = GDK_ACTION_COPY;
    }
  else if (target == gdk_atom_intern_static_string ("blade-bar/plugin-widget"))
    {
      /* move an existing plugin */
      drag_action = GDK_ACTION_MOVE;
    }

  if (drag_action != 0)
    {
      /* highlight the drop zone */
      itembar = gtk_bin_get_child (GTK_BIN (window));
      application->drop_index = bar_itembar_get_drop_index (BAR_ITEMBAR (itembar), x, y);
      bar_itembar_set_drop_highlight_item (BAR_ITEMBAR (itembar), application->drop_index);
    }

not_a_drop_zone:
  gdk_drag_status (context, drag_action, drag_time);

  return (drag_action == 0);
}



static gboolean
bar_application_drag_drop (GtkWidget        *window,
                             GdkDragContext   *context,
                             gint              x,
                             gint              y,
                             guint             drag_time,
                             BarApplication *application)
{
  GdkAtom target;

  bar_return_val_if_fail (BAR_IS_WINDOW (window), FALSE);
  bar_return_val_if_fail (GDK_IS_DRAG_CONTEXT (context), FALSE);
  bar_return_val_if_fail (BAR_IS_APPLICATION (application), FALSE);

  target = gtk_drag_dest_find_target (window, context, NULL);

  /* we cannot handle the drop */
  if (G_UNLIKELY (target == GDK_NONE))
    return FALSE;

  /* set state so the drag-data-received handler
   * knows that this is really a drop this time. */
  application->drop_occurred = TRUE;

  /* request the drag data from the source. */
  gtk_drag_get_data (window, context, target, drag_time);

  /* we call gtk_drag_finish() later */
  return TRUE;
}



static void
bar_application_drag_leave (GtkWidget        *window,
                              GdkDragContext   *context,
                              guint             drag_time,
                              BarApplication *application)
{
  GtkWidget *itembar;

  bar_return_if_fail (BAR_IS_WINDOW (window));
  bar_return_if_fail (GDK_IS_DRAG_CONTEXT (context));
  bar_return_if_fail (BAR_IS_APPLICATION (application));

  /* reset the state */
  application->drop_data_ready = FALSE;
  application->drop_desktop_files = FALSE;
  application->drop_occurred = FALSE;

  /* unset the highlight position */
  itembar = gtk_bin_get_child (GTK_BIN (window));
  bar_itembar_set_drop_highlight_item (BAR_ITEMBAR (itembar), -1);
}



static gboolean
bar_application_window_id_exists (BarApplication *application,
                                    gint              id)
{
  GSList *li;

  for (li = application->windows; li != NULL; li = li->next)
    if (bar_window_get_id (li->data) == id)
      return TRUE;

  return FALSE;
}



BarApplication *
bar_application_get (void)
{
  static BarApplication *application = NULL;

  if (G_LIKELY (application))
    {
      g_object_ref (G_OBJECT (application));
    }
  else
    {
      application = g_object_new (BAR_TYPE_APPLICATION, NULL);
      g_object_add_weak_pointer (G_OBJECT (application), (gpointer) &application);
    }

  return application;
}



void
bar_application_load (BarApplication  *application,
                        gboolean           disable_wm_check)
{
#ifdef GDK_WINDOWING_X11
  WaitForWM  *wfwm;
  guint       i;
  gchar     **atom_names;

  if (!disable_wm_check)
    {
      /* setup data for wm checking */
      wfwm = g_slice_new0 (WaitForWM);
      wfwm->application = application;
      wfwm->dpy = XOpenDisplay (NULL);
      wfwm->have_wm = FALSE;
      wfwm->counter = 0;

      /* preload wm atoms for all screens */
      wfwm->atom_count = XScreenCount (wfwm->dpy);
      wfwm->atoms = g_new (Atom, wfwm->atom_count);
      atom_names = g_new0 (gchar *, wfwm->atom_count + 1);

      for (i = 0; i < wfwm->atom_count; i++)
        atom_names[i] = g_strdup_printf ("WM_S%d", i);

      if (!XInternAtoms (wfwm->dpy, atom_names, wfwm->atom_count, False, wfwm->atoms))
        wfwm->atom_count = 0;

      g_strfreev (atom_names);

      /* setup timeout to check for a window manager */
      application->wait_for_wm_timeout_id =
          g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, 50, bar_application_wait_for_window_manager,
                              wfwm, bar_application_wait_for_window_manager_destroyed);
    }
  else
    {
      /* directly launch */
      bar_application_load_real (application);
    }
#else
  /* directly launch */
  bar_application_load_real (application);
#endif
}



void
bar_application_save (BarApplication *application,
                        BarSaveTypes    save_types)
{
  GSList        *li;
  BlconfChannel *channel = application->blconf;
  GValue        *value;
  GPtrArray     *bars = NULL;
  gint           bar_id;

  bar_return_if_fail (BAR_IS_APPLICATION (application));
  bar_return_if_fail (BLCONF_IS_CHANNEL (channel));

  /* leave if the whole application is locked */
  if (blconf_channel_is_property_locked (channel, "/bars"))
    return;

  if (BAR_HAS_FLAG (save_types, SAVE_BAR_IDS))
    bars = g_ptr_array_new ();

  for (li = application->windows; li != NULL; li = li->next)
    {
      if (bars != NULL)
        {
          /* store the bar id */
          value = g_new0 (GValue, 1);
          bar_id = bar_window_get_id (li->data);
          g_value_init (value, G_TYPE_INT);
          g_value_set_int (value, bar_id);
          g_ptr_array_add (bars, value);
        }

      /* save the bar settings */
      bar_application_save_window (application, li->data, save_types);
    }

  if (bars != NULL)
    {
      /* store the bar ids */
      if (!blconf_channel_set_arrayv (channel, "/bars", bars))
        g_warning ("Failed to store the number of bars");
      blconf_array_free (bars);
    }
}



void
bar_application_save_window (BarApplication *application,
                               BarWindow      *window,
                               BarSaveTypes    save_types)
{
  GList                   *children, *lp;
  GtkWidget               *itembar;
  BladeBarPluginProvider *provider;
  gchar                    buf[50];
  BlconfChannel           *channel = application->blconf;
  GPtrArray               *array = NULL;
  GValue                  *value;
  gint                     plugin_id;
  gint                     bar_id;

  bar_return_if_fail (BAR_IS_APPLICATION (application));
  bar_return_if_fail (BAR_IS_WINDOW (window));

  /* skip this window if it is locked */
  if (bar_window_get_locked (window)
      || !BAR_HAS_FLAG (save_types, SAVE_PLUGIN_IDS | SAVE_PLUGIN_PROVIDERS))
    return;

  bar_id = bar_window_get_id (window);
  bar_debug (BAR_DEBUG_APPLICATION,
               "saving /bars/bar-%d: ids=%s, providers=%s",
               bar_id,
               BAR_DEBUG_BOOL (BAR_HAS_FLAG (save_types, SAVE_PLUGIN_IDS)),
               BAR_DEBUG_BOOL (BAR_HAS_FLAG (save_types, SAVE_PLUGIN_PROVIDERS)));

  /* get the itembar children */
  itembar = gtk_bin_get_child (GTK_BIN (window));
  children = gtk_container_get_children (GTK_CONTAINER (itembar));

  /* only cleanup and continue if there are no children */
  if (BAR_HAS_FLAG (save_types, SAVE_PLUGIN_IDS))
    {
      if (G_UNLIKELY (children == NULL))
        {
          g_snprintf (buf, sizeof (buf), "/bars/bar-%d/plugin-ids", bar_id);
          if (blconf_channel_has_property (channel, buf))
            blconf_channel_reset_property (channel, buf, FALSE);
          return;
        }

      array = g_ptr_array_new ();
    }

  /* walk all the plugin children */
  for (lp = children; lp != NULL; lp = lp->next)
    {
      provider = BLADE_BAR_PLUGIN_PROVIDER (lp->data);

      if (array != NULL)
        {
          plugin_id = blade_bar_plugin_provider_get_unique_id (provider);

          /* add plugin id to the array */
          value = g_new0 (GValue, 1);
          g_value_init (value, G_TYPE_INT);
          g_value_set_int (value, plugin_id);
          g_ptr_array_add (array, value);

          /* make sure the plugin type-name is store in the plugin item */
          g_snprintf (buf, sizeof (buf), "/plugins/plugin-%d", plugin_id);
          blconf_channel_set_string (channel, buf, blade_bar_plugin_provider_get_name (provider));
        }

      /* ask the plugin to save */
      if (BAR_HAS_FLAG (save_types, SAVE_PLUGIN_PROVIDERS))
        blade_bar_plugin_provider_save (provider);
    }

  if (array != NULL)
    {
      /* store the plugin ids for this bar */
      g_snprintf (buf, sizeof (buf), "/bars/bar-%d/plugin-ids", bar_id);
      blconf_channel_set_arrayv (channel, buf, array);
      blconf_array_free (array);
    }

  g_list_free (children);
}



void
bar_application_take_dialog (BarApplication *application,
                               GtkWindow        *dialog)
{
  bar_return_if_fail (BAR_IS_APPLICATION (application));
  bar_return_if_fail (GTK_IS_WINDOW (dialog));

  /* block autohide if this will be the first dialog */
  if (application->dialogs == NULL)
    bar_application_windows_blocked (application, TRUE);

  /* monitor window destruction */
  g_signal_connect (G_OBJECT (dialog), "destroy",
      G_CALLBACK (bar_application_dialog_destroyed), application);
  application->dialogs = g_slist_prepend (application->dialogs, dialog);
}



void
bar_application_destroy_dialogs (BarApplication *application)
{
  GSList *li, *lnext;

  bar_return_if_fail (BAR_IS_APPLICATION (application));

  /* destroy all dialogs */
  for (li = application->dialogs; li != NULL; li = lnext)
    {
      lnext = li->next;
      gtk_widget_destroy (GTK_WIDGET (li->data));
    }

  bar_return_if_fail (application->dialogs == NULL);
}



void
bar_application_add_new_item (BarApplication  *application,
                                BarWindow       *window,
                                const gchar       *plugin_name,
                                gchar            **arguments)
{
  gint bar_id;

  bar_return_if_fail (BAR_IS_APPLICATION (application));
  bar_return_if_fail (plugin_name != NULL);
  bar_return_if_fail (application->windows != NULL);
  bar_return_if_fail (window == NULL || BAR_IS_WINDOW (window));

  /* leave if the config is locked */
  if (bar_application_get_locked (application))
    return;

  if (bar_module_factory_has_module (application->factory, plugin_name))
    {
      if (window == NULL)
        {
          /* find a suitable bar if there are 2 or more bar */
          if (LIST_HAS_TWO_OR_MORE_ENTRIES (application->windows))
            {
              /* ask the user to select a bar */
              bar_id = bar_dialogs_choose_bar (application);
              if (bar_id == -1)
                {
                  /* cancel was clicked */
                  return;
                }
              else
                {
                  /* get bar from the id */
                  window = bar_application_get_window (application, bar_id);
                }
            }
          else
            {
              /* get the first (and only) window */
              window = g_slist_nth_data (application->windows, 0);
            }
        }

      if (window != NULL && !bar_window_get_locked (window))
        {
          /* insert plugin at the end of the bar */
          if (bar_application_plugin_insert (application, window,
                                               plugin_name, -1,
                                               arguments, -1))
            {
              /* save the new plugin ids */
              bar_application_save_window (application, window, SAVE_PLUGIN_IDS);
            }
        }
    }
  else
    {
      g_warning ("The plugin \"%s\" you want to add is not "
                 "known by the bar", plugin_name);
    }
}



BarWindow *
bar_application_new_window (BarApplication *application,
                              GdkScreen        *screen,
                              gint              bar_id,
                              gboolean          new_window)
{
  GtkWidget          *window;
  GtkWidget          *itembar;
  gchar              *property;
  gint                idx;
  static const gchar *props[] = { "mode", "size", "nrows" };
  guint               i;
  gchar              *position;
  static gint         unqiue_id_counter = 1;
  GtkWindowGroup     *window_group;

  bar_return_val_if_fail (BAR_IS_APPLICATION (application), NULL);
  bar_return_val_if_fail (screen == NULL || GDK_IS_SCREEN (screen), NULL);
  bar_return_val_if_fail (BLCONF_IS_CHANNEL (application->blconf), NULL);
  bar_return_val_if_fail (new_window || !bar_application_window_id_exists (application, bar_id), NULL);

  if (new_window)
    {
      /* get a new unique id */
      bar_id = unqiue_id_counter;
      while (bar_application_window_id_exists (application, bar_id))
        bar_id = ++unqiue_id_counter;
    }

  /* create bar window */
  window = bar_window_new (screen, bar_id);

  /* put the window in its own group */
  window_group = gtk_window_group_new ();
  gtk_window_group_add_window (window_group, GTK_WINDOW (window));
  g_object_weak_ref (G_OBJECT (window), (GWeakNotify) g_object_unref, window_group);

  /* add the window to internal list */
  application->windows = g_slist_append (application->windows, window);

  if (new_window)
    {
      /* remove the old blconf properties to be sure */
      property = g_strdup_printf ("/bars/bar-%d", bar_id);
      blconf_channel_reset_property (application->blconf, property, TRUE);
      g_free (property);
    }

  /* add the itembar */
  itembar = bar_itembar_new ();
  for (i = 0; i < G_N_ELEMENTS (props); i++)
    blxo_binding_new (G_OBJECT (window), props[i], G_OBJECT (itembar), props[i]);
  gtk_container_add (GTK_CONTAINER (window), itembar);
  gtk_widget_show (itembar);

  /* set the itembar drag destination targets */
  gtk_drag_dest_set (GTK_WIDGET (window), 0,
                     drop_targets, G_N_ELEMENTS (drop_targets),
                     GDK_ACTION_COPY | GDK_ACTION_MOVE);

  /* signals for drag and drop */
  g_signal_connect (G_OBJECT (window), "drag-data-received",
                    G_CALLBACK (bar_application_drag_data_received), itembar);
  g_signal_connect (G_OBJECT (window), "drag-motion",
                    G_CALLBACK (bar_application_drag_motion), application);
  g_signal_connect (G_OBJECT (window), "drag-drop",
                    G_CALLBACK (bar_application_drag_drop), application);
  g_signal_connect (G_OBJECT (window), "drag-leave",
                    G_CALLBACK (bar_application_drag_leave), application);

  /* add the blconf bindings */
  bar_application_blconf_window_bindings (application, BAR_WINDOW (window), FALSE);

  /* make sure the bar has a valid position, else it is not visible */
  if (!bar_window_has_position (BAR_WINDOW (window)))
    {
      if (!new_window)
        g_message ("No bar position set, restoring default");

      /* create a position so not all bars overlap */
      idx = g_slist_index (application->windows, window);
      position = g_strdup_printf ("p=0;x=100;y=%d", 30 + (idx * (48 + 10)));
      g_object_set (G_OBJECT (window), "position", position, NULL);
      g_free (position);
    }

  /* save the new bar layout */
  if (new_window)
    bar_application_save (application, SAVE_BAR_IDS);

  return BAR_WINDOW (window);
}



void
bar_application_remove_window (BarApplication *application,
                                 BarWindow      *window)
{
  gchar     *property;
  GtkWidget *itembar;
  gint       bar_id;

  bar_return_if_fail (BAR_IS_WINDOW (window));
  bar_return_if_fail (BAR_IS_APPLICATION (application));
  bar_return_if_fail (g_slist_find (application->windows, window) != NULL);

  /* leave if the application or window is locked */
  if (bar_application_get_locked (application)
      || bar_window_get_locked (BAR_WINDOW (window)))
    return;

  bar_id = bar_window_get_id (BAR_WINDOW (window));
  bar_debug (BAR_DEBUG_APPLICATION,
               "removing configuration and plugins of bar %d",
               bar_id);

  /* remove from the internal list */
  application->windows = g_slist_remove (application->windows, window);

  /* disconnect bindings from this bar */
  bar_properties_unbind (G_OBJECT (window));

  /* set all the plugins on this bar the remove signal */
  itembar = gtk_bin_get_child (GTK_BIN (window));
  gtk_container_foreach (GTK_CONTAINER (itembar),
      bar_application_plugin_remove, NULL);

  /* destroy */
  gtk_widget_destroy (GTK_WIDGET (window));

  /* remove the bar settings */
  property = g_strdup_printf ("/bars/bar-%d", bar_id);
  blconf_channel_reset_property (application->blconf, property, TRUE);
  g_free (property);

  /* save changed bar ids */
  bar_application_save (application, SAVE_BAR_IDS);

  /* quit if there are no windows */
  /* TODO, allow removing all windows and ask user what to do */
  if (application->windows == NULL)
    gtk_main_quit ();
}



GSList *
bar_application_get_windows (BarApplication *application)
{
  bar_return_val_if_fail (BAR_IS_APPLICATION (application), NULL);
  return application->windows;
}



BarWindow *
bar_application_get_window (BarApplication  *application,
                              gint               bar_id)
{
  GSList *li;

  bar_return_val_if_fail (BAR_IS_APPLICATION (application), NULL);

  for (li = application->windows; li != NULL; li = li->next)
    if (bar_window_get_id (li->data) == bar_id)
      return li->data;

  return NULL;
}



void
bar_application_window_select (BarApplication *application,
                                 BarWindow      *window)
{
  GSList *li;

  bar_return_if_fail (BAR_IS_APPLICATION (application));

  /* update state for all windows */
  for (li = application->windows; li != NULL; li = li->next)
    g_object_set (G_OBJECT (li->data), "active", window == li->data, NULL);
}



void
bar_application_windows_blocked (BarApplication *application,
                                   gboolean          blocked)
{
  GSList *li;

  bar_return_if_fail (BAR_IS_APPLICATION (application));

  /* walk the windows */
  for (li = application->windows; li != NULL; li = li->next)
    {
      /* block autohide for all windows */
      if (blocked)
        bar_window_freeze_autohide (BAR_WINDOW (li->data));
      else
        bar_window_thaw_autohide (BAR_WINDOW (li->data));
    }
}



gboolean
bar_application_get_locked (BarApplication *application)
{
  GSList *li;

  bar_return_val_if_fail (BAR_IS_APPLICATION (application), TRUE);
  bar_return_val_if_fail (BLCONF_IS_CHANNEL (application->blconf), TRUE);

  /* don't even look for the individual window if the
   * entire channel is locked */
  if (blconf_channel_is_property_locked (application->blconf, "/"))
    return TRUE;

  /* if one of the windows is not locked, the user can still modify
   * some settings, so then we return %FALSE */
  for (li = application->windows; li != NULL; li = li->next)
    if (!bar_window_get_locked (li->data))
      return FALSE;

  /* TODO we could extend this to a plugin basis (ie. bars are
   * locked but maybe not all the plugins) */
  return TRUE;
}



void
bar_application_logout (void)
{
  XfceSMClient *sm_client;
  GError       *error = NULL;
  const gchar  *command = "xfce4-session-logout";

  /* first try to session client to logout else fallback and spawn xfce4-session-logout */
  sm_client = xfce_sm_client_get ();
  if (xfce_sm_client_is_connected (sm_client))
    {
      xfce_sm_client_request_shutdown (sm_client, XFCE_SM_CLIENT_SHUTDOWN_HINT_ASK);

      return;
    }
  else if (g_getenv ("SESSION_MANAGER") == NULL)
    {
      if (xfce_dialog_confirm (NULL, GTK_STOCK_QUIT, NULL,
          _("You have started X without session manager. Clicking Quit will close the X server."),
          _("Are you sure you want to quit the bar?")))
        command = "blade-bar --quit";
      else
        return;
    }

  if (!g_spawn_command_line_async (command, &error))
    {
      xfce_dialog_show_error (NULL, error, _("Failed to execute command \"%s\""),
                              command);
      g_error_free (error);
    }
}
