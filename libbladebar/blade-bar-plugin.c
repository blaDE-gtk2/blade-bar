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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_LIBINTL_H
#include <libintl.h>
#endif
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include <gtk/gtk.h>
#if GTK_CHECK_VERSION (3, 0, 0)
#include <gtk/gtkx.h>
#endif
#include <glib.h>
#include <libbladeutil/libbladeutil.h>

#include <common/bar-private.h>
#include <libbladebar/blade-bar-macros.h>
#include <libbladebar/blade-bar-plugin.h>
#include <libbladebar/blade-bar-plugin-provider.h>
#include <libbladebar/libbladebar-marshal.h>
#include <libbladebar/libbladebar-alias.h>



/**
 * SECTION: blade-bar-plugin
 * @title: BladeBarPlugin
 * @short_description: Interface for bar plugins
 * @include: libbladebar/libbladebar.h
 *
 * The interface plugin developers used to interact with the plugin and
 * the bar.
 **/



#define BLADE_BAR_PLUGIN_CONSTRUCTED(plugin) \
  BAR_HAS_FLAG (BLADE_BAR_PLUGIN (plugin)->priv->flags, \
                  PLUGIN_FLAG_CONSTRUCTED)



typedef const gchar *(*ProviderToPluginChar) (BladeBarPluginProvider *provider);
typedef gint         (*ProviderToPluginInt)  (BladeBarPluginProvider *provider);



static void          blade_bar_plugin_provider_init          (BladeBarPluginProviderInterface *iface);
static GObject      *blade_bar_plugin_constructor            (GType                             type,
                                                               guint                             n_props,
                                                               GObjectConstructParam            *props);
static void          blade_bar_plugin_get_property           (GObject                          *object,
                                                               guint                             prop_id,
                                                               GValue                           *value,
                                                               GParamSpec                       *pspec);
static void          blade_bar_plugin_set_property           (GObject                          *object,
                                                               guint                             prop_id,
                                                               const GValue                     *value,
                                                               GParamSpec                       *pspec);
static void          blade_bar_plugin_dispose                (GObject                          *object);
static void          blade_bar_plugin_finalize               (GObject                          *object);
static void          blade_bar_plugin_realize                (GtkWidget                        *widget);
static gboolean      blade_bar_plugin_button_press_event     (GtkWidget                        *widget,
                                                               GdkEventButton                   *event);
static void          blade_bar_plugin_menu_move              (BladeBarPlugin                  *plugin);
static void          blade_bar_plugin_menu_remove            (BladeBarPlugin                  *plugin);
static void          blade_bar_plugin_menu_add_items         (BladeBarPlugin                  *plugin);
static void          blade_bar_plugin_menu_bar_preferences (BladeBarPlugin                  *plugin);
static GtkMenu      *blade_bar_plugin_menu_get               (BladeBarPlugin                  *plugin);
static inline gchar *blade_bar_plugin_relative_filename      (BladeBarPlugin                  *plugin);
static void          blade_bar_plugin_unregister_menu        (GtkMenu                          *menu,
                                                               BladeBarPlugin                  *plugin);
static void          blade_bar_plugin_set_size               (BladeBarPluginProvider          *provider,
                                                               gint                              size);
static void          blade_bar_plugin_set_mode               (BladeBarPluginProvider          *provider,
                                                               BladeBarPluginMode               mode);
static void          blade_bar_plugin_set_nrows              (BladeBarPluginProvider          *provider,
                                                               guint                             nrows);
static void          blade_bar_plugin_set_screen_position    (BladeBarPluginProvider          *provider,
                                                               XfceScreenPosition                screen_position);
static void          blade_bar_plugin_save                   (BladeBarPluginProvider          *provider);
static gboolean      blade_bar_plugin_get_show_configure     (BladeBarPluginProvider          *provider);
static void          blade_bar_plugin_show_configure         (BladeBarPluginProvider          *provider);
static gboolean      blade_bar_plugin_get_show_about         (BladeBarPluginProvider          *provider);
static void          blade_bar_plugin_show_about             (BladeBarPluginProvider          *provider);
static void          blade_bar_plugin_removed                (BladeBarPluginProvider          *provider);
static gboolean      blade_bar_plugin_remote_event           (BladeBarPluginProvider          *provider,
                                                               const gchar                      *name,
                                                               const GValue                     *value,
                                                               guint                            *handle);
static void          blade_bar_plugin_set_locked             (BladeBarPluginProvider          *provider,
                                                               gboolean                          locked);
static void          blade_bar_plugin_ask_remove             (BladeBarPluginProvider          *provider);
static void          blade_bar_plugin_take_window_notify     (gpointer                          data,
                                                               GObject                          *where_the_object_was);
static void          blade_bar_plugin_menu_item_destroy      (GtkWidget                        *item,
                                                               BladeBarPlugin                  *plugin);



enum
{
  PROP_0,
  PROP_NAME,
  PROP_DISPLAY_NAME,
  PROP_COMMENT,
  PROP_ARGUMENTS,
  PROP_UNIQUE_ID,
  PROP_ORIENTATION,
  PROP_SIZE,
  PROP_SMALL,
  PROP_SCREEN_POSITION,
  PROP_EXPAND,
  PROP_MODE,
  PROP_NROWS,
  PROP_SHRINK,
  N_PROPERTIES
};

enum
{
  ABOUT,
  CONFIGURE_PLUGIN,
  FREE_DATA,
  ORIENTATION_CHANGED,
  REMOTE_EVENT,
  REMOVED,
  SAVE,
  SIZE_CHANGED,
  SCREEN_POSITION_CHANGED,
  MODE_CHANGED,
  NROWS_CHANGED,
  LAST_SIGNAL
};

typedef enum
{
  PLUGIN_FLAG_DISPOSED       = 1 << 0,
  PLUGIN_FLAG_CONSTRUCTED    = 1 << 1,
  PLUGIN_FLAG_REALIZED       = 1 << 2,
  PLUGIN_FLAG_SHOW_CONFIGURE = 1 << 3,
  PLUGIN_FLAG_SHOW_ABOUT     = 1 << 4,
  PLUGIN_FLAG_BLOCK_AUTOHIDE = 1 << 5
}
PluginFlags;

struct _BladeBarPluginPrivate
{
  /* plugin information */
  gchar               *name;
  gchar               *display_name;
  gchar               *comment;
  gint                 unique_id;
  gchar               *property_base;
  gchar              **arguments;
  gint                 size; /* single row size */
  guint                expand : 1;
  guint                shrink : 1;
  guint                nrows;
  BladeBarPluginMode  mode;
  guint                small : 1;
  XfceScreenPosition   screen_position;
  guint                locked : 1;
  GSList              *menu_items;

  /* flags for rembering states */
  PluginFlags          flags;

  /* plugin right-click menu */
  GtkMenu             *menu;

  /* menu block counter (configure insensitive) */
  gint                 menu_blocked;

  /* autohide block counter */
  gint                 bar_lock;
};



static guint       plugin_signals[LAST_SIGNAL];
static GQuark      item_properties = 0;
static GQuark      item_about = 0;
static GParamSpec *plugin_props[N_PROPERTIES] = { NULL, };



G_DEFINE_TYPE_WITH_CODE (BladeBarPlugin, blade_bar_plugin, GTK_TYPE_EVENT_BOX,
                         G_IMPLEMENT_INTERFACE (XFCE_TYPE_BAR_PLUGIN_PROVIDER,
                         blade_bar_plugin_provider_init));



