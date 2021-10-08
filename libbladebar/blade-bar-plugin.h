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

/* #if !defined(LIBBLADEBAR_INSIDE_LIBBLADEBAR_H) && !defined(LIBBLADEBAR_COMPILATION)
#error "Only <libbladebar/libbladebar.h> can be included directly, this file may disappear or change contents"
#endif */

#ifndef __BLADE_BAR_PLUGIN_H__
#define __BLADE_BAR_PLUGIN_H__

#include <gtk/gtk.h>
#include <libbladebar/libbladebar-enums.h>
#include <libbladebar/blade-bar-macros-46.h>

G_BEGIN_DECLS

typedef struct _BladeBarPluginPrivate BladeBarPluginPrivate;
typedef struct _BladeBarPluginClass   BladeBarPluginClass;
typedef struct _BladeBarPlugin        BladeBarPlugin;

/**
 * BladeBarPluginFunc:
 * @plugin : an #BladeBarPlugin
 *
 * Callback function to create the plugin contents. It should be given as
 * the argument to the registration macros.
 **/
typedef void (*BladeBarPluginFunc) (BladeBarPlugin *plugin);

/**
 * BladeBarPluginPreInit:
 * @argc: number of arguments to the plugin
 * @argv: argument array
 *
 * Callback function that is run in an external plugin before gtk_init(). It
 * should return %FALSE if the plugin is not available for whatever reason.
 * The function can be given as argument to one of the registration macros.
 *
 * The main purpose of this callback is to allow multithreaded plugins to call
 * g_thread_init().
 *
 * Returns: %TRUE on success, %FALSE otherwise.
 *
 * Since: 4.6
 **/
typedef gboolean (*BladeBarPluginPreInit) (gint    argc,
                                            gchar **argv);

/**
 * BladeBarPluginCheck:
 * @screen : the #GdkScreen the bar is running on
 *
 * Callback function that is run before creating a plugin. It should return
 * %FALSE if the plugin is not available for whatever reason. The function
 * can be given as argument to one of the registration macros.
 *
 * Returns: %TRUE if the plugin can be started, %FALSE otherwise.
 **/
typedef gboolean (*BladeBarPluginCheck) (GdkScreen *screen);

#define XFCE_TYPE_BAR_PLUGIN            (blade_bar_plugin_get_type ())
#define BLADE_BAR_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XFCE_TYPE_BAR_PLUGIN, BladeBarPlugin))
#define BLADE_BAR_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), XFCE_TYPE_BAR_PLUGIN, BladeBarPluginClass))
#define BLADE_IS_BAR_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XFCE_TYPE_BAR_PLUGIN))
#define BLADE_IS_BAR_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XFCE_TYPE_BAR_PLUGIN))
#define BLADE_BAR_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XFCE_TYPE_BAR_PLUGIN, BladeBarPluginClass))

/**
 * BladeBarPluginClass:
 * @construct :               This function is for object orientated plugins and
 *                            triggered after the init function of the object.
 *                            When this function is triggered, the plugin
 *                            information like name, display name, comment and unique
 *                            id are available. This is also the place where you would
 *                            call functions like blade_bar_plugin_menu_show_configure().
 *                            You can see this as the replacement of #BladeBarPluginFunc
 *                            for object based plugins. Since 4.8.
 * @screen_position_changed : See #BladeBarPlugin::screen-position-changed for more information.
 * @size_changed :            See #BladeBarPlugin::size-changed for more information.
 * @orientation_changed :     See #BladeBarPlugin::orientation-changed for more information.
 * @free_data :               See #BladeBarPlugin::free-data for more information.
 * @save :                    See #BladeBarPlugin::save for more information.
 * @about :                   See #BladeBarPlugin::about for more information.
 * @configure_plugin :        See #BladeBarPlugin::configure-plugin for more information.
 * @removed :                 See #BladeBarPlugin::removed for more information.
 * @remote_event :            See #BladeBarPlugin::remote-event for more information.
 *
 * Class of an #BladeBarPlugin. The interface can be used to create GObject based plugin.
 **/
struct _BladeBarPluginClass
{
  /*< private >*/
  GtkEventBoxClass __parent__;

  /*< public >*/
  /* for object oriented plugins only */
  void     (*construct)               (BladeBarPlugin    *plugin);

  /* signals */
  void     (*screen_position_changed) (BladeBarPlugin    *plugin,
                                       XfceScreenPosition  position);
  gboolean (*size_changed)            (BladeBarPlugin    *plugin,
                                       gint                size);
  void     (*orientation_changed)     (BladeBarPlugin    *plugin,
                                       GtkOrientation      orientation);
  void     (*free_data)               (BladeBarPlugin    *plugin);
  void     (*save)                    (BladeBarPlugin    *plugin);
  void     (*about)                   (BladeBarPlugin    *plugin);
  void     (*configure_plugin)        (BladeBarPlugin    *plugin);
  void     (*removed)                 (BladeBarPlugin    *plugin);
  gboolean (*remote_event)            (BladeBarPlugin    *plugin,
                                       const gchar        *name,
                                       const GValue       *value);

