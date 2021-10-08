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

#ifndef __BLADE_BAR_MACROS_H__
#define __BLADE_BAR_MACROS_H__

#include <glib.h>
#include <glib-object.h>
#include <libbladebar/blade-bar-plugin.h>
#include <libbladebar/blade-bar-macros-46.h>

G_BEGIN_DECLS



/**
 * SECTION: macros
 * @title: Commonly used plugin macros
 * @short_description: Various macros to make life easier for plugin developers.
 * @include: libbladebar/libbladebar.h
 *
 * Some commonly used macros in bar plugins.
 **/

/**
 * SECTION: register-plugins
 * @title: Bar Plugin Register Macros
 * @short_description: Register bar plugins that are compiled as modules.
 * @include: libbladebar/libbladebar.h
 *
 * Macros to register bar plugins that are compiled as modules. Internal or
 * external is defined by the boolean key <varname>X-XFCE-Internal</varname>
 * in the plugin's .desktop file.
 **/

/**
 * SECTION: register-gobject-plugins
 * @title: GObject Oriented Bar Plugin Registers Macros
 * @short_description: Macros to register bar plugins, written as GObjects.
 * @include: libbladebar/libbladebar.h
 *
 * Macros to register bar plugin types and custom types inside bar plugins.
 **/

/**
 * BladeBarTypeModule:
 *
 * Typedef for GTypeModule for possible future expansion.
 *
 * Since: 4.8
 **/
typedef GTypeModule BladeBarTypeModule;



/**
 * BLADE_BAR_CHANNEL_NAME:
 *
 * Macro to return the value of blade_bar_get_channel_name().
 *
 * See also: blade_bar_plugin_blconf_channel_new,
 *           blade_bar_plugin_get_property_base
 *
 * Since: 4.8
 **/
#define BLADE_BAR_CHANNEL_NAME (blade_bar_get_channel_name ())



/**
 * blade_bar_plugin_blconf_channel_new:
 * @plugin : An #BladeBarPlugin.
 *
 * Convienient function for opening an BlconfChannel for a plugin. The
 * channel's property base will be propery returned from
 * blade_bar_plugin_get_property_base().
 *
 * See also: blade_bar_plugin_get_property_base,
 *           BLADE_BAR_PLUGIN_CHANNEL_NAME and
 *           blade_bar_get_channel_name
 *
 * Since: 4.8
 **/
#define blade_bar_plugin_blconf_channel_new(plugin) \
  blconf_channel_new_with_property_base (BLADE_BAR_CHANNEL_NAME, \
    blade_bar_plugin_get_property_base (BLADE_BAR_PLUGIN (plugin)))



/**
 * BLADE_BAR_DEFINE_PLUGIN:
 * @TypeName  : The name of the new type, in Camel case.
 * @type_name : The name of the new type, in lowercase, with words separated by '_'.
 * @args...   : Optional list of *_register_type() function from other
 *              objects in the plugin created with #BLADE_BAR_DEFINE_TYPE.
 *
 * Define a new (resident) GObject bar plugin, the parent type of the object
 * should be XFCE_TYPE_BAR_PLUGIN.
 *
 * Since: 4.8
 **/
#define BLADE_BAR_DEFINE_PLUGIN(TypeName, type_name, args...) \
  _XPP_DEFINE_PLUGIN (TypeName, type_name, FALSE, args)



/**
 * BLADE_BAR_DEFINE_PLUGIN_RESIDENT:
 * @TypeName  : The name of the new type, in Camel case.
 * @type_name : The name of the new type, in lowercase, with words separated by '_'.
 * @args...   : Optional list of *_register_type() function from other
 *              objects in the plugin created with #BLADE_BAR_DEFINE_TYPE.
 *
 * Same as #BLADE_BAR_DEFINE_PLUGIN, but if you use special libraries or objects,
 * it is possible the plugin will give problems when unloading the library,
 * a resident plugin will never be unloaded after the first load, avoiding
 * those issues.
 *
 * Since: 4.8
 **/
#define BLADE_BAR_DEFINE_PLUGIN_RESIDENT(TypeName, type_name, args...) \
  _XPP_DEFINE_PLUGIN (TypeName, type_name, TRUE, args)



