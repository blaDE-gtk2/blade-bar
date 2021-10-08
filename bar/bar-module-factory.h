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

#ifndef __BAR_MODULE_FACTORY_H__
#define __BAR_MODULE_FACTORY_H__

#include <gtk/gtk.h>
#include <bar/bar-module.h>
#include <libbladebar/libbladebar.h>
#include <libbladebar/blade-bar-plugin-provider.h>

G_BEGIN_DECLS

typedef struct _BarModuleFactoryClass BarModuleFactoryClass;
typedef struct _BarModuleFactory      BarModuleFactory;

#define BAR_TYPE_MODULE_FACTORY            (bar_module_factory_get_type ())
#define BAR_MODULE_FACTORY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BAR_TYPE_MODULE_FACTORY, BarModuleFactory))
#define BAR_MODULE_FACTORY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BAR_TYPE_MODULE_FACTORY, BarModuleFactoryClass))
#define BAR_IS_MODULE_FACTORY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAR_TYPE_MODULE_FACTORY))
#define BAR_IS_MODULE_FACTORY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BAR_TYPE_MODULE_FACTORY))
#define BAR_MODULE_FACTORY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BAR_TYPE_MODULE_FACTORY, BarModuleFactoryClass))

#define LAUNCHER_PLUGIN_NAME "launcher"

GType               bar_module_factory_get_type            (void) G_GNUC_CONST;

BarModuleFactory *bar_module_factory_get                 (void);

void                bar_module_factory_force_all_external  (void);

gboolean            bar_module_factory_has_launcher        (BarModuleFactory  *factory);

void                bar_module_factory_emit_unique_changed (BarModule         *module);

GList              *bar_module_factory_get_modules         (BarModuleFactory  *factory);

gboolean            bar_module_factory_has_module          (BarModuleFactory  *factory,
                                                              const gchar         *name);

GSList             *bar_module_factory_get_plugins         (BarModuleFactory  *factory,
                                                              const gchar         *plugin_name);

GtkWidget          *bar_module_factory_new_plugin          (BarModuleFactory  *factory,
                                                              const gchar         *name,
                                                              GdkScreen           *screen,
                                                              gint                 unique_id,
                                                              gchar              **arguments,
                                                              gint                *return_unique_id) G_GNUC_MALLOC;

G_END_DECLS

#endif /* !__BAR_MODULE_FACTORY_H__ */