static void
blade_bar_plugin_class_init (BladeBarPluginClass *klass)
{
  GObjectClass   *gobject_class;
  GtkWidgetClass *gtkwidget_class;

  g_type_class_add_private (klass, sizeof (BladeBarPluginPrivate));

  klass->construct = NULL;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->constructor = blade_bar_plugin_constructor;
  gobject_class->get_property = blade_bar_plugin_get_property;
  gobject_class->set_property = blade_bar_plugin_set_property;
  gobject_class->dispose = blade_bar_plugin_dispose;
  gobject_class->finalize = blade_bar_plugin_finalize;

  gtkwidget_class = GTK_WIDGET_CLASS (klass);
  gtkwidget_class->realize = blade_bar_plugin_realize;
  gtkwidget_class->button_press_event = blade_bar_plugin_button_press_event;

  /**
   * BladeBarPlugin::about
   * @plugin : an #BladeBarPlugin.
   *
   * This signal is emmitted when the About entry in the right-click
   * menu is clicked. Plugin writes can use it to show information
   * about the plugin and display credits of the developers, translators
   * and other contributors.
   *
   * See also: blade_bar_plugin_menu_show_about().
   **/
  plugin_signals[ABOUT] =
    g_signal_new (g_intern_static_string ("about"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (BladeBarPluginClass, about),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  /**
   * BladeBarPlugin::configure-plugin
   * @plugin : an #BladeBarPlugin.
   *
   * This signal is emmitted when the Properties entry in the right-click
   * menu is clicked. Plugin writes can use this signal to open a
   * plugin settings dialog.
   *
   * See also: blade_bar_plugin_menu_show_configure() and
   *           xfce_titled_dialog_new ().
   **/
  plugin_signals[CONFIGURE_PLUGIN] =
    g_signal_new (g_intern_static_string ("configure-plugin"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (BladeBarPluginClass, configure_plugin),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  /**
   * BladeBarPlugin::free-data
   * @plugin : an #BladeBarPlugin.
   *
   * This signal is emmitted when the plugin is closing. Plugin
   * writers should use this signal to free any allocated resources.
   *
   * See also #XfceHVBox.
   **/
  plugin_signals[FREE_DATA] =
    g_signal_new (g_intern_static_string ("free-data"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (BladeBarPluginClass, free_data),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  /**
   * BladeBarPlugin::orientation-changed
   * @plugin      : an #BladeBarPlugin.
   * @orientation : new #GtkOrientation of the bar.
   *
   * This signal is emmitted whenever the orientation of the bar
   * the @plugin is on changes. Plugins writers can for example use
   * this signal to change the order of widgets in the plugin.
   *
   * See also: #XfceHVBox.
   **/
  plugin_signals[ORIENTATION_CHANGED] =
    g_signal_new (g_intern_static_string ("orientation-changed"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (BladeBarPluginClass, orientation_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__ENUM,
                  G_TYPE_NONE, 1, GTK_TYPE_ORIENTATION);

  /**
   * BladeBarPlugin::mode-changed
   * @plugin : an #BladeBarPlugin.
   * @mode   : new #BladeBarPluginMode of the bar.
   *
   * This signal is emmitted whenever the mode of the bar
   * the @plugin is on changes.
   *
   * Since: 4.10
   **/
  plugin_signals[MODE_CHANGED] =
    g_signal_new (g_intern_static_string ("mode-changed"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (BladeBarPluginClass, mode_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__ENUM,
                  G_TYPE_NONE, 1, XFCE_TYPE_BAR_PLUGIN_MODE);

  /**
   * BladeBarPlugin::nrows-changed
   * @plugin : an #BladeBarPlugin.
   * @rows   : new number of rows of the bar
   *
   * This signal is emmitted whenever the nrows of the bar
   * the @plugin is on changes.
   *
   * Since: 4.10
   **/
  plugin_signals[NROWS_CHANGED] =
    g_signal_new (g_intern_static_string ("nrows-changed"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (BladeBarPluginClass, nrows_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__UINT,
                  G_TYPE_NONE, 1, G_TYPE_UINT);

  /**
   * BladeBarPlugin::remote-event
   * @plugin : an #BladeBarPlugin.
   * @name   : name of the signal.
   * @value  : value of the signal.
   *
   * This signal is emmitted by the user by running
   * blade-bar --plugin-event=plugin-name:name:type:value. It can be
   * used for remote communication, like for example to popup a menu.
   *
   * Returns: %TRUE to stop signal emission to other plugins, %FALSE
   *          to send the signal also to other plugins with the same
   *          name.
   **/
  plugin_signals[REMOTE_EVENT] =
    g_signal_new (g_intern_static_string ("remote-event"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (BladeBarPluginClass, remote_event),
                  NULL, NULL,
                  _libbladebar_marshal_BOOLEAN__STRING_BOXED,
                  G_TYPE_BOOLEAN, 2, G_TYPE_STRING, G_TYPE_VALUE);

  /**
   * BladeBarPlugin::removed
   * @plugin : an #BladeBarPlugin.
   *
   * This signal is emmitted when the plugin is permanently removed from
   * the bar configuration by the user. Developers can use this signal
   * to cleanup custom setting locations that for example store passwords.
   *
   * The free-data signal is emitted after this signal!
   *
   * Note that if you use the blconf channel and base property provided
   * by blade_bar_plugin_get_property_base() or the rc file location
   * returned by blade_bar_plugin_save_location(), the bar will take
   * care of removing those settings.
   *
   * Since: 4.8
   **/
  plugin_signals[REMOVED] =
    g_signal_new (g_intern_static_string ("removed"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (BladeBarPluginClass, removed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  /**
   * BladeBarPlugin::save
   * @plugin : an #BladeBarPlugin.
   *
   * This signal is emitted when the plugin should save it's
   * configuration. The signal is always emmitted before the plugin
   * closes (before the "free-data" signal) and also once in 10
   * minutes or so.
   *
   * See also: blade_bar_plugin_save_location().
   **/
  plugin_signals[SAVE] =
    g_signal_new (g_intern_static_string ("save"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (BladeBarPluginClass, save),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  /**
   * BladeBarPlugin::size-changed
   * @plugin : an #BladeBarPlugin.
   * @size   : the new size of the bar.
   *
   * This signal is emmitted whenever the size of the bar
   * the @plugin is on changes. Plugins writers can for example use
   * this signal to update their icon size.
   *
   * If the function returns %FALSE or is not used, the bar will force
   * a square size to the plugin. If you want non-square plugins and you
   * don't need this signal you can use something like this:
   *
   * g_signal_connect (plugin, "size-changed", G_CALLBACK (gtk_true), NULL);
   **/
  plugin_signals[SIZE_CHANGED] =
    g_signal_new (g_intern_static_string ("size-changed"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (BladeBarPluginClass, size_changed),
                  g_signal_accumulator_true_handled, NULL,
                  _libbladebar_marshal_BOOLEAN__INT,
                  G_TYPE_BOOLEAN, 1, G_TYPE_INT);

  /**
   * BladeBarPlugin::screen-position-changed
   * @plugin   : an #BladeBarPlugin.
   * @position : the new #XfceScreenPosition of the bar.
   *
   * This signal is emmitted whenever the screen position of the bar
   * the @plugin is on changes. Plugins writers can for example use
   * this signal to change the arrow direction of buttons.
   **/
  plugin_signals[SCREEN_POSITION_CHANGED] =
    g_signal_new (g_intern_static_string ("screen-position-changed"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (BladeBarPluginClass, screen_position_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__ENUM,
                  G_TYPE_NONE, 1, XFCE_TYPE_SCREEN_POSITION);

  /**
   * BladeBarPlugin:name:
   *
   * The internal, unstranslated, name of the #BladeBarPlugin. Plugin
   * writer can use it to read the plugin name, but
   * blade_bar_plugin_get_name() is recommended since that returns
   * a const string.
   **/
  plugin_props[PROP_NAME] =
      g_param_spec_string ("name",
                           "Name",
                           "Plugin internal name",
                           NULL,
                           G_PARAM_READWRITE
                           | G_PARAM_STATIC_STRINGS
                           | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BladeBarPlugin:display-name:
   *
   * The translated display name of the #BladeBarPlugin. This property is set
   * during plugin construction and can't be set twice. Plugin writer can use
   * it to read the plugin display name, but blade_bar_plugin_get_display_name()
   * is recommended.
   **/
  plugin_props[PROP_DISPLAY_NAME] =
      g_param_spec_string ("display-name",
                           "Display Name",
                           "Plugin display name",
                           NULL,
                           G_PARAM_READWRITE
                           | G_PARAM_STATIC_STRINGS
                           | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BladeBarPlugin:comment:
   *
   * The translated description of the #BladeBarPlugin. This property is set
   * during plugin construction and can't be set twice. Plugin writer can use
   * it to read the plugin description, but blade_bar_plugin_get_comment()
   * is recommended.
   *
   * Since: 4.8
   **/
  plugin_props[PROP_COMMENT] =
      g_param_spec_string ("comment",
                           "Comment",
                           "Plugin comment",
                           NULL,
                           G_PARAM_READWRITE
                           | G_PARAM_STATIC_STRINGS
                           | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BladeBarPlugin:id:
   *
   * The unique id of the #BladeBarPlugin. This property is set during plugin
   * construction and can't be set twice. Plugin writer can use it to read the
   * plugin display name, but blade_bar_plugin_get_unique_id() is recommended.
   *
   * Since: 4.8
   **/
  plugin_props[PROP_UNIQUE_ID] =
      g_param_spec_int ("unique-id",
                        "Unique ID",
                        "Unique plugin ID",
                        -1, G_MAXINT, -1,
                        G_PARAM_READWRITE
                        | G_PARAM_STATIC_STRINGS
                        | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BladeBarPlugin:arguments:
   *
   * The arguments the plugin was started with. If the plugin was not
   * started with any arguments this value is %NULL. Plugin writer can
   * use it to read the arguments array, but
   * blade_bar_plugin_get_arguments() is recommended.
   **/
  plugin_props[PROP_ARGUMENTS] =
      g_param_spec_boxed ("arguments",
                          "Arguments",
                          "Startup arguments for the plugin",
                          G_TYPE_STRV,
                          G_PARAM_READWRITE
                          | G_PARAM_STATIC_STRINGS
                          | G_PARAM_CONSTRUCT_ONLY);

  /**
   * BladeBarPlugin:orientation:
   *
   * The #GtkOrientation of the #BladeBarPlugin. Plugin writer can use it to read the
   * plugin orientation, but blade_bar_plugin_get_orientation() is recommended.
   **/
  plugin_props[PROP_ORIENTATION] =
      g_param_spec_enum ("orientation",
                         "Orientation",
                         "Orientation of the plugin's bar",
                         GTK_TYPE_ORIENTATION,
                         GTK_ORIENTATION_HORIZONTAL,
                         G_PARAM_READABLE
                         | G_PARAM_STATIC_STRINGS);

  /**
   * BladeBarPlugin:size:
   *
   * The size in pixels of the #BladeBarPlugin. Plugin writer can use it to read the
   * plugin size, but blade_bar_plugin_get_size() is recommended.
   **/
  plugin_props[PROP_SIZE] =
      g_param_spec_int ("size",
                        "Size",
                        "Size of the plugin's bar",
                        0, (128 * 6), 0,
                        G_PARAM_READABLE
                        | G_PARAM_STATIC_STRINGS);

  /**
   * BladeBarPlugin:screen-position:
   *
   * The #XfceScreenPosition of the #BladeBarPlugin. Plugin writer can use it
   * to read the plugin's screen position, but blade_bar_plugin_get_screen_psotion()
   * is recommended.
   **/
  plugin_props[PROP_SCREEN_POSITION] =
      g_param_spec_enum  ("screen-position",
                          "Screen Position",
                          "Screen position of the plugin's bar",
                          XFCE_TYPE_SCREEN_POSITION,
                          XFCE_SCREEN_POSITION_NONE,
                          G_PARAM_READABLE
                          | G_PARAM_STATIC_STRINGS);

  /**
   * BladeBarPlugin:small:
   *
   * Wether the #BladeBarPlugin is small enough to fit a single row of a multi-row bar.
   * Plugin writes can use it to read or set this property, but blade_bar_plugin_set_small()
   * is recommended.
   *
   * Since: 4.10
   **/
  plugin_props[PROP_SMALL] =
      g_param_spec_boolean ("small",
                            "Small",
                            "Is this plugin small, e.g. a single button?",
                            FALSE,
                            G_PARAM_READWRITE
                            | G_PARAM_STATIC_STRINGS);

  /**
   * BladeBarPlugin:expand:
   *
   * Wether the #BladeBarPlugin expands on the bar. Plugin writes can use it
   * to read or set this property, but blade_bar_plugin_set_expand()
   * is recommended.
   **/
  plugin_props[PROP_EXPAND] =
      g_param_spec_boolean ("expand",
                            "Expand",
                            "Whether this plugin is expanded",
                            FALSE,
                            G_PARAM_READWRITE
                            | G_PARAM_STATIC_STRINGS);

  /**
   * BladeBarPlugin:shrink:
   *
   * Wether the #BladeBarPlugin can shrink when there is no space left on the bar.
   * Plugin writes can use it to read or set this property, but blade_bar_plugin_set_shrink()
   * is recommended.
   *
   * Since: 4.10
   **/
  plugin_props[PROP_SHRINK] =
      g_param_spec_boolean ("shrink",
                            "Shrink",
                            "Whether this plugin can shrink",
                            FALSE,
                            G_PARAM_READWRITE
                            | G_PARAM_STATIC_STRINGS);

  /**
   * BladeBarPlugin:mode:
   *
   * Display mode of the plugin.
   *
   * Since: 4.10
   **/
  plugin_props[PROP_MODE] =
      g_param_spec_enum ("mode",
                         "Mode",
                         "Disply mode of the plugin",
                         XFCE_TYPE_BAR_PLUGIN_MODE,
                         BLADE_BAR_PLUGIN_MODE_HORIZONTAL,
                         G_PARAM_READABLE
                         | G_PARAM_STATIC_STRINGS);

  /**
   * BladeBarPlugin:nrows:
   *
   * Number of rows the plugin is embedded on.
   *
   * Since: 4.10
   **/
  plugin_props[PROP_NROWS] =
      g_param_spec_uint ("nrows",
                         "Nrows",
                         "Number of rows of the bar",
                         1, 6, 1,
                         G_PARAM_READABLE
                         | G_PARAM_STATIC_STRINGS);

  /* install all properties */
  g_object_class_install_properties (gobject_class, N_PROPERTIES, plugin_props);

  item_properties = g_quark_from_static_string ("item-properties");
  item_about = g_quark_from_static_string ("item-about");
}



static void
blade_bar_plugin_init (BladeBarPlugin *plugin)
{
  plugin->priv = G_TYPE_INSTANCE_GET_PRIVATE (plugin, XFCE_TYPE_BAR_PLUGIN, BladeBarPluginPrivate);

  plugin->priv->name = NULL;
  plugin->priv->display_name = NULL;
  plugin->priv->comment = NULL;
  plugin->priv->unique_id = -1;
  plugin->priv->property_base = NULL;
  plugin->priv->arguments = NULL;
  plugin->priv->size = 0;
  plugin->priv->small = FALSE;
  plugin->priv->expand = FALSE;
  plugin->priv->shrink = FALSE;
  plugin->priv->mode = BLADE_BAR_PLUGIN_MODE_HORIZONTAL;
  plugin->priv->screen_position = XFCE_SCREEN_POSITION_NONE;
  plugin->priv->menu = NULL;
  plugin->priv->menu_blocked = 0;
  plugin->priv->bar_lock = 0;
  plugin->priv->flags = 0;
  plugin->priv->locked = TRUE;
  plugin->priv->menu_items = NULL;
  plugin->priv->nrows = 1;

  /* bind the text domain of the bar so our strings
   * are properly translated in the old 4.6 bar plugins */
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif

  /* hide the event box window to make the plugin transparent */
  gtk_event_box_set_visible_window (GTK_EVENT_BOX (plugin), FALSE);
}



static void
blade_bar_plugin_provider_init (BladeBarPluginProviderInterface *iface)
{
  iface->get_name = (ProviderToPluginChar) blade_bar_plugin_get_name;
  iface->get_unique_id = (ProviderToPluginInt) blade_bar_plugin_get_unique_id;
  iface->set_size = blade_bar_plugin_set_size;
  iface->set_mode = blade_bar_plugin_set_mode;
  iface->set_nrows = blade_bar_plugin_set_nrows;
  iface->set_screen_position = blade_bar_plugin_set_screen_position;
  iface->save = blade_bar_plugin_save;
  iface->get_show_configure = blade_bar_plugin_get_show_configure;
  iface->show_configure = blade_bar_plugin_show_configure;
  iface->get_show_about = blade_bar_plugin_get_show_about;
  iface->show_about = blade_bar_plugin_show_about;
  iface->removed = blade_bar_plugin_removed;
  iface->remote_event = blade_bar_plugin_remote_event;
  iface->set_locked = blade_bar_plugin_set_locked;
  iface->ask_remove = blade_bar_plugin_ask_remove;
}



static GObject *
blade_bar_plugin_constructor (GType                  type,
                               guint                  n_props,
                               GObjectConstructParam *props)
{
  GObject *plugin;

  plugin = G_OBJECT_CLASS (blade_bar_plugin_parent_class)->constructor (type, n_props, props);

  /* all the properties are set and can be used in public */
  BAR_SET_FLAG (BLADE_BAR_PLUGIN (plugin)->priv->flags, PLUGIN_FLAG_CONSTRUCTED);

  return plugin;
}



static void
blade_bar_plugin_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  BladeBarPluginPrivate *private = BLADE_BAR_PLUGIN (object)->priv;

  switch (prop_id)
    {
    case PROP_NAME:
      g_value_set_static_string (value, private->name);
      break;

    case PROP_DISPLAY_NAME:
      g_value_set_static_string (value, private->display_name);
      break;

    case PROP_COMMENT:
      g_value_set_static_string (value, private->comment);
      break;

    case PROP_UNIQUE_ID:
      g_value_set_int (value, private->unique_id);
      break;

    case PROP_ARGUMENTS:
      g_value_set_boxed (value, private->arguments);
      break;

    case PROP_ORIENTATION:
      g_value_set_enum (value, blade_bar_plugin_get_orientation (BLADE_BAR_PLUGIN (object)));
      break;

    case PROP_SIZE:
      g_value_set_int (value, private->size * private->nrows);
      break;

    case PROP_NROWS:
      g_value_set_uint (value, private->nrows);
      break;

    case PROP_MODE:
      g_value_set_enum (value, private->mode);
      break;

    case PROP_SMALL:
      g_value_set_boolean (value, private->small);
      break;

    case PROP_SCREEN_POSITION:
      g_value_set_enum (value, private->screen_position);
      break;

    case PROP_EXPAND:
      g_value_set_boolean (value, private->expand);
      break;

    case PROP_SHRINK:
      g_value_set_boolean (value, private->shrink);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
blade_bar_plugin_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  BladeBarPluginPrivate *private = BLADE_BAR_PLUGIN (object)->priv;
  gchar                  *name;

  switch (prop_id)
    {
    case PROP_NAME:
    case PROP_UNIQUE_ID:
      if (prop_id == PROP_NAME)
        private->name = g_value_dup_string (value);
      else
        private->unique_id = g_value_get_int (value);

      if (private->unique_id != -1 && private->name != NULL)
        {
          /* give the widget a unique name for theming */
          name = g_strdup_printf ("%s-%d", private->name, private->unique_id);
          gtk_widget_set_name (GTK_WIDGET (object), name);
          g_free (name);
        }
      break;

    case PROP_DISPLAY_NAME:
      private->display_name = g_value_dup_string (value);
      break;

    case PROP_COMMENT:
      private->comment = g_value_dup_string (value);
      break;

    case PROP_ARGUMENTS:
      private->arguments = g_value_dup_boxed (value);
      break;

    case PROP_SMALL:
      blade_bar_plugin_set_small (BLADE_BAR_PLUGIN (object),
                                   g_value_get_boolean (value));
      break;

    case PROP_EXPAND:
      blade_bar_plugin_set_expand (BLADE_BAR_PLUGIN (object),
                                    g_value_get_boolean (value));
      break;

    case PROP_SHRINK:
      blade_bar_plugin_set_shrink (BLADE_BAR_PLUGIN (object),
                                    g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
blade_bar_plugin_dispose (GObject *object)
{
  BladeBarPlugin *plugin = BLADE_BAR_PLUGIN (object);

  if (!BAR_HAS_FLAG (plugin->priv->flags, PLUGIN_FLAG_DISPOSED))
    {
      /* allow the plugin to cleanup */
      g_signal_emit (G_OBJECT (object), plugin_signals[FREE_DATA], 0);

      /* plugin disposed, don't try this again */
      BAR_SET_FLAG (plugin->priv->flags, PLUGIN_FLAG_DISPOSED);
    }

  (*G_OBJECT_CLASS (blade_bar_plugin_parent_class)->dispose) (object);
}



static void
blade_bar_plugin_finalize (GObject *object)
{
  BladeBarPlugin *plugin = BLADE_BAR_PLUGIN (object);
  GSList          *li;

  /* destroy the menu */
  if (plugin->priv->menu != NULL)
    {
      gtk_widget_destroy (GTK_WIDGET (plugin->priv->menu));
      bar_assert (plugin->priv->menu_items == NULL);
    }
  else
    {
      /* release custom menu items */
      for (li = plugin->priv->menu_items; li != NULL; li = li->next)
        {
          g_signal_handlers_disconnect_by_func (G_OBJECT (li->data),
              G_CALLBACK (blade_bar_plugin_menu_item_destroy), plugin);
          g_object_unref (G_OBJECT (li->data));
        }
      g_slist_free (plugin->priv->menu_items);
    }

  g_free (plugin->priv->name);
  g_free (plugin->priv->display_name);
  g_free (plugin->priv->comment);
  g_free (plugin->priv->property_base);
  g_strfreev (plugin->priv->arguments);

  (*G_OBJECT_CLASS (blade_bar_plugin_parent_class)->finalize) (object);
}



static void
blade_bar_plugin_realize (GtkWidget *widget)
{
  BladeBarPluginClass *klass;
  BladeBarPlugin      *plugin = BLADE_BAR_PLUGIN (widget);

  /* let gtk realize the plugin */
  (*GTK_WIDGET_CLASS (blade_bar_plugin_parent_class)->realize) (widget);

  /* launch the construct function for object oriented plugins, but
   * do this only once */
  if (!BAR_HAS_FLAG (plugin->priv->flags, PLUGIN_FLAG_REALIZED))
    {
      BAR_SET_FLAG (plugin->priv->flags, PLUGIN_FLAG_REALIZED);

      /* whether this is an object plugin */
      klass = BLADE_BAR_PLUGIN_GET_CLASS (widget);
      if (klass->construct != NULL)
        (*klass->construct) (BLADE_BAR_PLUGIN (widget));
    }
}



static gboolean
blade_bar_plugin_button_press_event (GtkWidget      *widget,
                                      GdkEventButton *event)
{
  BladeBarPlugin *plugin = BLADE_BAR_PLUGIN (widget);
  guint            modifiers;
  GtkMenu         *menu;
  GtkWidget       *item;

  bar_return_val_if_fail (BLADE_IS_BAR_PLUGIN (widget), FALSE);

  /* get the default accelerator modifier mask */
  modifiers = event->state & gtk_accelerator_get_default_mod_mask ();

  if (event->button == 3
      || (event->button == 1 && modifiers == GDK_CONTROL_MASK))
    {
      /* get the bar menu */
      menu = blade_bar_plugin_menu_get (plugin);

      /* if the menu is block, some items are insensitive */
      item = g_object_get_qdata (G_OBJECT (menu), item_properties);
      if (item != NULL)
        gtk_widget_set_sensitive (item, plugin->priv->menu_blocked == 0);

      /* popup the menu */
      gtk_menu_popup (menu, NULL, NULL, NULL, NULL, event->button, event->time);

      return TRUE;
    }

  return FALSE;
}



static void
blade_bar_plugin_menu_move (BladeBarPlugin *plugin)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (plugin));

  /* move the plugin */
  blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (plugin),
                                          PROVIDER_SIGNAL_MOVE_PLUGIN);
}



static void
blade_bar_plugin_menu_remove (BladeBarPlugin *plugin)
{
  GtkWidget *dialog;

  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));

  /* create question dialog (same code is also in bar-preferences-dialog.c) */
  dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
      GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
      /* I18N: %s is the name of the plugin */
      _("Are you sure that you want to remove \"%s\"?"),
      blade_bar_plugin_get_display_name (plugin));
  gtk_window_set_screen (GTK_WINDOW (dialog),
      gtk_widget_get_screen (GTK_WIDGET (plugin)));
  gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
      _("If you remove the item from the bar, it is permanently lost."));
  gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_CANCEL,
      GTK_RESPONSE_NO, GTK_STOCK_REMOVE, GTK_RESPONSE_YES, NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_NO);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_YES)
    {
      gtk_widget_hide (dialog);

      /* ask the bar or wrapper to remove the plugin */
      blade_bar_plugin_remove (plugin);
    }

  gtk_widget_destroy (dialog);
}



static void
blade_bar_plugin_menu_add_items (BladeBarPlugin *plugin)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (plugin));
  bar_return_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin));

  /* open items dialog */
  if (!blade_bar_plugin_get_locked (plugin))
    blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (plugin),
                                            PROVIDER_SIGNAL_ADD_NEW_ITEMS);
}



static void
blade_bar_plugin_menu_bar_preferences (BladeBarPlugin *plugin)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (plugin));
  bar_return_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin));

  /* open preferences dialog */
  if (!blade_bar_plugin_get_locked (plugin))
    blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (plugin),
                                            PROVIDER_SIGNAL_BAR_PREFERENCES);
}



static void
blade_bar_plugin_menu_bar_logout (BladeBarPlugin *plugin)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (plugin));
  bar_return_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin));

  /* logout the session */
  blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (plugin),
                                          PROVIDER_SIGNAL_BAR_LOGOUT);
}



static void
blade_bar_plugin_menu_bar_about (BladeBarPlugin *plugin)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (plugin));
  bar_return_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin));

  /* open the about dialog of the bar */
  blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (plugin),
                                          PROVIDER_SIGNAL_BAR_ABOUT);
}



