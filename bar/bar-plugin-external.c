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

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include <blxo/blxo.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <libbladeutil/libbladeutil.h>

#include <dbus/dbus-glib.h>

#include <common/bar-private.h>
#include <common/bar-dbus.h>
#include <common/bar-debug.h>

#include <libbladebar/libbladebar.h>
#include <libbladebar/blade-bar-plugin-provider.h>

#include <bar/bar-module.h>
#include <bar/bar-plugin-external.h>
#include <bar/bar-plugin-external-46.h>
#include <bar/bar-window.h>
#include <bar/bar-dialogs.h>



static void         bar_plugin_external_provider_init           (BladeBarPluginProviderInterface *iface);
static void         bar_plugin_external_finalize                (GObject                          *object);
static void         bar_plugin_external_get_property            (GObject                          *object,
                                                                   guint                             prop_id,
                                                                   GValue                           *value,
                                                                   GParamSpec                       *pspec);
static void         bar_plugin_external_set_property            (GObject                          *object,
                                                                   guint                             prop_id,
                                                                   const GValue                     *value,
                                                                   GParamSpec                       *pspec);
static void         bar_plugin_external_realize                 (GtkWidget                        *widget);
static void         bar_plugin_external_unrealize               (GtkWidget                        *widget);
static void         bar_plugin_external_plug_added              (GtkSocket                        *socket);
static gboolean     bar_plugin_external_plug_removed            (GtkSocket                        *socket);
static gboolean     bar_plugin_external_child_ask_restart       (BarPluginExternal              *external);
static void         bar_plugin_external_child_spawn             (BarPluginExternal              *external);
static void         bar_plugin_external_child_respawn_schedule  (BarPluginExternal              *external);
static void         bar_plugin_external_child_watch             (GPid                              pid,
                                                                   gint                              status,
                                                                   gpointer                          user_data);
static void         bar_plugin_external_child_watch_destroyed   (gpointer                          user_data);
static void         bar_plugin_external_queue_free              (BarPluginExternal              *external);
static void         bar_plugin_external_queue_send_to_child     (BarPluginExternal              *external);
static void         bar_plugin_external_queue_add               (BarPluginExternal              *external,
                                                                   BladeBarPluginProviderPropType   type,
                                                                   const GValue                     *value);
static void         bar_plugin_external_queue_add_action        (BarPluginExternal              *external,
                                                                   BladeBarPluginProviderPropType   type);
static const gchar *bar_plugin_external_get_name                (BladeBarPluginProvider          *provider);
static gint         bar_plugin_external_get_unique_id           (BladeBarPluginProvider          *provider);
static void         bar_plugin_external_set_size                (BladeBarPluginProvider          *provider,
                                                                   gint                              size);
static void         bar_plugin_external_set_mode                (BladeBarPluginProvider          *provider,
                                                                   BladeBarPluginMode               mode);
static void         bar_plugin_external_set_nrows               (BladeBarPluginProvider          *provider,
                                                                   guint                             rows);
static void         bar_plugin_external_set_screen_position     (BladeBarPluginProvider          *provider,
                                                                   XfceScreenPosition                screen_position);
static void         bar_plugin_external_save                    (BladeBarPluginProvider          *provider);
static gboolean     bar_plugin_external_get_show_configure      (BladeBarPluginProvider          *provider);
static void         bar_plugin_external_show_configure          (BladeBarPluginProvider          *provider);
static gboolean     bar_plugin_external_get_show_about          (BladeBarPluginProvider          *provider);
static void         bar_plugin_external_show_about              (BladeBarPluginProvider          *provider);
static void         bar_plugin_external_removed                 (BladeBarPluginProvider          *provider);
static gboolean     bar_plugin_external_remote_event            (BladeBarPluginProvider          *provider,
                                                                   const gchar                      *name,
                                                                   const GValue                     *value,
                                                                   guint                            *handler_id);
static void         bar_plugin_external_set_locked              (BladeBarPluginProvider          *provider,
                                                                   gboolean                          locked);
static void         bar_plugin_external_ask_remove              (BladeBarPluginProvider          *provider);
static void         bar_plugin_external_set_sensitive           (BarPluginExternal              *external);



struct _BarPluginExternalPrivate
{
  /* startup arguments */
  gchar     **arguments;

  guint       embedded : 1;

  /* dbus message queue */
  GSList     *queue;

  /* auto restart timer */
  GTimer     *restart_timer;

