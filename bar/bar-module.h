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

#ifndef __BAR_MODULE_H__
#define __BAR_MODULE_H__

#include <gtk/gtk.h>
#include <libbladebar/libbladebar.h>
#include <libbladebar/blade-bar-plugin-provider.h>

G_BEGIN_DECLS

typedef struct _BarModuleClass  BarModuleClass;
typedef struct _BarModule       BarModule;

#define BAR_TYPE_MODULE            (bar_module_get_type ())
#define BAR_MODULE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BAR_TYPE_MODULE, BarModule))
#define BAR_MODULE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BAR_TYPE_MODULE, BarModuleClass))
#define BAR_IS_MODULE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAR_TYPE_MODULE))
#define BAR_IS_MODULE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BAR_TYPE_MODULE))
#define BAR_MODULE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BAR_TYPE_MODULE, BarModuleClass))



GType        bar_module_get_type                 (void) G_GNUC_CONST;

BarModule *bar_module_new_from_desktop_file    (const gchar             *filename,
                                                    const gchar             *name,
                                                    gboolean                 force_external) G_GNUC_MALLOC;

GtkWidget   *bar_module_new_plugin               (BarModule             *module,
                                                    GdkScreen               *screen,
                                                    gint                     unique_id,
                                                    gchar                  **arguments) G_GNUC_MALLOC;

const gchar *bar_module_get_name                 (BarModule             *module) G_GNUC_PURE;

const gchar *bar_module_get_filename             (BarModule             *module) G_GNUC_PURE;

const gchar *bar_module_get_display_name         (BarModule             *module) G_GNUC_PURE;

const gchar *bar_module_get_comment              (BarModule             *module) G_GNUC_PURE;

const gchar *bar_module_get_icon_name            (BarModule             *module) G_GNUC_PURE;

const gchar *bar_module_get_api                  (BarModule             *module) G_GNUC_PURE;

BarModule *bar_module_get_from_plugin_provider (BladeBarPluginProvider *provider);

gboolean     bar_module_is_valid                 (BarModule             *module);

gboolean     bar_module_is_unique                (BarModule             *module) G_GNUC_PURE;

gboolean     bar_module_is_usable                (BarModule             *module,
                                                    GdkScreen               *screen);

G_END_DECLS

#endif /* !__BAR_MODULE_H__ */