static void
blade_bar_plugin_menu_bar_help (BladeBarPlugin *plugin)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (plugin));
  bar_return_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin));

  /* open the manual of the bar */
  blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (plugin),
                                          PROVIDER_SIGNAL_BAR_HELP);
}



static void
blade_bar_plugin_menu_destroy (BladeBarPlugin *plugin)
{
  GSList *li;

  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));

  if (plugin->priv->menu != NULL)
    {
      /* remove custom items before they get destroyed */
      for (li = plugin->priv->menu_items; li != NULL; li = li->next)
        gtk_container_remove (GTK_CONTAINER (plugin->priv->menu), GTK_WIDGET (li->data));

      gtk_widget_destroy (GTK_WIDGET (plugin->priv->menu));
      plugin->priv->menu = NULL;
    }
}



static GtkMenu *
blade_bar_plugin_menu_get (BladeBarPlugin *plugin)
{
  GtkWidget *menu, *submenu;
  GtkWidget *item;
  GtkWidget *image;
  gboolean   locked;
  GSList    *li;

  bar_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), NULL);

  if (G_UNLIKELY (plugin->priv->menu == NULL))
    {
      locked = blade_bar_plugin_get_locked (plugin);

      menu = gtk_menu_new ();
      gtk_menu_attach_to_widget (GTK_MENU (menu), GTK_WIDGET (plugin), NULL);

      /* item with plugin name */
      item = gtk_menu_item_new_with_label (blade_bar_plugin_get_display_name (plugin));
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
      gtk_widget_set_sensitive (item, FALSE);
      gtk_widget_show (item);

      /* separator */
      item = gtk_separator_menu_item_new ();
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
      gtk_widget_show (item);

      if (!locked)
        {
          /* properties item */
          item = gtk_image_menu_item_new_from_stock (GTK_STOCK_PROPERTIES, NULL);
          g_signal_connect_swapped (G_OBJECT (item), "activate",
              G_CALLBACK (blade_bar_plugin_show_configure), plugin);
          g_object_set_qdata (G_OBJECT (menu), item_properties, item);
          gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
          if (BAR_HAS_FLAG (plugin->priv->flags, PLUGIN_FLAG_SHOW_CONFIGURE))
            gtk_widget_show (item);

          /* about item */
          item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
          g_signal_connect_swapped (G_OBJECT (item), "activate",
              G_CALLBACK (blade_bar_plugin_show_about), plugin);
          g_object_set_qdata (G_OBJECT (menu), item_about, item);
          gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
          if (BAR_HAS_FLAG (plugin->priv->flags, PLUGIN_FLAG_SHOW_ABOUT))
            gtk_widget_show (item);

          /* move item */
          item = gtk_image_menu_item_new_with_mnemonic (_("_Move"));
          g_signal_connect_swapped (G_OBJECT (item), "activate",
              G_CALLBACK (blade_bar_plugin_menu_move), plugin);
          gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
          gtk_widget_show (item);

          image = gtk_image_new_from_stock (GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_MENU);
          gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
          gtk_widget_show (image);

          /* add custom menu items */
          for (li = plugin->priv->menu_items; li != NULL; li = li->next)
            gtk_menu_shell_append (GTK_MENU_SHELL (menu), GTK_WIDGET (li->data));

          /* separator */
          item = gtk_separator_menu_item_new ();
          gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
          gtk_widget_show (item);

          /* remove */
          item = gtk_image_menu_item_new_from_stock (GTK_STOCK_REMOVE, NULL);
          g_signal_connect_swapped (G_OBJECT (item), "activate",
              G_CALLBACK (blade_bar_plugin_menu_remove), plugin);
          gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
          gtk_widget_show (item);

          /* separator */
          item = gtk_separator_menu_item_new ();
          gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
          gtk_widget_show (item);
        }

      /* create a bar submenu item */
      submenu = gtk_menu_new ();
      item = gtk_menu_item_new_with_mnemonic (_("Pane_l"));
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), submenu);
      gtk_widget_show (item);

      if (!locked)
        {
          /* add new items */
          item = gtk_image_menu_item_new_with_mnemonic (_("Add _New Items..."));
          g_signal_connect_swapped (G_OBJECT (item), "activate",
              G_CALLBACK (blade_bar_plugin_menu_add_items), plugin);
          gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
          gtk_widget_show (item);

          image = gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_MENU);
          gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
          gtk_widget_show (image);

          /* customize bar */
          item = gtk_image_menu_item_new_with_mnemonic (_("Bar Pr_eferences..."));
          g_signal_connect_swapped (G_OBJECT (item), "activate",
              G_CALLBACK (blade_bar_plugin_menu_bar_preferences), plugin);
          gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
          gtk_widget_show (item);

          image = gtk_image_new_from_stock (GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU);
          gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
          gtk_widget_show (image);

          /* separator */
          item = gtk_separator_menu_item_new ();
          gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
          gtk_widget_show (item);
        }

      /* logout item */
      item = gtk_image_menu_item_new_with_mnemonic (_("Log _Out"));
      g_signal_connect_swapped (G_OBJECT (item), "activate",
          G_CALLBACK (blade_bar_plugin_menu_bar_logout), plugin);
      gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
      gtk_widget_show (item);

      image = gtk_image_new_from_icon_name ("system-log-out", GTK_ICON_SIZE_MENU);
      gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);
      gtk_widget_show (image);

      /* separator */
      item = gtk_separator_menu_item_new ();
      gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
      gtk_widget_show (item);

      /* help item */
      item = gtk_image_menu_item_new_from_stock (GTK_STOCK_HELP, NULL);
      g_signal_connect_swapped (G_OBJECT (item), "activate",
          G_CALLBACK (blade_bar_plugin_menu_bar_help), plugin);
      gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
      gtk_widget_show (item);

      /* about item */
      item = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
      g_signal_connect_swapped (G_OBJECT (item), "activate",
          G_CALLBACK (blade_bar_plugin_menu_bar_about), plugin);
      gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
      gtk_widget_show (item);

      /* set bar menu */
      plugin->priv->menu = GTK_MENU (menu);
    }

  /* block autohide when this menu is shown */
  blade_bar_plugin_register_menu (plugin, GTK_MENU (plugin->priv->menu));

  return plugin->priv->menu;
}



