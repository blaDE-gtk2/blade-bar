/*
 * Copyright (C) 2006-2007 Jasper Huijsmans <jasper@xfce.org>
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

#ifdef HAVE_MATH_H
#include <math.h>
#endif

#include <libbladeutil/libbladeutil.h>
#include <gtk/gtk.h>

#include <libbladebar/blade-bar-macros.h>
#include <libbladebar/blade-bar-convenience.h>
#include <libbladebar/libbladebar-alias.h>



/**
 * SECTION: convenience
 * @title: Convenience Functions
 * @short_description: Special purpose widgets and utilities
 * @include: libbladebar/libbladebar.h
 *
 * This section describes a number of functions that were created
 * to help developers of Blade Bar plugins.
 **/

/**
 * blade_bar_create_button:
 *
 * Create regular #GtkButton with a few properties set to be useful in the
 * Xfce bar: Flat (%GTK_RELIEF_NONE), no focus on click and minimal padding.
 *
 * Returns: newly created #GtkButton.
 **/
GtkWidget *
blade_bar_create_button (void)
{
  GtkWidget *button = gtk_button_new ();

  gtk_widget_set_can_default (GTK_WIDGET (button), FALSE);
  gtk_widget_set_can_focus (GTK_WIDGET (button), FALSE);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
  gtk_widget_set_name (button, "blade-bar-button");

  return button;
}



/**
 * blade_bar_create_toggle_button:
 *
 * Create regular #GtkToggleButton with a few properties set to be useful in
 * Xfce bar: Flat (%GTK_RELIEF_NONE), no focus on click and minimal padding.
 *
 * Returns: newly created #GtkToggleButton.
 **/
GtkWidget *
blade_bar_create_toggle_button (void)
{
  GtkWidget *button = gtk_toggle_button_new ();

  gtk_widget_set_can_default (GTK_WIDGET (button), FALSE);
  gtk_widget_set_can_focus (GTK_WIDGET (button), FALSE);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
  gtk_widget_set_name (button, "blade-bar-toggle-button");

  return button;
}



/**
 * blade_bar_get_channel_name:
 *
 * Function for the name of the Blconf channel used by the bar. By default
 * this returns "blade-bar", but you can override this value with the
 * environment variable BLADE_BAR_CHANNEL_NAME.
 *
 * Returns: name of the Blconf channel
 *
 * See also: BLADE_BAR_CHANNEL_NAME,
 *           blade_bar_plugin_blconf_channel_new and
 *           blade_bar_plugin_get_property_base
 *
 * Since: 4.8
 **/
const gchar *
blade_bar_get_channel_name (void)
{
  static const gchar *name = NULL;

  if (G_UNLIKELY (name == NULL))
    {
      name = g_getenv ("BLADE_BAR_CHANNEL_NAME");
      if (G_LIKELY (name == NULL))
        name = "blade-bar";
    }

  return name;
}



/**
 * blade_bar_pixbuf_from_source_at_size:
 * @source: string that contains the location of an icon
 * @icon_theme: icon theme or %NULL to use the default icon theme
 * @dest_width: the maximum returned width of the GdkPixbuf
 * @dest_height: the maximum returned height of the GdkPixbuf
 *
 * Try to load a pixbuf from a source string. The source could be
 * an abolute path, an icon name or a filename that points to a
 * file in the pixmaps directory.
 *
 * This function is particularly usefull for loading names from
 * the Icon key of desktop files.
 *
 * The pixbuf is never bigger than @dest_width and @dest_height.
 * If it is when loaded from the disk, the pixbuf is scaled
 * preserving the aspect ratio.
 *
 * Returns: a GdkPixbuf or %NULL if nothing was found. The value should
 *          be released with g_object_unref when no longer used.
 *
 * See also: BladeBarImage
 *
 * Since: 4.10
 **/
