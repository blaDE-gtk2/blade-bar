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

#ifndef __BAR_PLUGIN_EXTERNAL_46_H__
#define __BAR_PLUGIN_EXTERNAL_46_H__

#include <gtk/gtk.h>
#include <libbladebar/libbladebar.h>
#include <libbladebar/blade-bar-plugin-provider.h>
#include <bar/bar-module.h>

G_BEGIN_DECLS

typedef struct _BarPluginExternal46Class BarPluginExternal46Class;
typedef struct _BarPluginExternal46      BarPluginExternal46;

#define BAR_TYPE_PLUGIN_EXTERNAL_46            (bar_plugin_external_46_get_type ())
#define BAR_PLUGIN_EXTERNAL_46(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BAR_TYPE_PLUGIN_EXTERNAL_46, BarPluginExternal46))
#define BAR_PLUGIN_EXTERNAL_46_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BAR_TYPE_PLUGIN_EXTERNAL_46, BarPluginExternal46Class))
#define BAR_IS_PLUGIN_EXTERNAL_46(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAR_TYPE_PLUGIN_EXTERNAL_46))
#define BAR_IS_PLUGIN_EXTERNAL_46_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BAR_TYPE_PLUGIN_EXTERNAL_46))
#define BAR_PLUGIN_EXTERNAL_46_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BAR_TYPE_PLUGIN_EXTERNAL_46, BarPluginExternal46Class))

GType      bar_plugin_external_46_get_type             (void) G_GNUC_CONST;

GtkWidget *bar_plugin_external_46_new                  (BarModule            *module,
                                                          gint                    unique_id,
                                                          gchar                 **arguments) G_GNUC_MALLOC;

void       bar_plugin_external_46_set_background_image (BarPluginExternal46  *external,
                                                          const gchar            *image);

G_END_DECLS

#endif /* !__BAR_PLUGIN_EXTERNAL_46_H__ */
