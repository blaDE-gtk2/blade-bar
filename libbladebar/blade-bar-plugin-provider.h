/*
 * Copyright (C) 2008-2010 Nick Schermer <nick@xfce.org>
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

#ifndef __BLADE_BAR_PLUGIN_PROVIDER_H__
#define __BLADE_BAR_PLUGIN_PROVIDER_H__

#include <gtk/gtk.h>
#include <libbladebar/libbladebar.h>

G_BEGIN_DECLS

typedef struct _BladeBarPluginProviderInterface BladeBarPluginProviderInterface;
typedef struct _BladeBarPluginProvider          BladeBarPluginProvider;

#define XFCE_TYPE_BAR_PLUGIN_PROVIDER               (blade_bar_plugin_provider_get_type ())
#define BLADE_BAR_PLUGIN_PROVIDER(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), XFCE_TYPE_BAR_PLUGIN_PROVIDER, BladeBarPluginProvider))
#define BLADE_IS_BAR_PLUGIN_PROVIDER(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XFCE_TYPE_BAR_PLUGIN_PROVIDER))
#define BLADE_BAR_PLUGIN_PROVIDER_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), XFCE_TYPE_BAR_PLUGIN_PROVIDER, BladeBarPluginProviderInterface))

/* plugin module functions */
typedef GtkWidget *(*PluginConstructFunc) (const gchar  *name,
                                           gint          unique_id,
                                           const gchar  *display_name,
                                           const gchar  *comment,
                                           gchar       **arguments,
                                           GdkScreen    *screen);
typedef GType      (*PluginInitFunc)      (GTypeModule  *module,
                                           gboolean     *make_resident);

struct _BladeBarPluginProviderInterface
{
  /*< private >*/
  GTypeInterface __parent__;

  /*<public >*/
  const gchar *(*get_name)            (BladeBarPluginProvider       *provider);
  gint         (*get_unique_id)       (BladeBarPluginProvider       *provider);
  void         (*set_size)            (BladeBarPluginProvider       *provider,
                                       gint                           size);
  void         (*set_mode)            (BladeBarPluginProvider       *provider,
                                       BladeBarPluginMode            mode);
  void         (*set_nrows)           (BladeBarPluginProvider       *provider,
                                       guint                          rows);
  void         (*set_screen_position) (BladeBarPluginProvider       *provider,
                                       XfceScreenPosition             screen_position);
  void         (*save)                (BladeBarPluginProvider       *provider);
  gboolean     (*get_show_configure)  (BladeBarPluginProvider       *provider);
  void         (*show_configure)      (BladeBarPluginProvider       *provider);
  gboolean     (*get_show_about)      (BladeBarPluginProvider       *provider);
  void         (*show_about)          (BladeBarPluginProvider       *provider);
  void         (*removed)             (BladeBarPluginProvider       *provider);
  gboolean     (*remote_event)        (BladeBarPluginProvider       *provider,
                                       const gchar                   *name,
                                       const GValue                  *value,
                                       guint                         *handle);
  void         (*set_locked)          (BladeBarPluginProvider       *provider,
                                       gboolean                       locked);
  void         (*ask_remove)          (BladeBarPluginProvider       *provider);
};

/* signals send from the plugin to the bar (possibly through the wrapper) */
typedef enum /*< skip >*/
{
  PROVIDER_SIGNAL_MOVE_PLUGIN = 0,
  PROVIDER_SIGNAL_EXPAND_PLUGIN,
  PROVIDER_SIGNAL_COLLAPSE_PLUGIN,
  PROVIDER_SIGNAL_SMALL_PLUGIN,
  PROVIDER_SIGNAL_UNSMALL_PLUGIN,
  PROVIDER_SIGNAL_LOCK_BAR,
  PROVIDER_SIGNAL_UNLOCK_BAR,
  PROVIDER_SIGNAL_REMOVE_PLUGIN,
  PROVIDER_SIGNAL_ADD_NEW_ITEMS,
  PROVIDER_SIGNAL_BAR_PREFERENCES,
  PROVIDER_SIGNAL_BAR_LOGOUT,
  PROVIDER_SIGNAL_BAR_ABOUT,
  PROVIDER_SIGNAL_BAR_HELP,
  PROVIDER_SIGNAL_SHOW_CONFIGURE,
  PROVIDER_SIGNAL_SHOW_ABOUT,
  PROVIDER_SIGNAL_FOCUS_PLUGIN,
  PROVIDER_SIGNAL_SHRINK_PLUGIN,
  PROVIDER_SIGNAL_UNSHRINK_PLUGIN
}
BladeBarPluginProviderSignal;

