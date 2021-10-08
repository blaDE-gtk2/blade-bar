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

#ifndef __BAR_PLUGIN_EXTERNAL_H__
#define __BAR_PLUGIN_EXTERNAL_H__

#include <gtk/gtk.h>
#include <libbladebar/libbladebar.h>
#include <libbladebar/blade-bar-plugin-provider.h>
#include <bar/bar-module.h>

G_BEGIN_DECLS

typedef struct _BarPluginExternalClass   BarPluginExternalClass;
typedef struct _BarPluginExternal        BarPluginExternal;
typedef struct _BarPluginExternalPrivate BarPluginExternalPrivate;

#define BAR_TYPE_PLUGIN_EXTERNAL            (bar_plugin_external_get_type ())
#define BAR_PLUGIN_EXTERNAL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BAR_TYPE_PLUGIN_EXTERNAL, BarPluginExternal))
#define BAR_PLUGIN_EXTERNAL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BAR_TYPE_PLUGIN_EXTERNAL, BarPluginExternalClass))
#define BAR_IS_PLUGIN_EXTERNAL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAR_TYPE_PLUGIN_EXTERNAL))
#define BAR_IS_PLUGIN_EXTERNAL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BAR_TYPE_PLUGIN_EXTERNAL))
#define BAR_PLUGIN_EXTERNAL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BAR_TYPE_PLUGIN_EXTERNAL, BarPluginExternalClass))

struct _BarPluginExternalClass
{
  GtkSocketClass __parent__;

  /* send bar values to the plugin or wrapper */
  void       (*set_properties) (BarPluginExternal *external,
                                GSList              *properties);

  /* complete startup array for the plugin */
  gchar    **(*get_argv)       (BarPluginExternal  *external,
                                gchar               **arguments);

  /* handling of remote events */
  gboolean   (*remote_event)   (BarPluginExternal  *external,
                                const gchar          *name,
                                const GValue         *value,
                                guint                *handle);
};

struct _BarPluginExternal
{
  GtkSocket __parent__;

  BarPluginExternalPrivate *priv;

  BarModule                *module;

  gint                        unique_id;

  /* some info received on plugin startup by the
   * implementations of the abstract object */
  guint                       show_configure : 1;
  guint                       show_about : 1;
};

typedef struct
{
  BladeBarPluginProviderPropType type;
  GValue                          value;
}
PluginProperty;



GType        bar_plugin_external_get_type             (void) G_GNUC_CONST;

void         bar_plugin_external_restart              (BarPluginExternal  *external);

void         bar_plugin_external_set_background_alpha (BarPluginExternal  *external,
                                                         gdouble               alpha);

void         bar_plugin_external_set_background_color (BarPluginExternal  *external,
                                                         const GdkColor       *color);

void         bar_plugin_external_set_background_image (BarPluginExternal  *external,
                                                         const gchar          *image);

GPid         bar_plugin_external_get_pid              (BarPluginExternal  *external);

G_END_DECLS

#endif /* !__BAR_PLUGIN_EXTERNAL_H__ */