  /* child watch data */
  GPid        pid;
  guint       watch_id;

  /* delayed spawning */
  guint       spawn_timeout_id;
};

enum
{
  PROP_0,
  PROP_MODULE,
  PROP_UNIQUE_ID,
  PROP_ARGUMENTS
};



G_DEFINE_ABSTRACT_TYPE_WITH_CODE (BarPluginExternal, bar_plugin_external, GTK_TYPE_SOCKET,
  G_IMPLEMENT_INTERFACE (XFCE_TYPE_BAR_PLUGIN_PROVIDER, bar_plugin_external_provider_init))



static void
bar_plugin_external_class_init (BarPluginExternalClass *klass)
{
  GObjectClass   *gobject_class;
  GtkWidgetClass *gtkwidget_class;
  GtkSocketClass *gtksocket_class;

  g_type_class_add_private (klass, sizeof (BarPluginExternalPrivate));

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = bar_plugin_external_finalize;
  gobject_class->set_property = bar_plugin_external_set_property;
  gobject_class->get_property = bar_plugin_external_get_property;

  gtkwidget_class = GTK_WIDGET_CLASS (klass);
  gtkwidget_class->realize = bar_plugin_external_realize;
  gtkwidget_class->unrealize = bar_plugin_external_unrealize;

  gtksocket_class = GTK_SOCKET_CLASS (klass);
  gtksocket_class->plug_added = bar_plugin_external_plug_added;
  gtksocket_class->plug_removed = bar_plugin_external_plug_removed;

  g_object_class_install_property (gobject_class,
                                   PROP_UNIQUE_ID,
                                   g_param_spec_int ("unique-id",
                                                     NULL, NULL,
                                                     -1, G_MAXINT, -1,
                                                     BLXO_PARAM_READWRITE
                                                     | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (gobject_class,
                                   PROP_MODULE,
                                   g_param_spec_object ("module",
                                                        NULL, NULL,
                                                        BAR_TYPE_MODULE,
                                                        BLXO_PARAM_READWRITE
                                                        | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (gobject_class,
                                   PROP_ARGUMENTS,
                                   g_param_spec_boxed ("arguments",
                                                       NULL, NULL,
                                                       G_TYPE_STRV,
                                                       BLXO_PARAM_READWRITE
                                                       | G_PARAM_CONSTRUCT_ONLY));
}



static void
bar_plugin_external_init (BarPluginExternal *external)
{
  external->priv = G_TYPE_INSTANCE_GET_PRIVATE (external, BAR_TYPE_PLUGIN_EXTERNAL, BarPluginExternalPrivate);

  external->module = NULL;
  external->show_configure = FALSE;
  external->show_about = FALSE;
  external->unique_id = -1;

  external->priv->arguments = NULL;
  external->priv->queue = NULL;
  external->priv->restart_timer = NULL;
  external->priv->embedded = FALSE;
  external->priv->pid = 0;
  external->priv->spawn_timeout_id = 0;

  /* signal to pass gtk_widget_set_sensitive() changes to the remote window */
  g_signal_connect (G_OBJECT (external), "notify::sensitive",
      G_CALLBACK (bar_plugin_external_set_sensitive), NULL);
}



static void
bar_plugin_external_provider_init (BladeBarPluginProviderInterface *iface)
{
  iface->get_name = bar_plugin_external_get_name;
  iface->get_unique_id = bar_plugin_external_get_unique_id;
  iface->set_size = bar_plugin_external_set_size;
  iface->set_mode = bar_plugin_external_set_mode;
  iface->set_nrows = bar_plugin_external_set_nrows;
  iface->set_screen_position = bar_plugin_external_set_screen_position;
  iface->save = bar_plugin_external_save;
  iface->get_show_configure = bar_plugin_external_get_show_configure;
  iface->show_configure = bar_plugin_external_show_configure;
  iface->get_show_about = bar_plugin_external_get_show_about;
  iface->show_about = bar_plugin_external_show_about;
  iface->removed = bar_plugin_external_removed;
  iface->remote_event = bar_plugin_external_remote_event;
  iface->set_locked = bar_plugin_external_set_locked;
  iface->ask_remove = bar_plugin_external_ask_remove;
}



static void
bar_plugin_external_finalize (GObject *object)
{
  BarPluginExternal *external = BAR_PLUGIN_EXTERNAL (object);

  if (external->priv->spawn_timeout_id != 0)
    g_source_remove (external->priv->spawn_timeout_id);

  if (external->priv->watch_id != 0)
    {
      /* remove the child watch and don't leave zombies */
      g_source_remove (external->priv->watch_id);
      g_child_watch_add (external->priv->pid, (GChildWatchFunc) g_spawn_close_pid, NULL);
    }

  bar_plugin_external_queue_free (external);

  g_strfreev (external->priv->arguments);

  if (external->priv->restart_timer != NULL)
    g_timer_destroy (external->priv->restart_timer);

  g_object_unref (G_OBJECT (external->module));

  (*G_OBJECT_CLASS (bar_plugin_external_parent_class)->finalize) (object);
}



static void
bar_plugin_external_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  BarPluginExternal *external = BAR_PLUGIN_EXTERNAL (object);

  switch (prop_id)
    {
    case PROP_UNIQUE_ID:
      g_value_set_int (value, external->unique_id);
      break;

    case PROP_ARGUMENTS:
      g_value_set_boxed (value, external->priv->arguments);
      break;

    case PROP_MODULE:
      g_value_set_object (value, external->module);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
bar_plugin_external_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  BarPluginExternal *external = BAR_PLUGIN_EXTERNAL (object);

  switch (prop_id)
    {
    case PROP_UNIQUE_ID:
      external->unique_id = g_value_get_int (value);
      break;

    case PROP_ARGUMENTS:
      external->priv->arguments = g_value_dup_boxed (value);
      break;

    case PROP_MODULE:
      external->module = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
bar_plugin_external_realize (GtkWidget *widget)
{
  BarPluginExternal *external = BAR_PLUGIN_EXTERNAL (widget);

  /* realize the socket first */
  (*GTK_WIDGET_CLASS (bar_plugin_external_parent_class)->realize) (widget);

  if (external->priv->pid == 0)
    {
      if (external->priv->spawn_timeout_id != 0)
        g_source_remove (external->priv->spawn_timeout_id);

      bar_plugin_external_child_spawn (external);
    }
  else
    {
      /* the child was asked to quit during unrealize and there is
       * still an pid, so wait for the child to quit and then
       * spawn it again */
      bar_plugin_external_child_respawn_schedule (external);
    }
}



static void
bar_plugin_external_unrealize (GtkWidget *widget)
{
  BarPluginExternal *external = BAR_PLUGIN_EXTERNAL (widget);

  /* ask the child to quit */
  if (external->priv->pid != 0)
    {
      if (external->priv->embedded)
        bar_plugin_external_queue_add_action (external, PROVIDER_PROP_TYPE_ACTION_QUIT);
      else
        kill (external->priv->pid, SIGTERM);
    }

  bar_debug (BAR_DEBUG_EXTERNAL,
               "%s-%d: plugin unrealized; quiting child",
               bar_module_get_name (external->module),
               external->unique_id);

  (*GTK_WIDGET_CLASS (bar_plugin_external_parent_class)->unrealize) (widget);
}



static void
bar_plugin_external_plug_added (GtkSocket *socket)
{
  BarPluginExternal *external = BAR_PLUGIN_EXTERNAL (socket);

  external->priv->embedded = TRUE;

  bar_debug (BAR_DEBUG_EXTERNAL,
               "%s-%d: child is embedded; %d properties in queue",
               bar_module_get_name (external->module),
               external->unique_id,
               g_slist_length (external->priv->queue));

  /* send queue to wrapper */
  bar_plugin_external_queue_send_to_child (external);
}



static gboolean
bar_plugin_external_plug_removed (GtkSocket *socket)
{
  BarPluginExternal *external = BAR_PLUGIN_EXTERNAL (socket);

  external->priv->embedded = FALSE;

  bar_debug (BAR_DEBUG_EXTERNAL,
               "%s-%d: child is unembedded",
               bar_module_get_name (external->module),
               external->unique_id);

  return TRUE;
}



static gboolean
bar_plugin_external_child_ask_restart_dialog (GtkWindow   *parent,
                                                const gchar *plugin_name)
{
  GtkWidget *dialog;
  gint       response;

  bar_return_val_if_fail (parent == NULL || GTK_IS_WINDOW (parent), FALSE);
  bar_return_val_if_fail (plugin_name != NULL, FALSE);

  dialog = gtk_message_dialog_new (parent,
                                   GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
                                   _("Plugin \"%s\" unexpectedly left the bar, do you want to restart it?"),
                                   plugin_name);
  gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), _("The plugin restarted more than once in "
                                            "the last %d seconds. If you press Execute the bar will try to restart "
                                            "the plugin otherwise it will be permanently removed from the bar."),
                                            BAR_PLUGIN_AUTO_RESTART);
  gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_EXECUTE, GTK_RESPONSE_OK,
                          GTK_STOCK_REMOVE, GTK_RESPONSE_CLOSE, NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  return (response == GTK_RESPONSE_OK);
}



static gboolean
bar_plugin_external_child_ask_restart (BarPluginExternal *external)
{
  GtkWidget *toplevel;

  bar_return_val_if_fail (BAR_IS_PLUGIN_EXTERNAL (external), FALSE);

  toplevel = gtk_widget_get_toplevel (GTK_WIDGET (external));
  bar_return_val_if_fail (BAR_IS_WINDOW (toplevel), FALSE);

  if (external->priv->restart_timer == NULL
      || g_timer_elapsed (external->priv->restart_timer, NULL) > BAR_PLUGIN_AUTO_RESTART)
    {
      g_message ("Plugin %s-%d has been automatically restarted after crash.",
                 bar_module_get_name (external->module),
                 external->unique_id);
    }
  else if (!bar_plugin_external_child_ask_restart_dialog (GTK_WINDOW (toplevel),
               bar_module_get_display_name (external->module)))
    {
      if (external->priv->watch_id != 0)
        {
          /* remove the child watch and don't leave zombies */
          g_source_remove (external->priv->watch_id);
          g_child_watch_add (external->priv->pid, (GChildWatchFunc) g_spawn_close_pid, NULL);
          external->priv->watch_id = 0;
        }

      /* cleanup the plugin configuration (in BarApplication) and
       * destroy the plugin */
      blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (external),
                                              PROVIDER_SIGNAL_REMOVE_PLUGIN);

      return FALSE;
    }

  /* create or reset the restart timer */
  if (external->priv->restart_timer == NULL)
    external->priv->restart_timer = g_timer_new ();
  else
    g_timer_reset (external->priv->restart_timer);

  return TRUE;
}



static void
bar_plugin_external_child_spawn_child_setup (gpointer data)
{
  BarPluginExternal *external = BAR_PLUGIN_EXTERNAL (data);
  GdkScreen           *screen;
  gchar               *name;

  /* this is what gdk_spawn_on_screen does */
  screen = gtk_widget_get_screen (GTK_WIDGET (external));
  name = gdk_screen_make_display_name (screen);
  g_setenv ("DISPLAY", name, TRUE);
  g_free (name);
}



static void
bar_plugin_external_child_spawn (BarPluginExternal *external)
{
  gchar        **argv, **dbg_argv, **tmp_argv;
  GError        *error = NULL;
  gboolean       succeed;
  GPid           pid;
  gchar         *program, *cmd_line;
  guint          i;
  gint           tmp_argc;
  GTimeVal       timestamp;

  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (external));
  bar_return_if_fail (GTK_WIDGET_REALIZED (external));

  /* set plugin specific arguments */
  argv = (*BAR_PLUGIN_EXTERNAL_GET_CLASS (external)->get_argv) (external, external->priv->arguments);
  bar_return_if_fail (argv != NULL);

  /* check debugging state */
  if (bar_debug_has_domain (BAR_DEBUG_GDB)
      || bar_debug_has_domain (BAR_DEBUG_VALGRIND))
    {
      g_get_current_time (&timestamp);
      cmd_line = NULL;
      program = NULL;

      /* note that if the program was not found in PATH, we already warned
       * for it in bar_debug_notify_proxy, so no need to do that again */
      if (bar_debug_has_domain (BAR_DEBUG_GDB))
        {
          program = g_find_program_in_path ("gdb");
          if (G_LIKELY (program != NULL))
            {
              cmd_line = g_strdup_printf ("%s -batch "
                                          "-ex 'set logging file %s" G_DIR_SEPARATOR_S "%li_gdb_%s_%s.log' "
                                          "-ex 'set logging on' "
                                          "-ex 'set pagination off' "
                                          "-ex 'set logging redirect on' "
                                          "-ex 'run' "
                                          "-ex 'backtrace full' "
                                          "-ex 'info registers' "
                                          "-args",
                                          program, g_get_tmp_dir (), timestamp.tv_sec,
                                          bar_module_get_name (external->module),
                                          argv[PLUGIN_ARGV_UNIQUE_ID]);
            }
        }
      else if (bar_debug_has_domain (BAR_DEBUG_VALGRIND))
        {
          program = g_find_program_in_path ("valgrind");
          if (G_LIKELY (program != NULL))
            {
              cmd_line = g_strdup_printf ("%s "
                                          "--log-file='%s" G_DIR_SEPARATOR_S "%li_valgrind_%s_%s.log' "
                                          "--leak-check=full --show-reachable=yes -v ",
                                          program, g_get_tmp_dir (), timestamp.tv_sec,
                                          bar_module_get_name (external->module),
                                          argv[PLUGIN_ARGV_UNIQUE_ID]);
            }
        }

      if (cmd_line != NULL
          && g_shell_parse_argv (cmd_line, &tmp_argc, &tmp_argv, &error))
        {
          dbg_argv = g_new0 (gchar *, tmp_argc + g_strv_length (argv) + 1);

          for (i = 0; tmp_argv[i] != NULL; i++)
            dbg_argv[i] = tmp_argv[i];
          g_free (tmp_argv);

          for (i = 0; argv[i] != NULL; i++)
            dbg_argv[i + tmp_argc] = argv[i];
          g_free (argv);

          argv = dbg_argv;
        }
      else
        {
          bar_debug (BAR_DEBUG_EXTERNAL,
                       "%s-%d: Failed to run the plugin in %s: %s",
                       bar_module_get_name (external->module),
                       external->unique_id, program,
                       cmd_line != NULL ? error->message : "debugger not found");
          g_error_free (error);

          return;
        }

      g_free (program);
      g_free (cmd_line);
    }

  /* spawn the proccess */
  succeed = g_spawn_async (NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD,
                           bar_plugin_external_child_spawn_child_setup,
                           external, &pid, &error);

  bar_debug (BAR_DEBUG_EXTERNAL,
               "%s-%d: child spawned; pid=%d, argc=%d",
               bar_module_get_name (external->module),
               external->unique_id, pid, g_strv_length (argv));

  if (G_LIKELY (succeed))
    {
      /* watch the child */
      external->priv->pid = pid;
      external->priv->watch_id = g_child_watch_add_full (G_PRIORITY_LOW, pid,
                                                         bar_plugin_external_child_watch, external,
                                                         bar_plugin_external_child_watch_destroyed);
    }
  else
    {
      g_critical ("Failed to spawn the blade-bar-wrapper: %s", error->message);
      g_error_free (error);
    }

  g_strfreev (argv);
}



static gboolean
bar_plugin_external_child_respawn (gpointer user_data)
{
  BarPluginExternal *external = BAR_PLUGIN_EXTERNAL (user_data);
  GtkWidget           *window;

  bar_return_val_if_fail (BAR_IS_PLUGIN_EXTERNAL (external), FALSE);

  /* abort startup if the plugin is not realized */
  if (!GTK_WIDGET_REALIZED (external))
    return FALSE;

  /* delay startup if the old child is still embedded */
  if (external->priv->embedded
      || external->priv->pid != 0)
    {
      bar_debug (BAR_DEBUG_EXTERNAL,
                   "%s-%d: still a child embedded, respawn delayed",
                   bar_module_get_name (external->module), external->unique_id);

      return TRUE;
    }

  bar_plugin_external_queue_free (external);

  window = gtk_widget_get_toplevel (GTK_WIDGET (external));
  bar_return_val_if_fail (BAR_IS_WINDOW (window), FALSE);
  bar_window_set_povider_info (BAR_WINDOW (window), GTK_WIDGET (external), FALSE);

  bar_plugin_external_child_spawn (external);

  /* stop timeout */
  return FALSE;
}



static void
bar_plugin_external_child_respawn_destroyed (gpointer user_data)
{
  BAR_PLUGIN_EXTERNAL (user_data)->priv->spawn_timeout_id = 0;
}



static void
bar_plugin_external_child_respawn_schedule (BarPluginExternal *external)
{
  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (external));

  if (external->priv->spawn_timeout_id == 0)
    {
      bar_debug (BAR_DEBUG_EXTERNAL,
                   "%s-%d: scheduled a respawn of the child",
                   bar_module_get_name (external->module), external->unique_id);

      /* schedule a restart timeout */
      external->priv->spawn_timeout_id = g_timeout_add_full (G_PRIORITY_LOW, 100, bar_plugin_external_child_respawn,
                                                             external, bar_plugin_external_child_respawn_destroyed);
    }
}



static void
bar_plugin_external_child_watch (GPid     pid,
                                   gint     status,
                                   gpointer user_data)
{
  BarPluginExternal *external = BAR_PLUGIN_EXTERNAL (user_data);
  gboolean             auto_restart = FALSE;

  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (external));
  bar_return_if_fail (external->priv->pid == pid);

  /* reset the pid, it can't be embedded as well */
  external->priv->pid = 0;
  external->priv->embedded = FALSE;

  bar_debug (BAR_DEBUG_EXTERNAL,
               "%s-%d: child exited with status %d",
               bar_module_get_name (external->module),
               external->unique_id, status);

  if (WIFEXITED (status))
    {
      /* extract our return value from the status */
      switch (WEXITSTATUS (status))
        {
        case PLUGIN_EXIT_SUCCESS:
          /* normal exit, do not try to restart */
          goto close_pid;

        case PLUGIN_EXIT_SUCCESS_AND_RESTART:
          /* the bar asked for a restart, so do not bother the user */
          auto_restart = TRUE;
          break;

        case PLUGIN_EXIT_FAILURE:
          /* do nothing, maybe we try to restart */
          break;

        case PLUGIN_EXIT_ARGUMENTS_FAILED:
        case PLUGIN_EXIT_PREINIT_FAILED:
        case PLUGIN_EXIT_CHECK_FAILED:
        case PLUGIN_EXIT_NO_PROVIDER:
          g_warning ("Plugin %s-%d exited with status %d, removing from bar configuration",
                     bar_module_get_name (external->module),
                     external->unique_id, WEXITSTATUS (status));

          /* cleanup the plugin configuration (in BarApplication) */
          blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (external),
                                                  PROVIDER_SIGNAL_REMOVE_PLUGIN);

          /* wait until everything is settled before we destroy */
          blxo_gtk_object_destroy_later (GTK_OBJECT (external));
          goto close_pid;
        }
    }
  else if (WIFSIGNALED (status))
    {
      switch (WTERMSIG (status))
        {
        case SIGUSR1:
          /* the bar asked for a restart, so do not bother the user */
          auto_restart = TRUE;
          break;
        }
    }

  if (GTK_WIDGET_REALIZED (external)
      && (auto_restart || bar_plugin_external_child_ask_restart (external)))
    {
      bar_plugin_external_child_respawn_schedule (external);
    }