static inline gchar *
blade_bar_plugin_relative_filename (BladeBarPlugin *plugin)
{
  bar_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), NULL);
  bar_return_val_if_fail (blade_bar_plugin_get_name (plugin) != NULL, NULL);
  bar_return_val_if_fail (blade_bar_plugin_get_unique_id (plugin) != -1, NULL);

  /* return the relative configuration filename */
  return g_strdup_printf (BAR_PLUGIN_RC_RELATIVE_PATH,
                          plugin->priv->name, plugin->priv->unique_id);
}



static void
blade_bar_plugin_unregister_menu (GtkMenu         *menu,
                                   BladeBarPlugin *plugin)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  bar_return_if_fail (plugin->priv->bar_lock > 0);
  bar_return_if_fail (GTK_IS_MENU (menu));

  /* disconnect this signal */
  g_signal_handlers_disconnect_by_func (G_OBJECT (menu),
      G_CALLBACK (blade_bar_plugin_unregister_menu), plugin);

  if (G_LIKELY (plugin->priv->bar_lock > 0))
    {
      /* decrease the counter */
      plugin->priv->bar_lock--;

      /* emit signal to unlock the bar */
      if (plugin->priv->bar_lock == 0)
        blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (plugin),
                                                PROVIDER_SIGNAL_UNLOCK_BAR);
    }
}