/**
 * BLADE_BAR_DEFINE_TYPE:
 * @TypeName    : The name of the new type, in Camel case.
 * @type_name   : The name of the new type, in lowercase, with words separated by '_'.
 * @TYPE_PARENT : The GType of the parent type.
 *
 * A convenient macro of #G_DEFINE_DYNAMIC_TYPE for bar plugins. Only
 * difference with #G_DEFINE_DYNAMIC_TYPE is that the type name send to
 * g_type_module_register_type() is prefixed with "Xfce". This allows you
 * use use shorted structure names in the code, while the real name of the
 * object is a full "Xfce" name.
 *
 * The _register_type function should be added to the args in
 * #BLADE_BAR_DEFINE_PLUGIN.
 *
 * Since: 4.8
 **/
#define BLADE_BAR_DEFINE_TYPE(TypeName, type_name, TYPE_PARENT) \
  static gpointer type_name##_parent_class = NULL; \
  static GType    type_name##_type = 0; \
  \
  static void     type_name##_init              (TypeName        *self); \
  static void     type_name##_class_init        (TypeName##Class *klass); \
  static void     type_name##_class_intern_init (gpointer klass) \
  { \
    type_name##_parent_class = g_type_class_peek_parent (klass); \
    type_name##_class_init ((TypeName##Class*) klass); \
  } \
  \
  GType \
  type_name##_get_type (void) \
  { \
    return type_name##_type; \
  } \
  \
  void \
  type_name##_register_type (BladeBarTypeModule *type_module) \
  { \
    GType plugin_define_type_id; \
    static const GTypeInfo plugin_define_type_info = \
    { \
      sizeof (TypeName##Class), \
      NULL, \
      NULL, \
      (GClassInitFunc) type_name##_class_intern_init, \
      NULL, \
      NULL, \
      sizeof (TypeName), \
      0, \
      (GInstanceInitFunc) type_name##_init, \
      NULL, \
    }; \
    \
    plugin_define_type_id = \
        g_type_module_register_type (G_TYPE_MODULE (type_module), TYPE_PARENT, \
                                     "Xfce" #TypeName, &plugin_define_type_info, 0); \
    \
    type_name##_type = plugin_define_type_id; \
  }



/* <private> */
#define _XPP_DEFINE_PLUGIN(TypeName, type_name, resident, args...) \
  GType blade_bar_module_init (GTypeModule *type_module, gboolean *make_resident); \
  \
  BLADE_BAR_DEFINE_TYPE (TypeName, type_name, XFCE_TYPE_BAR_PLUGIN) \
  \
  G_MODULE_EXPORT GType \
  blade_bar_module_init (GTypeModule *type_module, \
                          gboolean    *make_resident) \
  { \
    typedef void (*XppRegFunc) (BladeBarTypeModule *module); \
    XppRegFunc reg_funcs[] = { type_name##_register_type, args }; \
    guint      i; \
    \
    /* whether to make this plugin resident */ \
    if (make_resident != NULL) \
      *make_resident = resident; \
    \
    /* register the plugin types */ \
    for (i = 0; i < G_N_ELEMENTS (reg_funcs); i++) \
      (* reg_funcs[i]) (type_module); \
    \
    return type_name##_get_type (); \
  }



/**
 * BLADE_BAR_DEFINE_PREINIT_FUNC:
 * @preinit_func: name of the function that points to an
 *                #BladeBarPluginPreInit function.
 *
 * Registers a pre-init function in the plugin module. This function
 * is called before gtk_init() and can be used to initialize
 * special libaries.
 * Downside of this that the plugin cannot run internal. Even if you
 * set X-XFCE-Interal=TRUE in the desktop file, the bar will force
 * the plugin to run inside a wrapper (this because the bar called
 * gtk_init() long before it starts to load the plugins).
 *
 * Note that you can only use this once and it only works in
 * combination with the plugins register/define functions added
 * in 4.8.
 *
 * Since: 4.8
 **/
#define BLADE_BAR_DEFINE_PREINIT_FUNC(preinit_func) \
  G_MODULE_EXPORT gboolean blade_bar_module_preinit (gint argc, gchar **argv); \
  \
  G_MODULE_EXPORT gboolean \
  blade_bar_module_preinit (gint    argc, \
                             gchar **argv) \
  { \
    return (*preinit_func) (argc, argv); \
  }



/**
 * BLADE_BAR_PLUGIN_REGISTER:
 * @construct_func : name of the function that points to an
 *                   #BladeBarPluginFunc function.
 *
 * Register a bar plugin using a construct function. This is the
 * simplest way to register a bar plugin.
 * The @construct_func is called everytime a plugin is created.
 *
 * Since: 4.8
 **/
#define BLADE_BAR_PLUGIN_REGISTER(construct_func) \
  _BLADE_BAR_PLUGIN_REGISTER_EXTENDED (construct_func, /* foo */, /* foo */)



/**
 * BLADE_BAR_PLUGIN_REGISTER_WITH_CHECK:
 * @construct_func : name of the function that points to an
 *                   #BladeBarPluginFunc function.
 * @check_func     : name of the function that points to an
 *                   #BladeBarPluginCheck function.
 *
 * Register a bar plugin using a construct function. The @check_func
 * will be called before the plugin is created. If this function returns
 * %FALSE, the plugin won't be added to the bar. For proper feedback,
 * you are responsible for showing a dialog why the plugin is not added
 * to the bar.
 *
 * Since: 4.8
 **/
#define BLADE_BAR_PLUGIN_REGISTER_WITH_CHECK(construct_func, check_func) \
  _BLADE_BAR_PLUGIN_REGISTER_EXTENDED (construct_func, /* foo */, \
    if (G_LIKELY ((*check_func) (xpp_screen) == TRUE)))



/**
 * BLADE_BAR_PLUGIN_REGISTER_FULL:
 * @construct_func : name of the function that points to an
 *                   #BladeBarPluginFunc function.
 * @preinit_func   : name of the function that points to an
 *                   #BladeBarPluginPreInit function.
 * @check_func     : name of the function that points to an
 *                   #BladeBarPluginCheck function.
 *
 * Same as calling #BLADE_BAR_DEFINE_PREINIT_FUNC and
 * #BLADE_BAR_PLUGIN_REGISTER_WITH_CHECK. See those macros
 * for more information.
 *
 * Since: 4.8
 **/
#define BLADE_BAR_PLUGIN_REGISTER_FULL(construct_func, preinit_func, check_func) \
  BLADE_BAR_DEFINE_PREINIT_FUNC (preinit_func) \
  BLADE_BAR_PLUGIN_REGISTER_WITH_CHECK (construct_func, check_func)



/* <private> */
#define _BLADE_BAR_PLUGIN_REGISTER_EXTENDED(construct_func, PREINIT_CODE, CHECK_CODE) \
  static void \
  blade_bar_module_realize (BladeBarPlugin *xpp) \
  { \
    g_return_if_fail (BLADE_IS_BAR_PLUGIN (xpp)); \
    \
    g_signal_handlers_disconnect_by_func (G_OBJECT (xpp), \
        G_CALLBACK (blade_bar_module_realize), NULL); \
    \
    (*construct_func) (xpp); \
  } \
  \
  PREINIT_CODE \
  \
  G_MODULE_EXPORT BladeBarPlugin * \
  blade_bar_module_construct (const gchar  *xpp_name, \
                               gint          xpp_unique_id, \
                               const gchar  *xpp_display_name, \
                               const gchar  *xpp_comment, \
                               gchar       **xpp_arguments, \
                               GdkScreen    *xpp_screen); \
  G_MODULE_EXPORT BladeBarPlugin * \
  blade_bar_module_construct (const gchar  *xpp_name, \
                               gint          xpp_unique_id, \
                               const gchar  *xpp_display_name, \
                               const gchar  *xpp_comment, \
                               gchar       **xpp_arguments, \
                               GdkScreen    *xpp_screen) \
  { \
    BladeBarPlugin *xpp = NULL; \
    \
    g_return_val_if_fail (GDK_IS_SCREEN (xpp_screen), NULL); \
    g_return_val_if_fail (xpp_name != NULL && xpp_unique_id != -1, NULL); \
    \
    CHECK_CODE \
      { \
        xpp = g_object_new (XFCE_TYPE_BAR_PLUGIN, \
                            "name", xpp_name, \
                            "unique-id", xpp_unique_id, \
                            "display-name", xpp_display_name, \
                            "comment", xpp_comment, \
                            "arguments", xpp_arguments, NULL); \
        \
        g_signal_connect_after (G_OBJECT (xpp), "realize", \
            G_CALLBACK (blade_bar_module_realize), NULL); \
      } \
    \
    return xpp; \
  }

G_END_DECLS

#endif /* !__BLADE_BAR_MACROS_H__ */