GdkPixbuf *
blade_bar_pixbuf_from_source_at_size (const gchar  *source,
                                       GtkIconTheme *icon_theme,
                                       gint          dest_width,
                                       gint          dest_height)
{
  GdkPixbuf *pixbuf = NULL;
  gchar     *p;
  gchar     *name;
  gchar     *filename;
  gint       src_w, src_h;
  gdouble    ratio;
  GdkPixbuf *dest;
  GError    *error = NULL;
  gint       size = MIN (dest_width, dest_height);

  g_return_val_if_fail (source != NULL, NULL);
  g_return_val_if_fail (icon_theme == NULL || GTK_IS_ICON_THEME (icon_theme), NULL);
  g_return_val_if_fail (dest_width > 0, NULL);
  g_return_val_if_fail (dest_height > 0, NULL);

  if (G_UNLIKELY (g_path_is_absolute (source)))
    {
      pixbuf = gdk_pixbuf_new_from_file (source, &error);
      if (G_UNLIKELY (pixbuf == NULL))
        {
          g_message ("Failed to load image \"%s\": %s",
                     source, error->message);
          g_error_free (error);
        }
    }
  else
    {
      if (G_UNLIKELY (icon_theme == NULL))
        icon_theme = gtk_icon_theme_get_default ();

      /* try to load from the icon theme */
      pixbuf = gtk_icon_theme_load_icon (icon_theme, source, size, 0, NULL);
      if (G_UNLIKELY (pixbuf == NULL))
        {
          /* try to lookup names like application.png in the theme */
          p = strrchr (source, '.');
          if (p != NULL)
            {
              name = g_strndup (source, p - source);
              pixbuf = gtk_icon_theme_load_icon (icon_theme, name, size, 0, NULL);
              g_free (name);
            }

          /* maybe they point to a file in the pixbufs folder */
          if (G_UNLIKELY (pixbuf == NULL))
            {
              filename = g_build_filename ("pixmaps", source, NULL);
              name = xfce_resource_lookup (XFCE_RESOURCE_DATA, filename);
              g_free (filename);

              if (name != NULL)
                {
                  pixbuf = gdk_pixbuf_new_from_file (name, NULL);
                  g_free (name);
                }
            }
        }
    }

  if (G_UNLIKELY (pixbuf == NULL))
    {
      if (G_UNLIKELY (icon_theme == NULL))
        icon_theme = gtk_icon_theme_get_default ();

      /* bit ugly as a fallback, but in most cases better then no icon */
      pixbuf = gtk_icon_theme_load_icon (icon_theme, GTK_STOCK_MISSING_IMAGE,
                                         size, GTK_ICON_LOOKUP_USE_BUILTIN, NULL);
    }

  /* scale the pixbug if required */
  if (G_LIKELY (pixbuf != NULL))
    {
      src_w = gdk_pixbuf_get_width (pixbuf);
      src_h = gdk_pixbuf_get_height (pixbuf);

      if (src_w > dest_width || src_h > dest_height)
        {
          /* calculate the new dimensions */
          ratio = MIN ((gdouble) dest_width / (gdouble) src_w,
                       (gdouble) dest_height / (gdouble) src_h);

          dest_width  = rint (src_w * ratio);
          dest_height = rint (src_h * ratio);

          dest = gdk_pixbuf_scale_simple (pixbuf,
                                          MAX (dest_width, 1),
                                          MAX (dest_height, 1),
                                          GDK_INTERP_BILINEAR);

          g_object_unref (G_OBJECT (pixbuf));
          pixbuf = dest;
        }
    }

  return pixbuf;
}



/**
 * blade_bar_pixbuf_from_source:
 * @source: string that contains the location of an icon
 * @icon_theme: icon theme or %NULL to use the default icon theme
 * @size: size the icon that should be loaded
 *
 * See blade_bar_pixbuf_from_source_at_size
 *
 * Returns: a GdkPixbuf or %NULL if nothing was found. The value should
 *          be released with g_object_unref when no longer used.
 *
 * See also: BladeBarImage
 *
 * Since: 4.8
 **/
GdkPixbuf *
blade_bar_pixbuf_from_source (const gchar  *source,
                               GtkIconTheme *icon_theme,
                               gint          size)
{
  return blade_bar_pixbuf_from_source_at_size (source, icon_theme, size, size);
}



#define __BLADE_BAR_CONVENIENCE_C__
#include <libbladebar/libbladebar-aliasdef.c>
