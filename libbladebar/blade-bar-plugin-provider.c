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

#include <gtk/gtk.h>

#include <common/bar-private.h>

#include <libbladebar/blade-bar-macros.h>
#include <libbladebar/blade-bar-plugin-provider.h>
#include <libbladebar/libbladebar-alias.h>


enum
{
  PROVIDER_SIGNAL,
  LAST_SIGNAL
};



static void blade_bar_plugin_provider_default_init (BladeBarPluginProviderInterface *klass);



static guint provider_signals[LAST_SIGNAL];



GType
blade_bar_plugin_provider_get_type (void)
{
  static volatile gsize type__volatile = 0;
  GType                 type;

  if (g_once_init_enter (&type__volatile))
    {
      type = g_type_register_static_simple (G_TYPE_INTERFACE,
                                            g_intern_static_string ("BladeBarPluginProvider"),
                                            sizeof (BladeBarPluginProviderInterface),
                                            (GClassInitFunc) blade_bar_plugin_provider_default_init,
                                            0,
                                            NULL,
                                            0);

      g_once_init_leave (&type__volatile, type);
    }

  return type__volatile;
}



static void
blade_bar_plugin_provider_default_init (BladeBarPluginProviderInterface *klass)
{
  provider_signals[PROVIDER_SIGNAL] =
    g_signal_new (g_intern_static_string ("provider-signal"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  g_cclosure_marshal_VOID__UINT,
                  G_TYPE_NONE, 1, G_TYPE_UINT);
}



const gchar *
blade_bar_plugin_provider_get_name (BladeBarPluginProvider *provider)
{
  bar_return_val_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider), NULL);

  return (*BLADE_BAR_PLUGIN_PROVIDER_GET_INTERFACE (provider)->get_name) (provider);
}



gint
blade_bar_plugin_provider_get_unique_id (BladeBarPluginProvider *provider)
{
  bar_return_val_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider), -1);

  return (*BLADE_BAR_PLUGIN_PROVIDER_GET_INTERFACE (provider)->get_unique_id) (provider);
}



void
blade_bar_plugin_provider_set_size (BladeBarPluginProvider *provider,
                                     gint                     size)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  (*BLADE_BAR_PLUGIN_PROVIDER_GET_INTERFACE (provider)->set_size) (provider, size);
}



void
blade_bar_plugin_provider_set_mode (BladeBarPluginProvider *provider,
                                     BladeBarPluginMode      mode)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  (*BLADE_BAR_PLUGIN_PROVIDER_GET_INTERFACE (provider)->set_mode) (provider, mode);
}



void
blade_bar_plugin_provider_set_nrows (BladeBarPluginProvider *provider,
                                      guint                    rows)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  (*BLADE_BAR_PLUGIN_PROVIDER_GET_INTERFACE (provider)->set_nrows) (provider, rows);
}



void
blade_bar_plugin_provider_set_screen_position (BladeBarPluginProvider *provider,
                                                XfceScreenPosition       screen_position)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  (*BLADE_BAR_PLUGIN_PROVIDER_GET_INTERFACE (provider)->set_screen_position) (provider, screen_position);
}



void
blade_bar_plugin_provider_save (BladeBarPluginProvider *provider)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  (*BLADE_BAR_PLUGIN_PROVIDER_GET_INTERFACE (provider)->save) (provider);
}



void
blade_bar_plugin_provider_emit_signal (BladeBarPluginProvider       *provider,
                                        BladeBarPluginProviderSignal  provider_signal)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  /* emit the signal */
  g_signal_emit (G_OBJECT (provider), provider_signals[PROVIDER_SIGNAL], 0, provider_signal);
}



gboolean
blade_bar_plugin_provider_get_show_configure (BladeBarPluginProvider *provider)
{
  bar_return_val_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider), FALSE);

  return (*BLADE_BAR_PLUGIN_PROVIDER_GET_INTERFACE (provider)->get_show_configure) (provider);
}



void
blade_bar_plugin_provider_show_configure (BladeBarPluginProvider *provider)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  (*BLADE_BAR_PLUGIN_PROVIDER_GET_INTERFACE (provider)->show_configure) (provider);
}



gboolean
blade_bar_plugin_provider_get_show_about (BladeBarPluginProvider *provider)
{
  bar_return_val_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider), FALSE);

  return (*BLADE_BAR_PLUGIN_PROVIDER_GET_INTERFACE (provider)->get_show_about) (provider);
}



void
blade_bar_plugin_provider_show_about (BladeBarPluginProvider *provider)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  (*BLADE_BAR_PLUGIN_PROVIDER_GET_INTERFACE (provider)->show_about) (provider);
}



void
blade_bar_plugin_provider_removed (BladeBarPluginProvider *provider)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  (*BLADE_BAR_PLUGIN_PROVIDER_GET_INTERFACE (provider)->removed) (provider);
}



gboolean
blade_bar_plugin_provider_remote_event (BladeBarPluginProvider *provider,
                                         const gchar             *name,
                                         const GValue            *value,
                                         guint                   *handle)
{
  const GValue *real_value = value;

  bar_return_val_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider), TRUE);
  bar_return_val_if_fail (name != NULL, TRUE);
  bar_return_val_if_fail (value == NULL || G_IS_VALUE (value), TRUE);

  if (BLADE_BAR_PLUGIN_PROVIDER_GET_INTERFACE (provider)->remote_event != NULL)
    {
      if (real_value != NULL
          && G_VALUE_HOLDS_UCHAR (real_value)
          && g_value_get_uchar (real_value) == '\0')
        real_value = NULL;

      return (*BLADE_BAR_PLUGIN_PROVIDER_GET_INTERFACE (provider)->remote_event) (provider, name, real_value, handle);
    }

  return FALSE;
}



void
blade_bar_plugin_provider_set_locked (BladeBarPluginProvider *provider,
                                       gboolean                 locked)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  (*BLADE_BAR_PLUGIN_PROVIDER_GET_INTERFACE (provider)->set_locked) (provider, locked);
}



void
blade_bar_plugin_provider_ask_remove (BladeBarPluginProvider *provider)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));

  (*BLADE_BAR_PLUGIN_PROVIDER_GET_INTERFACE (provider)->ask_remove) (provider);
}


#define __BLADE_BAR_PLUGIN_PROVIDER_C__
#include <libbladebar/libbladebar-aliasdef.c>