static void
blade_bar_plugin_set_size (BladeBarPluginProvider *provider,
                            gint                     size)
{
  BladeBarPlugin *plugin = BLADE_BAR_PLUGIN (provider);
  gboolean         handled = FALSE;
  gint             real_size;

  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (provider));

  /* check if update is required, -1 for forced property emit
   * by blade_bar_plugin_set_nrows */
  if (G_LIKELY (plugin->priv->size != size))
    {
      if (size != -1)
        plugin->priv->size = size;

      real_size = plugin->priv->size * plugin->priv->nrows;

      g_signal_emit (G_OBJECT (plugin),
                     plugin_signals[SIZE_CHANGED], 0, real_size, &handled);

      /* handle the size when not done by the plugin */
      if (!handled)
        gtk_widget_set_size_request (GTK_WIDGET (plugin), real_size, real_size);

      g_object_notify_by_pspec (G_OBJECT (plugin), plugin_props[PROP_SIZE]);
    }
}



static void
blade_bar_plugin_set_mode (BladeBarPluginProvider *provider,
                            BladeBarPluginMode      mode)
{
  BladeBarPlugin *plugin = BLADE_BAR_PLUGIN (provider);
  GtkOrientation   old_orientation;
  GtkOrientation   new_orientation;

  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (provider));

  /* check if update is required */
  if (G_LIKELY (plugin->priv->mode != mode))
    {
      old_orientation = blade_bar_plugin_get_orientation (plugin);

      plugin->priv->mode = mode;

      g_signal_emit (G_OBJECT (plugin),
                     plugin_signals[MODE_CHANGED], 0, mode);

      g_object_notify_by_pspec (G_OBJECT (plugin), plugin_props[PROP_MODE]);

      /* emit old orientation property for compatibility */
      new_orientation = blade_bar_plugin_get_orientation (plugin);
      if (old_orientation != new_orientation)
        {
          g_signal_emit (G_OBJECT (plugin),
                         plugin_signals[ORIENTATION_CHANGED], 0, new_orientation);

          g_object_notify_by_pspec (G_OBJECT (plugin), plugin_props[PROP_ORIENTATION]);
        }
    }
}



static void
blade_bar_plugin_set_nrows (BladeBarPluginProvider *provider,
                             guint                    nrows)
{
  BladeBarPlugin *plugin = BLADE_BAR_PLUGIN (provider);

  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (provider));

  nrows = MAX (nrows, 1);

  /* check if update is required */
  if (G_LIKELY (plugin->priv->nrows != nrows))
    {
      plugin->priv->nrows = nrows;

      g_signal_emit (G_OBJECT (plugin),
                     plugin_signals[NROWS_CHANGED], 0, nrows);

      g_object_notify_by_pspec (G_OBJECT (plugin), plugin_props[PROP_NROWS]);

      /* also the size changed */
      blade_bar_plugin_set_size (provider, -1);
    }
}



static void
blade_bar_plugin_set_screen_position (BladeBarPluginProvider *provider,
                                       XfceScreenPosition       screen_position)
{
  BladeBarPlugin *plugin = BLADE_BAR_PLUGIN (provider);

  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (provider));

  /* check if update is required */
  if (G_LIKELY (plugin->priv->screen_position != screen_position
      || xfce_screen_position_is_floating (screen_position)))
    {
      plugin->priv->screen_position = screen_position;

      g_signal_emit (G_OBJECT (plugin),
                     plugin_signals[SCREEN_POSITION_CHANGED], 0,
                     screen_position);

      g_object_notify_by_pspec (G_OBJECT (plugin), plugin_props[PROP_SCREEN_POSITION]);
    }
}



static void
blade_bar_plugin_save (BladeBarPluginProvider *provider)
{
  BladeBarPlugin *plugin = BLADE_BAR_PLUGIN (provider);

  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (provider));

  /* only send the save signal if the plugin is not locked */
  if (BLADE_BAR_PLUGIN (provider)->priv->menu_blocked == 0
      && !blade_bar_plugin_get_locked (plugin))
    g_signal_emit (G_OBJECT (provider), plugin_signals[SAVE], 0);
}



static gboolean
blade_bar_plugin_get_show_configure (BladeBarPluginProvider *provider)
{
  bar_return_val_if_fail (BLADE_IS_BAR_PLUGIN (provider), FALSE);

  /* TODO, not sure, but maybe return FALSE when menu_blocked > 0 */

  return BAR_HAS_FLAG (BLADE_BAR_PLUGIN (provider)->priv->flags,
                         PLUGIN_FLAG_SHOW_CONFIGURE);
}



static void
blade_bar_plugin_show_configure (BladeBarPluginProvider *provider)
{
  BladeBarPlugin *plugin = BLADE_BAR_PLUGIN (provider);

  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (provider));

  if (plugin->priv->menu_blocked == 0
      && !blade_bar_plugin_get_locked (plugin))
    g_signal_emit (G_OBJECT (plugin), plugin_signals[CONFIGURE_PLUGIN], 0);
}



static gboolean
blade_bar_plugin_get_show_about (BladeBarPluginProvider *provider)
{
  bar_return_val_if_fail (BLADE_IS_BAR_PLUGIN (provider), FALSE);

  /* TODO, not sure, but maybe return FALSE when menu_blocked > 0 */

  return BAR_HAS_FLAG (BLADE_BAR_PLUGIN (provider)->priv->flags,
                         PLUGIN_FLAG_SHOW_ABOUT);
}



static void
blade_bar_plugin_show_about (BladeBarPluginProvider *provider)
{
  BladeBarPlugin *plugin = BLADE_BAR_PLUGIN (provider);

  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (provider));

  if (G_LIKELY (plugin->priv->menu_blocked == 0))
    g_signal_emit (G_OBJECT (provider), plugin_signals[ABOUT], 0);
}



static void
blade_bar_plugin_removed (BladeBarPluginProvider *provider)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (provider));

  if (!blade_bar_plugin_get_locked (BLADE_BAR_PLUGIN (provider)))
    g_signal_emit (G_OBJECT (provider), plugin_signals[REMOVED], 0);
}



static gboolean
blade_bar_plugin_remote_event (BladeBarPluginProvider *provider,
                                const gchar             *name,
                                const GValue            *value,
                                guint                   *handle)
{
  gboolean stop_emission;

  bar_return_val_if_fail (BLADE_IS_BAR_PLUGIN (provider), TRUE);
  bar_return_val_if_fail (name != NULL, TRUE);
  bar_return_val_if_fail (value == NULL || G_IS_VALUE (value), TRUE);

  g_signal_emit (G_OBJECT (provider), plugin_signals[REMOTE_EVENT], 0,
                 name, value, &stop_emission);

  return stop_emission;
}



static void
blade_bar_plugin_set_locked (BladeBarPluginProvider *provider,
                              gboolean                 locked)
{
  BladeBarPlugin *plugin = BLADE_BAR_PLUGIN (provider);

  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (provider));

  if (G_LIKELY (plugin->priv->locked != locked))
    {
      plugin->priv->locked = locked;

      /* destroy the menu if it exists */
      blade_bar_plugin_menu_destroy (plugin);
    }
}



static void
blade_bar_plugin_ask_remove (BladeBarPluginProvider *provider)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (provider));

  blade_bar_plugin_menu_remove (BLADE_BAR_PLUGIN (provider));
}



static void
blade_bar_plugin_take_window_notify (gpointer  data,
                                      GObject  *where_the_object_was)
{
  bar_return_if_fail (GTK_IS_WINDOW (data) || BLADE_IS_BAR_PLUGIN (data));

  /* release the opposite weak ref */
  g_object_weak_unref (G_OBJECT (data),
      blade_bar_plugin_take_window_notify, where_the_object_was);

  /* destroy the dialog if the plugin was finalized */
  if (GTK_IS_WINDOW (data))
    gtk_widget_destroy (GTK_WIDGET (data));
}



static void
blade_bar_plugin_menu_item_destroy (GtkWidget       *item,
                                     BladeBarPlugin *plugin)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  bar_return_if_fail (GTK_IS_MENU_ITEM (item));
  bar_return_if_fail (g_slist_find (plugin->priv->menu_items, item) != NULL);

  /* remote the item from the list and release it */
  plugin->priv->menu_items = g_slist_remove (plugin->priv->menu_items, item);
  g_object_unref (G_OBJECT (item));
}



/**
 * blade_bar_plugin_get_name:
 * @plugin : an #BladeBarPlugin.
 *
 * The internal name of the bar plugin.
 *
 * Returns: the name of the bar plugin.
 **/
const gchar *
blade_bar_plugin_get_name (BladeBarPlugin *plugin)
{
  g_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), NULL);
  g_return_val_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin), NULL);

  return plugin->priv->name;
}



/**
 * blade_bar_plugin_get_display_name:
 * @plugin : an #BladeBarPlugin.
 *
 * This returns the translated name of the plugin set in the .desktop
 * file of the plugin.
 *
 * Returns: the (translated) display name of the plugin.
 **/
const gchar *
blade_bar_plugin_get_display_name (BladeBarPlugin *plugin)
{
  g_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), NULL);
  g_return_val_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin), NULL);

  if (G_LIKELY (plugin->priv->display_name))
    return plugin->priv->display_name;

  return plugin->priv->name;
}



/**
 * blade_bar_plugin_get_comment:
 * @plugin : an #BladeBarPlugin.
 *
 * This returns the translated comment of the plugin set in
 * the .desktop file of the plugin.
 *
 * Returns: the (translated) comment of the plugin.
 *
 * Since: 4.8
 **/
const gchar *
blade_bar_plugin_get_comment (BladeBarPlugin *plugin)
{
  g_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), NULL);
  g_return_val_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin), NULL);

  return plugin->priv->comment;
}



/**
 * blade_bar_plugin_get_unique_id:
 * @plugin : an #BladeBarPlugin.
 *
 * The internal unique id of the plugin. Each plugin in the bar has
 * a unique number that is for example used for the config file name
 * or property base in the blconf channel.
 *
 * Returns: the unique id of the plugin.
 *
 * Since 4.8
 **/
gint
blade_bar_plugin_get_unique_id (BladeBarPlugin *plugin)
{
  g_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), -1);
  g_return_val_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin), -1);

  return plugin->priv->unique_id;
}



/**
 * blade_bar_plugin_get_property_base:
 * @plugin : an #BladeBarPlugin.
 *
 * The property base for this plugin in the blade-bar BlconfChannel,
 * this name is something like /plugins/plugin-1.
 *
 * Returns: the property base for the blconf channel userd by a plugin.
 *
 * See also: blconf_channel_new_with_property_base.
 *           BLADE_BAR_PLUGIN_CHANNEL_NAME and
 *           blade_bar_get_channel_name
 **/