  /* new in 4.10 */
  void     (*mode_changed)            (BladeBarPlugin    *plugin,
                                       BladeBarPluginMode mode);
  void     (*nrows_changed)           (BladeBarPlugin    *plugin,
                                       guint               rows);

  /*< private >*/
  void (*reserved1) (void);
  void (*reserved2) (void);
};


/**
 * BladeBarPlugin:
 *
 * This struct contain private data only and should be accessed by
 * the functions below.
 **/
struct _BladeBarPlugin
{
  /*< private >*/
  GtkEventBox __parent__;

  /*< private >*/
  BladeBarPluginPrivate *priv;
};



GType                 blade_bar_plugin_get_type            (void) G_GNUC_CONST;

const gchar          *blade_bar_plugin_get_name            (BladeBarPlugin   *plugin) G_GNUC_PURE;

const gchar          *blade_bar_plugin_get_display_name    (BladeBarPlugin   *plugin) G_GNUC_PURE;

const gchar          *blade_bar_plugin_get_comment         (BladeBarPlugin   *plugin) G_GNUC_PURE;

gint                  blade_bar_plugin_get_unique_id       (BladeBarPlugin   *plugin) G_GNUC_PURE;

const gchar          *blade_bar_plugin_get_property_base   (BladeBarPlugin   *plugin) G_GNUC_PURE;

const gchar * const  *blade_bar_plugin_get_arguments       (BladeBarPlugin   *plugin) G_GNUC_PURE;

gint                  blade_bar_plugin_get_size            (BladeBarPlugin   *plugin) G_GNUC_PURE;

gboolean              blade_bar_plugin_get_expand          (BladeBarPlugin   *plugin) G_GNUC_PURE;

void                  blade_bar_plugin_set_expand          (BladeBarPlugin   *plugin,
                                                             gboolean           expand);

gboolean              blade_bar_plugin_get_shrink          (BladeBarPlugin   *plugin) G_GNUC_PURE;

void                  blade_bar_plugin_set_shrink          (BladeBarPlugin   *plugin,
                                                             gboolean           shrink);

gboolean              blade_bar_plugin_get_small           (BladeBarPlugin   *plugin) G_GNUC_PURE;

void                  blade_bar_plugin_set_small           (BladeBarPlugin   *plugin,
                                                             gboolean           small);

GtkOrientation        blade_bar_plugin_get_orientation     (BladeBarPlugin   *plugin) G_GNUC_PURE;

BladeBarPluginMode   blade_bar_plugin_get_mode            (BladeBarPlugin   *plugin) G_GNUC_PURE;

guint                 blade_bar_plugin_get_nrows           (BladeBarPlugin   *plugin) G_GNUC_PURE;

XfceScreenPosition    blade_bar_plugin_get_screen_position (BladeBarPlugin   *plugin) G_GNUC_PURE;

void                  blade_bar_plugin_take_window         (BladeBarPlugin   *plugin,
                                                             GtkWindow         *window);

void                  blade_bar_plugin_add_action_widget   (BladeBarPlugin   *plugin,
                                                             GtkWidget         *widget);

void                  blade_bar_plugin_menu_insert_item    (BladeBarPlugin   *plugin,
                                                             GtkMenuItem       *item);

void                  blade_bar_plugin_menu_show_configure (BladeBarPlugin   *plugin);

void                  blade_bar_plugin_menu_show_about     (BladeBarPlugin   *plugin);

gboolean              blade_bar_plugin_get_locked          (BladeBarPlugin   *plugin);

void                  blade_bar_plugin_remove              (BladeBarPlugin   *plugin);

void                  blade_bar_plugin_block_menu          (BladeBarPlugin   *plugin);

void                  blade_bar_plugin_unblock_menu        (BladeBarPlugin   *plugin);

void                  blade_bar_plugin_register_menu       (BladeBarPlugin   *plugin,
                                                             GtkMenu           *menu);

GtkArrowType          blade_bar_plugin_arrow_type          (BladeBarPlugin   *plugin);

void                  blade_bar_plugin_position_widget     (BladeBarPlugin   *plugin,
                                                             GtkWidget         *menu_widget,
                                                             GtkWidget         *attach_widget,
                                                             gint              *x,
                                                             gint              *y);

void                  blade_bar_plugin_position_menu       (GtkMenu           *menu,
                                                             gint              *x,
                                                             gint              *y,
                                                             gboolean          *push_in,
                                                             gpointer           bar_plugin);

void                  blade_bar_plugin_focus_widget        (BladeBarPlugin   *plugin,
                                                             GtkWidget         *widget);

void                  blade_bar_plugin_block_autohide      (BladeBarPlugin   *plugin,
                                                             gboolean           blocked);

gchar                *blade_bar_plugin_lookup_rc_file      (BladeBarPlugin   *plugin) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

gchar                *blade_bar_plugin_save_location       (BladeBarPlugin   *plugin,
                                                             gboolean           create) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS

#endif /* !__BLADE_BAR_PLUGIN_H__ */
