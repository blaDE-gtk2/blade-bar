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

#ifndef __BAR_DBUS_SERVICE_H__
#define __BAR_DBUS_SERVICE_H__

#include <glib.h>
#include <common/bar-dbus.h>

typedef struct _BarDBusServiceClass BarDBusServiceClass;
typedef struct _BarDBusService      BarDBusService;

#define BAR_TYPE_DBUS_SERVICE            (bar_dbus_service_get_type ())
#define BAR_DBUS_SERVICE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BAR_TYPE_DBUS_SERVICE, BarDBusService))
#define BAR_DBUS_CLASS_SERVICE(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BAR_TYPE_DBUS_SERVICE, BarDBusServiceClass))
#define BAR_IS_DBUS_SERVICE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAR_TYPE_DBUS_SERVICE))
#define BAR_IS_DBUS_SERVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BAR_TYPE_DBUS_SERVICE))
#define BAR_DBUS_SERVICE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BAR_TYPE_DBUS_SERVICE, BarDBusServiceClass))



GType               bar_dbus_service_get_type    (void) G_GNUC_CONST;

BarDBusService   *bar_dbus_service_get         (void);

gboolean            bar_dbus_service_is_owner    (BarDBusService *service);

void                bar_dbus_service_exit_bar  (gboolean          restart);

gboolean            bar_dbus_service_get_restart (void);

G_END_DECLS

#endif /* !__BAR_DBUS_SERVICE_H__ */