const gchar *
blade_bar_plugin_get_property_base (BladeBarPlugin *plugin)
{
  g_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), NULL);
  g_return_val_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin), NULL);

  /* create the propert if needed */
  if (plugin->priv->property_base == NULL)
    plugin->priv->property_base = g_strdup_printf (BAR_PLUGIN_PROPERTY_BASE,
                                                   plugin->priv->unique_id);

  return plugin->priv->property_base;
}



/**
 * blade_bar_plugin_get_arguments:
 * @plugin : an #BladeBarPlugin.
 *
 * Argument vector passed to the plugin when it was added. Most of the
 * time the return value will be %NULL, but if could for example contain
 * a list of filenames when the user added the plugin with
 *
 * blade-bar --add=launcher *.desktop
 *
 * see the code of the launcher plugin how to use this.
 *
 * Returns: the argument vector. The vector is owned by the plugin and
 *          should not be freed.
 *
 * Since: 4.8
 **/
const gchar * const *
blade_bar_plugin_get_arguments (BladeBarPlugin *plugin)
{
  g_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), NULL);
  g_return_val_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin), NULL);

  return (const gchar * const *) plugin->priv->arguments;
}



/**
 * blade_bar_plugin_get_size:
 * @plugin : an #BladeBarPlugin.
 *
 * The size of the bar in which the plugin is embedded.
 *
 * Returns: the current size of the bar.
 **/
gint
blade_bar_plugin_get_size (BladeBarPlugin *plugin)
{
  gint real_size;

  g_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), -1);
  g_return_val_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin), -1);

  /* always return a 'positive' size that makes sence */
  real_size = plugin->priv->size * plugin->priv->nrows;
  return MAX (16, real_size);
}



/**
 * blade_bar_plugin_get_expand:
 * @plugin : an #BladeBarPlugin.
 *
 * Whether the plugin is expanded or not. This set by the plugin using
 * blade_bar_plugin_set_expand().
 *
 * Returns: %TRUE when the plugin should expand,
 *          %FALSE otherwise.
 **/
gboolean
blade_bar_plugin_get_expand (BladeBarPlugin *plugin)
{
  g_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), FALSE);
  g_return_val_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin), FALSE);

  return plugin->priv->expand;
}



/**
 * blade_bar_plugin_set_expand:
 * @plugin : an #BladeBarPlugin.
 * @expand : whether to expand the plugin.
 *
 * Wether the plugin should expand of not
 **/
void
blade_bar_plugin_set_expand (BladeBarPlugin *plugin,
                              gboolean         expand)
{
  g_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  g_return_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin));

  /* normalize the value */
  expand = !!expand;

  /* check if update is required */
  if (G_LIKELY (plugin->priv->expand != expand))
    {
      plugin->priv->expand = expand;

      /* emit signal (in provider) */
      blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (plugin),
                                              expand ? PROVIDER_SIGNAL_EXPAND_PLUGIN :
                                                  PROVIDER_SIGNAL_COLLAPSE_PLUGIN);

      g_object_notify_by_pspec (G_OBJECT (plugin), plugin_props[PROP_EXPAND]);
    }
}



/**
 * blade_bar_plugin_get_shrink:
 * @plugin : an #BladeBarPlugin.
 *
 * Whether the plugin can shrink if the size on the bar is limited. This
 * is effective with plugins that do not have expand set, but can accept
 * a smaller size when needed.
 *
 * Returns: %TRUE when the plugin can shrink,
 *          %FALSE otherwise.
 *
 * Since: 4.10
 **/
gboolean
blade_bar_plugin_get_shrink (BladeBarPlugin *plugin)
{
  g_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), FALSE);
  g_return_val_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin), FALSE);

  return plugin->priv->shrink;
}



/**
 * blade_bar_plugin_set_shrink:
 * @plugin : an #BladeBarPlugin.
 * @shrink : whether the plugin can shrink.
 *
 * Wether the plugin can shrink if the size on the bar
 * is limited. This does not work if the plugin is expanded.
 **/
void
blade_bar_plugin_set_shrink (BladeBarPlugin *plugin,
                              gboolean         shrink)
{
  g_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  g_return_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin));

  /* normalize the value */
  shrink = !!shrink;

  /* check if update is required */
  if (G_LIKELY (plugin->priv->shrink != shrink))
    {
      plugin->priv->shrink = shrink;

      /* emit signal (in provider) */
      blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (plugin),
                                              shrink ? PROVIDER_SIGNAL_SHRINK_PLUGIN :
                                                  PROVIDER_SIGNAL_UNSHRINK_PLUGIN);

      g_object_notify_by_pspec (G_OBJECT (plugin), plugin_props[PROP_SHRINK]);
    }
}


/**
 * blade_bar_plugin_get_small:
 * @plugin : an #BladeBarPlugin.
 *
 * Whether the plugin is small enough to fit in a single row of
 * a multi-row bar. E.g. if it is a button-like applet.
 *
 * Returns: %TRUE when the plugin is small,
 *          %FALSE otherwise.
 *
 * Since: 4.10
 **/
gboolean
blade_bar_plugin_get_small (BladeBarPlugin *plugin)
{
  g_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), FALSE);
  g_return_val_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin), FALSE);

  return plugin->priv->small;
}



/**
 * blade_bar_plugin_set_small:
 * @plugin : an #BladeBarPlugin.
 * @small : whether the plugin is a small button-like applet.
 *
 * Whether the plugin is small enough to fit in a single row of
 * a multi-row bar. E.g. if it is a button-like applet.
 **/
void
blade_bar_plugin_set_small (BladeBarPlugin *plugin,
                             gboolean         small)
{
  g_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  g_return_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin));

  /* normalize the value */
  small = !!small;

  /* check if update is required */
  if (G_LIKELY (plugin->priv->small != small))
    {
      plugin->priv->small = small;

      /* emit signal (in provider) */
      blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (plugin),
                                              small ? PROVIDER_SIGNAL_SMALL_PLUGIN :
                                              PROVIDER_SIGNAL_UNSMALL_PLUGIN);

      g_object_notify_by_pspec (G_OBJECT (plugin), plugin_props[PROP_SMALL]);
    }
}



/**
 * blade_bar_plugin_get_orientation:
 * @plugin : an #BladeBarPlugin.
 *
 * The orientation of the bar in which the plugin is embedded.
 *
 * Returns: the current #GtkOrientation of the bar.
 **/
GtkOrientation
blade_bar_plugin_get_orientation (BladeBarPlugin *plugin)
{
  g_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), GTK_ORIENTATION_HORIZONTAL);

  if (plugin->priv->mode == BLADE_BAR_PLUGIN_MODE_HORIZONTAL)
    return GTK_ORIENTATION_HORIZONTAL;
  else
    return GTK_ORIENTATION_VERTICAL;
}



/**
 * blade_bar_plugin_get_mode:
 * @plugin : an #BladeBarPlugin.
 *
 * The mode of the bar in which the plugin is embedded.
 *
 * Returns: the current #BladeBarPluginMode of the bar.
 *
 * Since: 4.10
 **/
BladeBarPluginMode
blade_bar_plugin_get_mode (BladeBarPlugin *plugin)
{
  g_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), BLADE_BAR_PLUGIN_MODE_HORIZONTAL);
  g_return_val_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin), BLADE_BAR_PLUGIN_MODE_HORIZONTAL);

  return plugin->priv->mode;
}



/**
 * blade_bar_plugin_get_nrows:
 * @plugin : an #BladeBarPlugin.
 *
 * The number of rows of the bar in which the plugin is embedded.
 *
 * Returns: the current number of rows of the bar.
 *
 * Since: 4.10
 **/
guint
blade_bar_plugin_get_nrows (BladeBarPlugin *plugin)
{
  g_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), 1);
  g_return_val_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin), 1);

  return plugin->priv->nrows;
}



/**
 * blade_bar_plugin_get_screen_position:
 * @plugin : an #BladeBarPlugin.
 *
 * The screen position of the bar in which the plugin is embedded.
 *
 * Returns: the current #XfceScreenPosition of the bar.
 **/
XfceScreenPosition
blade_bar_plugin_get_screen_position (BladeBarPlugin *plugin)
{
  g_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), XFCE_SCREEN_POSITION_NONE);
  g_return_val_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin), XFCE_SCREEN_POSITION_NONE);

  return plugin->priv->screen_position;
}



/**
 * blade_bar_plugin_take_window:
 * @plugin : an #BladeBarPlugin.
 * @window : a #GtkWindow.
 *
 * Connect a dialog to a plugin. When the @plugin is closed, it will
 * destroy the @window.
 *
 * Since: 4.8
 **/
void
blade_bar_plugin_take_window (BladeBarPlugin *plugin,
                               GtkWindow       *window)
{
  g_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  g_return_if_fail (GTK_IS_WINDOW (window));

  gtk_window_set_screen (window, gtk_widget_get_screen (GTK_WIDGET (plugin)));

  /* monitor both objects */
  g_object_weak_ref (G_OBJECT (plugin),
      blade_bar_plugin_take_window_notify, window);
  g_object_weak_ref (G_OBJECT (window),
      blade_bar_plugin_take_window_notify, plugin);
}



/**
 * blade_bar_plugin_add_action_widget:
 * @plugin : an #BladeBarPlugin.
 * @widget : a #GtkWidget that receives mouse events.
 *
 * Attach the plugin menu to this widget. Plugin writers should call this
 * for every widget that can receive mouse events. If you forget to call this
 * the plugin will not have a right-click menu and the user won't be able to
 * remove it.
 **/
void
blade_bar_plugin_add_action_widget (BladeBarPlugin *plugin,
                                     GtkWidget       *widget)
{
  g_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  g_signal_connect_swapped (G_OBJECT (widget), "button-press-event",
      G_CALLBACK (blade_bar_plugin_button_press_event), plugin);
}



/**
 * blade_bar_plugin_menu_insert_item:
 * @plugin : an #BladeBarPlugin.
 * @item   : a #GtkMenuItem.
 *
 * Insert a custom menu item to the plugin's right click menu. This item
 * is packed below the "Move" menu item.
 **/
