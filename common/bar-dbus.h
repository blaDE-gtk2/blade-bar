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

#ifndef __BAR_DBUS_H__
#define __BAR_DBUS_H__

/* bar dbus names */
#define BAR_DBUS_NAME              "org.blade.Bar"
#define BAR_DBUS_PATH              "/org/blade/Bar"
#define BAR_DBUS_INTERFACE         BAR_DBUS_NAME
#define BAR_DBUS_WRAPPER_PATH      BAR_DBUS_PATH "/Wrapper/%d"
#define BAR_DBUS_WRAPPER_INTERFACE BAR_DBUS_INTERFACE ".Wrapper"

/* special types for dbus communication */
#define BAR_TYPE_DBUS_SET_PROPERTY \
  dbus_g_type_get_struct ("GValueArray", \
                          G_TYPE_UINT, \
                          G_TYPE_VALUE, \
                          G_TYPE_INVALID)

#define BAR_TYPE_DBUS_SET_SIGNAL \
  dbus_g_type_get_collection ("GPtrArray", \
                              BAR_TYPE_DBUS_SET_PROPERTY)

enum
{
  DBUS_SET_TYPE,
  DBUS_SET_VALUE
};

#endif /* !__BAR_DBUS_H__ */