close_pid:
  g_spawn_close_pid (pid);
}



static void
bar_plugin_external_child_watch_destroyed (gpointer user_data)
{
  if (BAR_IS_PLUGIN_EXTERNAL (user_data))
    BAR_PLUGIN_EXTERNAL (user_data)->priv->watch_id = 0;
}



static void
bar_plugin_external_queue_free (BarPluginExternal *external)
{
  PluginProperty *property;
  GSList         *li;

  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (external));

  for (li = external->priv->queue; li != NULL; li = li->next)
    {
      property = li->data;
      g_value_unset (&property->value);
      g_slice_free (PluginProperty, property);
    }

  g_slist_free (external->priv->queue);
  external->priv->queue = NULL;
}



static void
bar_plugin_external_queue_send_to_child (BarPluginExternal *external)
{
  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (external));

  if (external->priv->queue != NULL)
    {
      external->priv->queue = g_slist_reverse (external->priv->queue);

      (*BAR_PLUGIN_EXTERNAL_GET_CLASS (external)->set_properties) (external, external->priv->queue);

      bar_plugin_external_queue_free (external);
    }
}



static void
bar_plugin_external_queue_add (BarPluginExternal             *external,
                                 BladeBarPluginProviderPropType  type,
                                 const GValue                    *value)
{
  PluginProperty *prop;

  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (external));
  bar_return_if_fail (G_TYPE_CHECK_VALUE (value));

  prop = g_slice_new0 (PluginProperty);
  prop->type = type;
  g_value_init (&prop->value, G_VALUE_TYPE (value));
  g_value_copy (value, &prop->value);

  external->priv->queue = g_slist_prepend (external->priv->queue, prop);

  if (external->priv->embedded)
    bar_plugin_external_queue_send_to_child (external);
}



