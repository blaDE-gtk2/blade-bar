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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <dbus/dbus-glib.h>

#include <common/bar-private.h>
#include <common/bar-blconf.h>
#include <libbladebar/blade-bar-macros.h>



static void
bar_properties_store_value (BlconfChannel *channel,
                              const gchar   *blconf_property,
                              GType          blconf_property_type,
                              GObject       *object,
                              const gchar   *object_property)
{
  GValue      value = { 0, };
  GdkColor   *color;
  guint16     alpha = 0xffff;
#ifndef NDEBUG
  GParamSpec *pspec;
#endif

  bar_return_if_fail (G_IS_OBJECT (object));
  bar_return_if_fail (BLCONF_IS_CHANNEL (channel));

#ifndef NDEBUG
  /* check if the types match */
  pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (object), object_property);
  bar_assert (pspec != NULL);
  if (G_PARAM_SPEC_VALUE_TYPE (pspec) != blconf_property_type)
    {
      g_critical ("Object and Blconf properties don't match! %s::%s. %s != %s",
                  G_OBJECT_TYPE_NAME (object), blconf_property,
                  g_type_name (blconf_property_type),
                  g_type_name (G_PARAM_SPEC_VALUE_TYPE (pspec)));
    }
#endif

  /* write the property to the blconf channel */
  g_value_init (&value, blconf_property_type);
  g_object_get_property (G_OBJECT (object), object_property, &value);

  if (G_LIKELY (blconf_property_type != GDK_TYPE_COLOR))
    {
      blconf_channel_set_property (channel, blconf_property, &value);
    }
  else
    {
      /* work around blconf's lack of storing colors (bug #7117) and
       * do the same as blconf_g_property_bind_gdkcolor() does */
      color = g_value_get_boxed (&value);
      blconf_channel_set_array (channel, blconf_property,
                                BLCONF_TYPE_UINT16, &color->red,
                                BLCONF_TYPE_UINT16, &color->green,
                                BLCONF_TYPE_UINT16, &color->blue,
                                BLCONF_TYPE_UINT16, &alpha,
                                G_TYPE_INVALID);
    }

  g_value_unset (&value);
}



BlconfChannel *
bar_properties_get_channel (GObject *object_for_weak_ref)
{
  GError        *error = NULL;
  BlconfChannel *channel;

  bar_return_val_if_fail (G_IS_OBJECT (object_for_weak_ref), NULL);

  if (!blconf_init (&error))
    {
      g_critical ("Failed to initialize Blconf: %s", error->message);
      g_error_free (error);
      return NULL;
    }

  channel = blconf_channel_get (BLADE_BAR_CHANNEL_NAME);
  g_object_weak_ref (object_for_weak_ref, (GWeakNotify) blconf_shutdown, NULL);

  return channel;
}



void
bar_properties_bind (BlconfChannel       *channel,
                       GObject             *object,
                       const gchar         *property_base,
                       const BarProperty *properties,
                       gboolean             save_properties)
{
  const BarProperty *prop;
  gchar               *property;

  bar_return_if_fail (channel == NULL || BLCONF_IS_CHANNEL (channel));
  bar_return_if_fail (G_IS_OBJECT (object));
  bar_return_if_fail (property_base != NULL && *property_base == '/');
  bar_return_if_fail (properties != NULL);

  if (G_LIKELY (channel == NULL))
    channel = bar_properties_get_channel (object);
  bar_return_if_fail (BLCONF_IS_CHANNEL (channel));

  /* walk the properties array */
  for (prop = properties; prop->property != NULL; prop++)
    {
      property = g_strconcat (property_base, "/", prop->property, NULL);

      if (save_properties)
        bar_properties_store_value (channel, property, prop->type, object, prop->property);

      if (G_LIKELY (prop->type != GDK_TYPE_COLOR))
        blconf_g_property_bind (channel, property, prop->type, object, prop->property);
      else
        blconf_g_property_bind_gdkcolor (channel, property, object, prop->property);

      g_free (property);
    }
}



void
bar_properties_unbind (GObject *object)
{
  blconf_g_property_unbind_all (object);
}



GType
bar_properties_value_array_get_type (void)
{
  static volatile gsize type__volatile = 0;
  GType                 type;

  if (g_once_init_enter (&type__volatile))
    {
      type = dbus_g_type_get_collection ("GPtrArray", G_TYPE_VALUE);
      g_once_init_leave (&type__volatile, type);
    }

  return type__volatile;
}
