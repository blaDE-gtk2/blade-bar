/*
 * Copyright (C) 2009-2010 Nick Schermer <nick@xfce.org>
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

#ifndef __BAR_BLCONF_H__
#define __BAR_BLCONF_H__

#include <blconf/blconf.h>

#define BAR_PROPERTIES_TYPE_VALUE_ARRAY (bar_properties_value_array_get_type ())



typedef struct _BarProperty BarProperty;
struct _BarProperty
{
  const gchar *property;
  GType        type;
};



BlconfChannel *bar_properties_get_channel          (GObject             *object_for_weak_ref);

void           bar_properties_bind                 (BlconfChannel       *channel,
                                                      GObject             *object,
                                                      const gchar         *property_base,
                                                      const BarProperty *properties,
                                                      gboolean             save_properties);

void           bar_properties_unbind               (GObject             *object);

GType          bar_properties_value_array_get_type (void) G_GNUC_CONST;

#endif /* !__BAR_BLCONF_H__ */