static void
bar_plugin_external_queue_add_action (BarPluginExternal             *external,
                                        BladeBarPluginProviderPropType  type)
{
  GValue value = { 0, };

  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (external));

  /* add to queue with noop boolean */
  g_value_init (&value, G_TYPE_BOOLEAN);
  bar_plugin_external_queue_add (external, type, &value);
  g_value_unset (&value);
}



static const gchar *
bar_plugin_external_get_name (BladeBarPluginProvider *provider)
{
  bar_return_val_if_fail (BAR_IS_PLUGIN_EXTERNAL (provider), NULL);
  bar_return_val_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider), NULL);

  return bar_module_get_name (BAR_PLUGIN_EXTERNAL (provider)->module);
}



static gint
bar_plugin_external_get_unique_id (BladeBarPluginProvider *provider)
{
  bar_return_val_if_fail (BAR_IS_PLUGIN_EXTERNAL (provider), -1);
  bar_return_val_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider), -1);

  return BAR_PLUGIN_EXTERNAL (provider)->unique_id;
}



static void
bar_plugin_external_set_size (BladeBarPluginProvider *provider,
                                gint                     size)
{
  GValue value = { 0, };

  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (provider));
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, size);

  bar_plugin_external_queue_add (BAR_PLUGIN_EXTERNAL (provider),
                                   PROVIDER_PROP_TYPE_SET_SIZE, &value);

  g_value_unset (&value);
}