/* properties to the plugin; with a value or as an action */
typedef enum /*< skip >*/
{
  PROVIDER_PROP_TYPE_SET_SIZE,                /* gint */
  PROVIDER_PROP_TYPE_SET_MODE,                /* BladeBarPluginMode (as gint) */
  PROVIDER_PROP_TYPE_SET_SCREEN_POSITION,     /* XfceScreenPosition (as gint) */
  PROVIDER_PROP_TYPE_SET_BACKGROUND_ALPHA,    /* gdouble */
  PROVIDER_PROP_TYPE_SET_NROWS,               /* gint */
  PROVIDER_PROP_TYPE_SET_LOCKED,              /* gboolean */
  PROVIDER_PROP_TYPE_SET_SENSITIVE,           /* gboolean */
  PROVIDER_PROP_TYPE_SET_BACKGROUND_COLOR,    /* string, wrapper only */
  PROVIDER_PROP_TYPE_SET_BACKGROUND_IMAGE,    /* string, wrapper only */
  PROVIDER_PROP_TYPE_ACTION_REMOVED,          /* none */
  PROVIDER_PROP_TYPE_ACTION_SAVE,             /* none */
  PROVIDER_PROP_TYPE_ACTION_QUIT,             /* none */
  PROVIDER_PROP_TYPE_ACTION_QUIT_FOR_RESTART, /* none */
  PROVIDER_PROP_TYPE_ACTION_BACKGROUND_UNSET, /* none */
  PROVIDER_PROP_TYPE_ACTION_SHOW_CONFIGURE,   /* none */
  PROVIDER_PROP_TYPE_ACTION_SHOW_ABOUT,       /* none */
  PROVIDER_PROP_TYPE_ACTION_ASK_REMOVE        /* none */
}
BladeBarPluginProviderPropType;

/* plugin exit values */
enum
{
  PLUGIN_EXIT_SUCCESS = 0,
  PLUGIN_EXIT_FAILURE,
  PLUGIN_EXIT_ARGUMENTS_FAILED,
  PLUGIN_EXIT_PREINIT_FAILED,
  PLUGIN_EXIT_CHECK_FAILED,
  PLUGIN_EXIT_NO_PROVIDER,
  PLUGIN_EXIT_SUCCESS_AND_RESTART
};

/* argument handling in plugin and wrapper */
enum
{
  PLUGIN_ARGV_0 = 0,
  PLUGIN_ARGV_FILENAME,
  PLUGIN_ARGV_UNIQUE_ID,
  PLUGIN_ARGV_SOCKET_ID,
  PLUGIN_ARGV_NAME,
  PLUGIN_ARGV_DISPLAY_NAME,
  PLUGIN_ARGV_COMMENT,
  PLUGIN_ARGV_BACKGROUND_IMAGE,
  PLUGIN_ARGV_ARGUMENTS
};



GType                 blade_bar_plugin_provider_get_type            (void) G_GNUC_CONST;

const gchar          *blade_bar_plugin_provider_get_name            (BladeBarPluginProvider       *provider);

gint                  blade_bar_plugin_provider_get_unique_id       (BladeBarPluginProvider       *provider);

void                  blade_bar_plugin_provider_set_size            (BladeBarPluginProvider       *provider,
                                                                      gint                           size);

void                  blade_bar_plugin_provider_set_mode            (BladeBarPluginProvider       *provider,
                                                                      BladeBarPluginMode            mode);

void                  blade_bar_plugin_provider_set_nrows           (BladeBarPluginProvider       *provider,
                                                                      guint                          rows);

void                  blade_bar_plugin_provider_set_screen_position (BladeBarPluginProvider       *provider,
                                                                      XfceScreenPosition             screen_position);

void                  blade_bar_plugin_provider_save                (BladeBarPluginProvider       *provider);

void                  blade_bar_plugin_provider_emit_signal         (BladeBarPluginProvider       *provider,
                                                                      BladeBarPluginProviderSignal  provider_signal);

gboolean              blade_bar_plugin_provider_get_show_configure  (BladeBarPluginProvider       *provider);

void                  blade_bar_plugin_provider_show_configure      (BladeBarPluginProvider       *provider);

gboolean              blade_bar_plugin_provider_get_show_about      (BladeBarPluginProvider       *provider);

void                  blade_bar_plugin_provider_show_about          (BladeBarPluginProvider       *provider);

void                  blade_bar_plugin_provider_removed             (BladeBarPluginProvider       *provider);

gboolean              blade_bar_plugin_provider_remote_event        (BladeBarPluginProvider       *provider,
                                                                      const gchar                   *name,
                                                                      const GValue                  *value,
                                                                      guint                         *handle);

void                  blade_bar_plugin_provider_set_locked          (BladeBarPluginProvider       *provider,
                                                                      gboolean                       locked);

void                  blade_bar_plugin_provider_ask_remove          (BladeBarPluginProvider       *provider);

G_END_DECLS

#endif /* !__BLADE_BAR_PLUGIN_PROVIDER_H__ */