void
blade_bar_plugin_menu_insert_item (BladeBarPlugin *plugin,
                                    GtkMenuItem     *item)
{
  g_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  g_return_if_fail (GTK_IS_MENU_ITEM (item));

  /* take the item and add to internal list */
  plugin->priv->menu_items = g_slist_append (plugin->priv->menu_items,
                                             g_object_ref_sink (item));
  g_signal_connect (G_OBJECT (item), "destroy",
      G_CALLBACK (blade_bar_plugin_menu_item_destroy), plugin);

  /* destroy the menu */
  blade_bar_plugin_menu_destroy (plugin);
}



/**
 * blade_bar_plugin_menu_show_configure:
 * @plugin : an #BladeBarPlugin.
 *
 * Show the "Properties" item in the menu. Clicking on the menu item
 * will emit the "configure-plugin" signal.
 **/
void
blade_bar_plugin_menu_show_configure (BladeBarPlugin *plugin)
{
  GtkMenu   *menu;
  GtkWidget *item;

  g_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  g_return_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin));

  BAR_SET_FLAG (plugin->priv->flags, PLUGIN_FLAG_SHOW_CONFIGURE);

  /* show the menu item if the menu is already generated */
  if (G_UNLIKELY (plugin->priv->menu != NULL))
    {
       /* get and show the properties item */
       menu = blade_bar_plugin_menu_get (plugin);
       item = g_object_get_qdata (G_OBJECT (menu), item_properties);
       if (G_LIKELY (item != NULL))
         gtk_widget_show (item);
    }

  /* emit signal, used by the external plugin */
  blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (plugin),
                                          PROVIDER_SIGNAL_SHOW_CONFIGURE);
}



/**
 * blade_bar_plugin_menu_show_about:
 * @plugin : an #BladeBarPlugin.
 *
 * Show the "About" item in the menu. Clicking on the menu item
 * will emit the "about" signal.
 **/
void
blade_bar_plugin_menu_show_about (BladeBarPlugin *plugin)
{
  GtkMenu   *menu;
  GtkWidget *item;

  g_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  g_return_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin));

  BAR_SET_FLAG (plugin->priv->flags, PLUGIN_FLAG_SHOW_ABOUT);

  /* show the menu item if the menu is already generated */
  if (G_UNLIKELY (plugin->priv->menu != NULL))
    {
       /* get and show the about item */
       menu = blade_bar_plugin_menu_get (plugin);
       item = g_object_get_qdata (G_OBJECT (menu), item_about);
       if (G_LIKELY (item != NULL))
         gtk_widget_show (item);
    }

  /* emit signal, used by the external plugin */
  blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (plugin),
                                          PROVIDER_SIGNAL_SHOW_ABOUT);
}



/**
 * blade_bar_plugin_get_locked:
 * @plugin : an #BladeBarPlugin.
 *
 * Whether the plugin is locked (not allowing customization). This
 * is emitted through the bar based on the Blconf locking of the
 * bar window the plugin is embedded on.
 *
 * It is however possible to send a fake signal to the plugin to
 * override this propery, so you should only use this for interface
 * elements and (if you use Blconf) check the locking yourself
 * before you write any values or query the kiosk mode using the
 * api in libbladeutil.
 *
 * Returns: %TRUE if the user is not allowed to modify the plugin,
 *          %FALSE is customization is allowed.
 *
 * Since: 4.8
 **/
gboolean
blade_bar_plugin_get_locked (BladeBarPlugin *plugin)
{
  g_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), TRUE);

  return plugin->priv->locked;
}



/**
 * blade_bar_plugin_remove:
 * @plugin : an #BladeBarPlugin.
 *
 * Remove this plugin from the bar and remove all its configuration.
 *
 * Plugins should not use this function to implement their own
 * menu item or button to remove theirselfs from the bar, but only
 * in case the there are problems with the plugin in the bar. Always
 * try to inform the user why this occured.
 *
 * Since: 4.8
 **/
void
blade_bar_plugin_remove (BladeBarPlugin *plugin)
{
  g_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));

  /* ask the bar or wrapper to remove the plugin */
  blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (plugin),
                                          PROVIDER_SIGNAL_REMOVE_PLUGIN);
}



/**
 * blade_bar_plugin_block_menu:
 * @plugin : an #BladeBarPlugin.
 *
 * Block configuring the plugin. This will make the "Properties" menu
 * item insensitive.
 **/
void
blade_bar_plugin_block_menu (BladeBarPlugin *plugin)
{
  g_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));

  /* increase block counter */
  plugin->priv->menu_blocked++;
}



/**
 * blade_bar_plugin_unblock_menu:
 * @plugin : an #BladeBarPlugin.
 *
 * Unblock configuring the plugin. This will make the "Properties" menu
 * item sensitive.
 **/
void
blade_bar_plugin_unblock_menu (BladeBarPlugin *plugin)
{
  g_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  g_return_if_fail (plugin->priv->menu_blocked > 0);

  /* decrease block counter */
  if (G_LIKELY (plugin->priv->menu_blocked > 0))
    plugin->priv->menu_blocked--;
}



/**
 * blade_bar_plugin_register_menu:
 * @plugin : an #BladeBarPlugin.
 * @menu   : a #GtkMenu that will be opened
 *
 * Register a menu that is about to popup. This will make sure the bar
 * will properly handle its autohide behaviour. You have to call this
 * function every time the menu is opened (e.g. using gtk_popup_menu()).
 *
 * If you want to open the menu aligned to the side of the bar (and the
 * plugin), you should use blade_bar_plugin_position_menu() as
 * #GtkMenuPositionFunc. This callback function will take care of calling
 * blade_bar_plugin_register_menu() as well.
 *
 * See also: blade_bar_plugin_position_menu() and blade_bar_plugin_block_autohide().
 **/
void
blade_bar_plugin_register_menu (BladeBarPlugin *plugin,
                                 GtkMenu         *menu)
{
  g_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  g_return_if_fail (GTK_IS_MENU (menu));
  g_return_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin));

  /* increase the counter */
  plugin->priv->bar_lock++;

  /* connect signal to menu to decrease counter */
  g_signal_connect (G_OBJECT (menu), "deactivate",
      G_CALLBACK (blade_bar_plugin_unregister_menu), plugin);
  g_signal_connect (G_OBJECT (menu), "destroy",
      G_CALLBACK (blade_bar_plugin_unregister_menu), plugin);

  /* tell bar it needs to lock */
  if (plugin->priv->bar_lock == 1)
    blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (plugin),
                                            PROVIDER_SIGNAL_LOCK_BAR);
}



/**
 * blade_bar_plugin_arrow_type:
 * @plugin : an #BladeBarPlugin.
 *
 * Determine the #GtkArrowType for a widget that opens a menu and uses
 * blade_bar_plugin_position_menu() to position the menu.
 *
 * Returns: the #GtkArrowType to use.
 **/
GtkArrowType
blade_bar_plugin_arrow_type (BladeBarPlugin *plugin)
{
  XfceScreenPosition  screen_position;
  GdkScreen          *screen;
  gint                monitor_num;
  GdkRectangle        monitor;
  gint                x, y;
  GdkWindow          *window;

  g_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), GTK_ARROW_NONE);
  g_return_val_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin), GTK_ARROW_NONE);

  /* get the plugin screen position */
  screen_position = blade_bar_plugin_get_screen_position (plugin);

  /* detect the arrow type */
  if (xfce_screen_position_is_top (screen_position))
    return GTK_ARROW_DOWN;
  else if (xfce_screen_position_is_bottom (screen_position))
    return GTK_ARROW_UP;
  else if (xfce_screen_position_is_left (screen_position))
    return GTK_ARROW_RIGHT;
  else if (xfce_screen_position_is_right (screen_position))
    return GTK_ARROW_LEFT;
  else /* floating */
    {
      window = gtk_widget_get_window (GTK_WIDGET (plugin));
      if (G_UNLIKELY (window == NULL))
        return GTK_ARROW_NONE;

      /* get the monitor geometry */
      screen = gtk_widget_get_screen (GTK_WIDGET (plugin));
      monitor_num = gdk_screen_get_monitor_at_window (screen, window);
      gdk_screen_get_monitor_geometry (screen, monitor_num, &monitor);

      /* get the plugin root origin */
      gdk_window_get_root_origin (window, &x, &y);

      /* detect arrow type */
      if (screen_position == XFCE_SCREEN_POSITION_FLOATING_H)
        return (y < (monitor.y + monitor.height / 2)) ? GTK_ARROW_DOWN : GTK_ARROW_UP;
      else
        return (x < (monitor.x + monitor.width / 2)) ? GTK_ARROW_RIGHT : GTK_ARROW_LEFT;
    }
}



/**
 * blade_bar_plugin_position_widget:
 * @plugin        : an #BladeBarPlugin.
 * @menu_widget   : a #GtkWidget that will be used as popup menu.
 * @attach_widget : a #GtkWidget relative to which the menu should be positioned.
 * @x             : return location for the x coordinate.
 * @y             : return location for the x coordinate.
 *
 * The menu widget is positioned relative to @attach_widget.
 * If @attach_widget is NULL, the menu widget is instead positioned
 * relative to @bar_plugin.
 *
 * This function is intended for custom menu widgets.
 * For a regular #GtkMenu you should use blade_bar_plugin_position_menu()
 * instead (as callback argument to gtk_menu_popup()).
 *
 * See also: blade_bar_plugin_position_menu().
 **/
void
blade_bar_plugin_position_widget (BladeBarPlugin *plugin,
                                   GtkWidget       *menu_widget,
                                   GtkWidget       *attach_widget,
                                   gint            *x,
                                   gint            *y)
{
  GtkRequisition  requisition;
  GdkScreen      *screen;
  GdkRectangle    monitor;
  gint            monitor_num;
  GTimeVal        now_t, end_t;
  GtkWidget      *toplevel, *plug;
  gint            px, py;
  GtkAllocation   alloc;

  g_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  g_return_if_fail (GTK_IS_WIDGET (menu_widget));
  g_return_if_fail (attach_widget == NULL || GTK_IS_WIDGET (attach_widget));
  g_return_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin));

  /* if the attach widget is null, use the bar plugin */
  if (attach_widget == NULL)
    attach_widget = GTK_WIDGET (plugin);

  /* make sure the menu is realized to get valid rectangle sizes */
  if (!gtk_widget_get_realized (menu_widget))
    gtk_widget_realize (menu_widget);

  /* make sure the attach widget is realized for the gdkwindow */
  if (!gtk_widget_get_realized (attach_widget))
    gtk_widget_realize (attach_widget);

  /* get the menu/widget size request */