static void
bar_plugin_external_set_mode (BladeBarPluginProvider *provider,
                                BladeBarPluginMode      mode)
{
  GValue value = { 0, };

  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (provider));
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, mode);

  bar_plugin_external_queue_add (BAR_PLUGIN_EXTERNAL (provider),
                                   PROVIDER_PROP_TYPE_SET_MODE, &value);

  g_value_unset (&value);
}



static void
bar_plugin_external_set_nrows (BladeBarPluginProvider *provider,
                                 guint                    rows)
{
  GValue value = { 0, };

  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (provider));
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, rows);

  bar_plugin_external_queue_add (BAR_PLUGIN_EXTERNAL (provider),
                                   PROVIDER_PROP_TYPE_SET_NROWS, &value);

  g_value_unset (&value);
}



static void
bar_plugin_external_set_screen_position (BladeBarPluginProvider *provider,
                                           XfceScreenPosition       screen_position)
{
  GValue value = { 0, };

  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (provider));
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  g_value_init (&value, G_TYPE_INT);
  g_value_set_int (&value, screen_position);

  bar_plugin_external_queue_add (BAR_PLUGIN_EXTERNAL (provider),
                                   PROVIDER_PROP_TYPE_SET_SCREEN_POSITION, &value);

  g_value_unset (&value);
}



static void
bar_plugin_external_save (BladeBarPluginProvider *provider)
{
  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (provider));
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  bar_plugin_external_queue_add_action (BAR_PLUGIN_EXTERNAL (provider),
                                          PROVIDER_PROP_TYPE_ACTION_SAVE);
}



static gboolean
bar_plugin_external_get_show_configure (BladeBarPluginProvider *provider)
{
  bar_return_val_if_fail (BAR_IS_PLUGIN_EXTERNAL (provider), FALSE);
  bar_return_val_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider), FALSE);

  return BAR_PLUGIN_EXTERNAL (provider)->show_configure;
}



static void
bar_plugin_external_show_configure (BladeBarPluginProvider *provider)
{
  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (provider));
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  bar_plugin_external_queue_add_action (BAR_PLUGIN_EXTERNAL (provider),
                                          PROVIDER_PROP_TYPE_ACTION_SHOW_CONFIGURE);
}



static gboolean
bar_plugin_external_get_show_about (BladeBarPluginProvider *provider)
{
  bar_return_val_if_fail (BAR_IS_PLUGIN_EXTERNAL (provider), FALSE);
  bar_return_val_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider), FALSE);

  return BAR_PLUGIN_EXTERNAL (provider)->show_about;
}



static void
bar_plugin_external_show_about (BladeBarPluginProvider *provider)
{
  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (provider));
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  bar_plugin_external_queue_add_action (BAR_PLUGIN_EXTERNAL (provider),
                                          PROVIDER_PROP_TYPE_ACTION_SHOW_ABOUT);
}