#if GTK_CHECK_VERSION (3, 0, 0)
  gtk_widget_get_preferred_size (menu_widget, &requisition, NULL);
#else
  gtk_widget_size_request (menu_widget, &requisition);
#endif

  /* get the root position of the attach widget */
  toplevel = gtk_widget_get_toplevel (attach_widget);
  gtk_window_get_position (GTK_WINDOW (toplevel), x, y);

  /* correct position for external plugins */
  plug = gtk_widget_get_ancestor (attach_widget, GTK_TYPE_PLUG);
  if (plug != NULL)
    {
#if GTK_CHECK_VERSION (3, 0, 0)
       gdk_window_get_geometry (gtk_plug_get_socket_window (GTK_PLUG (plug)),
                                &px, &py, NULL, NULL);
#else
       gdk_window_get_geometry (gtk_plug_get_socket_window (GTK_PLUG (plug)),
                                &px, &py, NULL, NULL, NULL);
#endif

       *x += px;
       *y += py;
    }

  /* if the bar is hidden (auto hide is enabled) and we requested a
   * bar lock, wait for gtk to position the bar before we actually
   * use the coordinates */
  if (plugin->priv->bar_lock > 0)
    {
      g_get_current_time (&end_t);
      g_time_val_add (&end_t, G_USEC_PER_SEC / 2);

      while (*x == -9999 && *y == -9999)
        {
          while (gtk_events_pending ())
            gtk_main_iteration ();

          gdk_window_get_position (gtk_widget_get_window (attach_widget), x, y);

          /* don't try longer then 1/2 a second */
          g_get_current_time (&now_t);
          if (now_t.tv_sec > end_t.tv_sec
              || (now_t.tv_sec == end_t.tv_sec
                  && now_t.tv_usec > end_t.tv_usec))
            break;
        }
    }

  /* add the widgets allocation */
  gtk_widget_get_allocation (attach_widget, &alloc);
  *x += alloc.x;
  *y += alloc.y;

  switch (blade_bar_plugin_arrow_type (plugin))
    {
    case GTK_ARROW_UP:
      *y -= requisition.height;
      break;

    case GTK_ARROW_DOWN:
      *y += alloc.height;
      break;

    case GTK_ARROW_LEFT:
      *x -= requisition.width;
      break;

    default: /* GTK_ARROW_RIGHT and GTK_ARROW_NONE */
      *x += alloc.width;
      break;
    }

  /* get the monitor geometry */
  screen = gtk_widget_get_screen (attach_widget);
  monitor_num = gdk_screen_get_monitor_at_window (screen, gtk_widget_get_window (attach_widget));
  gdk_screen_get_monitor_geometry (screen, monitor_num, &monitor);

  /* keep the menu inside the screen */
  if (*x > monitor.x + monitor.width - requisition.width)
    *x = monitor.x + monitor.width - requisition.width;
  if (*x < monitor.x)
    *x = monitor.x;
  if (*y > monitor.y + monitor.height - requisition.height)
    *y = monitor.y + monitor.height - requisition.height;
  if (*y < monitor.y)
    *y = monitor.y;

  /* popup on the correct screen */
  if (G_LIKELY (GTK_IS_MENU (menu_widget)))
    gtk_menu_set_screen (GTK_MENU (menu_widget), screen);
  else if (GTK_IS_WINDOW (menu_widget))
    gtk_window_set_screen (GTK_WINDOW (menu_widget), screen);
}



/**
 * blade_bar_plugin_position_menu:
 * @menu         : a #GtkMenu.
 * @x            : return location for the x coordinate.
 * @y            : return location for the y coordinate.
 * @push_in      : keep inside the screen (see #GtkMenuPositionFunc)
 * @bar_plugin : an #BladeBarPlugin.
 *
 * Function to be used as #GtkMenuPositionFunc in a call to gtk_menu_popup().
 * As data argument it needs an #BladeBarPlugin.
 *
 * The menu is normally positioned relative to @bar_plugin. If you want the
 * menu to be positioned relative to another widget, you can use
 * gtk_menu_attach_to_widget() to explicitly set a 'parent' widget.
 *
 * As a convenience, blade_bar_plugin_position_menu() calls
 * blade_bar_plugin_register_menu() for the menu.
 *
 * <example>
 * void
 * myplugin_popup_menu (BladeBarPlugin *plugin,
 *                      GtkMenu         *menu,
 *                      GdkEventButton  *ev)
 * {
 *     gtk_menu_popup (menu, NULL, NULL,
 *                     blade_bar_plugin_position_menu, plugin,
 *                     ev->button, ev->time );
 * }
 * </example>
 *
 * For a custom widget that will be used as a popup menu, use
 * blade_bar_plugin_position_widget() instead.
 *
 * See also: gtk_menu_popup().
 **/
void
blade_bar_plugin_position_menu (GtkMenu  *menu,
                                 gint     *x,
                                 gint     *y,
                                 gboolean *push_in,
                                 gpointer  bar_plugin)
{
  GtkWidget *attach_widget;

  g_return_if_fail (BLADE_IS_BAR_PLUGIN (bar_plugin));
  g_return_if_fail (GTK_IS_MENU (menu));
  g_return_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (bar_plugin));

  /* register the menu */
  blade_bar_plugin_register_menu (BLADE_BAR_PLUGIN (bar_plugin), menu);

  /* calculate the coordinates */
  attach_widget = gtk_menu_get_attach_widget (menu);
  blade_bar_plugin_position_widget (BLADE_BAR_PLUGIN (bar_plugin),
                                     GTK_WIDGET (menu), attach_widget, x, y);

#if GTK_CHECK_VERSION (3, 0, 0)
  /* FIXME */
  /* A workaround for Gtk3 popup menus with scroll buttons */
  /* Menus are "pushed in" anyway */
  *push_in = FALSE;
#else
  /* keep the menu inside screen */
  *push_in = TRUE;
#endif
}



/**
 * blade_bar_plugin_focus_widget:
 * @plugin : an #BladeBarPlugin.
 * @widget : a #GtkWidget inside the plugins that should be focussed.
 *
 * Grab the focus on @widget. Asks the bar to allow focus on its items
 * and set the focus to the requested widget.
 **/
void
blade_bar_plugin_focus_widget (BladeBarPlugin *plugin,
                                GtkWidget       *widget)
{
  g_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin));

  /* focus the bar window */
  blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (plugin),
                                          PROVIDER_SIGNAL_FOCUS_PLUGIN);

  /* let the widget grab focus */
  gtk_widget_grab_focus (widget);
}



/**
 * blade_bar_plugin_block_autohide:
 * @plugin  : an #BladeBarPlugin.
 * @blocked : new blocking state of this plugin.
 *
 * Wether this plugin blocks the autohide functality of the bar. Use
 * this when you 'popup' something that is visually attached to the
 * plugin at it will look weird for a user if the bar will hide while
 * he/she is working in the popup.
 *
 * For menus it there is blade_bar_plugin_register_menu() which will
 * take care of this.
 **/
void
blade_bar_plugin_block_autohide (BladeBarPlugin *plugin,
                                  gboolean         blocked)
{
  g_return_if_fail (BLADE_IS_BAR_PLUGIN (plugin));
  g_return_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin));

  /* leave when requesting the same block state */
  if (BAR_HAS_FLAG (plugin->priv->flags, PLUGIN_FLAG_BLOCK_AUTOHIDE) == blocked)
    return;

  if (blocked)
    {
      /* increase the counter */
      bar_return_if_fail (plugin->priv->bar_lock >= 0);
      plugin->priv->bar_lock++;

      BAR_SET_FLAG (plugin->priv->flags, PLUGIN_FLAG_BLOCK_AUTOHIDE);

      /* tell bar it needs to lock */
      if (plugin->priv->bar_lock == 1)
        blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (plugin),
                                                PROVIDER_SIGNAL_LOCK_BAR);
    }
  else
    {
      /* decrease the counter */
      bar_return_if_fail (plugin->priv->bar_lock > 0);
      plugin->priv->bar_lock--;

      BAR_UNSET_FLAG (plugin->priv->flags, PLUGIN_FLAG_BLOCK_AUTOHIDE);

      /* tell bar it needs to unlock */
      if (plugin->priv->bar_lock == 0)
        blade_bar_plugin_provider_emit_signal (BLADE_BAR_PLUGIN_PROVIDER (plugin),
                                                PROVIDER_SIGNAL_UNLOCK_BAR);
    }
}



/**
 * blade_bar_plugin_lookup_rc_file:
 * @plugin : an #BladeBarPlugin.
 *
 * Looks for the plugin resource file. This should be used to get the
 * plugin read location of the config file. You should only use the
 * returned path to read information from, since it might point to a
 * not-writable file (in kiosk mode for example).
 *
 * See also: blade_bar_plugin_save_location() and xfce_resource_lookup()
 *
 * Returns: The path to a config file or %NULL if no file was found.
 *          The returned string must be freed using g_free()
 **/
gchar *
blade_bar_plugin_lookup_rc_file (BladeBarPlugin *plugin)
{
  gchar *filename, *path;

  g_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), NULL);
  g_return_val_if_fail (BLADE_BAR_PLUGIN_CONSTRUCTED (plugin), NULL);

  filename = blade_bar_plugin_relative_filename (plugin);
  path = xfce_resource_lookup (XFCE_RESOURCE_CONFIG, filename);
  g_free (filename);

  return path;
}



/**
 * blade_bar_plugin_save_location:
 * @plugin : an #BladeBarPlugin.
 * @create : whether to create missing directories.
 *
 * Returns the path that can be used to store configuration information.
 * Don't use this function if you want to read from the config file, but
 * use blade_bar_plugin_lookup_rc_file() instead.
 *
 * See also: blade_bar_plugin_lookup_rc_file() and xfce_resource_save_location()
 *
 * Returns: The path to a config file or %NULL if no file was found.
 *          The returned string must be freed u sing g_free().
 **/
gchar *
blade_bar_plugin_save_location (BladeBarPlugin *plugin,
                                 gboolean         create)
{
  gchar *filename, *path;

  g_return_val_if_fail (BLADE_IS_BAR_PLUGIN (plugin), NULL);

  filename = blade_bar_plugin_relative_filename (plugin);
  path = xfce_resource_save_location (XFCE_RESOURCE_CONFIG, filename, create);
  g_free (filename);

  return path;
}



#define __BLADE_BAR_PLUGIN_C__
#include <libbladebar/libbladebar-aliasdef.c>