static void
bar_plugin_external_removed (BladeBarPluginProvider *provider)
{
  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (provider));
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  bar_plugin_external_queue_add_action (BAR_PLUGIN_EXTERNAL (provider),
                                          PROVIDER_PROP_TYPE_ACTION_REMOVED);
}



static gboolean
bar_plugin_external_remote_event (BladeBarPluginProvider *provider,
                                    const gchar             *name,
                                    const GValue            *value,
                                    guint                   *handle)
{
  return (*BAR_PLUGIN_EXTERNAL_GET_CLASS (provider)->remote_event) (BAR_PLUGIN_EXTERNAL (provider),
                                                                      name, value, handle);
}



static void
bar_plugin_external_set_locked (BladeBarPluginProvider *provider,
                                  gboolean                 locked)
{
  GValue value = { 0, };

  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (provider));
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  g_value_init (&value, G_TYPE_BOOLEAN);
  g_value_set_boolean (&value, locked);

  bar_plugin_external_queue_add (BAR_PLUGIN_EXTERNAL (provider),
                                   PROVIDER_PROP_TYPE_SET_LOCKED, &value);

  g_value_unset (&value);
}



static void
bar_plugin_external_ask_remove (BladeBarPluginProvider *provider)
{
  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (provider));
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  bar_plugin_external_queue_add_action (BAR_PLUGIN_EXTERNAL (provider),
                                          PROVIDER_PROP_TYPE_ACTION_ASK_REMOVE);
}



static void
bar_plugin_external_set_sensitive (BarPluginExternal *external)
{
  GValue value = { 0, };

  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (external));

  g_value_init (&value, G_TYPE_BOOLEAN);
  g_value_set_boolean (&value, GTK_WIDGET_IS_SENSITIVE (external));

  bar_plugin_external_queue_add (external, PROVIDER_PROP_TYPE_SET_SENSITIVE,
                                   &value);

  g_value_unset (&value);
}



void
bar_plugin_external_restart (BarPluginExternal *external)
{
  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (external));

  if (external->priv->pid != 0)
    {
      bar_debug (BAR_DEBUG_EXTERNAL,
                   "%s-%d: child asked to restart; pid=%d",
                   bar_module_get_name (external->module),
                   external->unique_id, external->priv->pid);

      bar_plugin_external_queue_free (external);

      if (external->priv->embedded)
        bar_plugin_external_queue_add_action (external, PROVIDER_PROP_TYPE_ACTION_QUIT_FOR_RESTART);
      else
        kill (external->priv->pid, SIGUSR1);
    }
}



void
bar_plugin_external_set_background_alpha (BarPluginExternal *external,
                                            gdouble              alpha)
{
  GValue value = { 0, };

  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (external));

  g_value_init (&value, G_TYPE_DOUBLE);
  g_value_set_double (&value, alpha);

  bar_plugin_external_queue_add (external, PROVIDER_PROP_TYPE_SET_BACKGROUND_ALPHA,
                                   &value);

  g_value_unset (&value);
}



void
bar_plugin_external_set_background_color (BarPluginExternal *external,
                                            const GdkColor      *color)
{
  GValue value = { 0, };

  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (external));

  if (G_LIKELY (color != NULL))
    {
      g_value_init (&value, G_TYPE_STRING);
      g_value_take_string (&value, gdk_color_to_string (color));

      bar_plugin_external_queue_add (external,
                                       PROVIDER_PROP_TYPE_SET_BACKGROUND_COLOR,
                                       &value);

      g_value_unset (&value);
    }
  else
    {
      bar_plugin_external_queue_add_action (external,
                                              PROVIDER_PROP_TYPE_ACTION_BACKGROUND_UNSET);
    }
}



void
bar_plugin_external_set_background_image (BarPluginExternal *external,
                                            const gchar         *image)
{
  GValue value = { 0, };

  bar_return_if_fail (BAR_IS_PLUGIN_EXTERNAL (external));

  if (!external->priv->embedded
      && BAR_IS_PLUGIN_EXTERNAL_46 (external))
    {
      /* hack to set the background of 4.6 plugins before the child is
       * embedded, so it is directly send with the startup arguments */
      bar_plugin_external_46_set_background_image (BAR_PLUGIN_EXTERNAL_46 (external), image);
    }
  else if (G_UNLIKELY (image != NULL))
    {
      g_value_init (&value, G_TYPE_STRING);
      g_value_set_string (&value, image);

      bar_plugin_external_queue_add (external,
                                       PROVIDER_PROP_TYPE_SET_BACKGROUND_IMAGE,
                                       &value);

      g_value_unset (&value);
    }
  else
    {
      bar_plugin_external_queue_add_action (external,
                                              PROVIDER_PROP_TYPE_ACTION_BACKGROUND_UNSET);
    }
}



GPid
bar_plugin_external_get_pid (BarPluginExternal *external)
{
  bar_return_val_if_fail (BAR_IS_PLUGIN_EXTERNAL (external), 0);
  return external->priv->pid;
}
