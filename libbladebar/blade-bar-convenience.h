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

/* #if !defined(LIBBLADEBAR_INSIDE_LIBBLADEBAR_H) && !defined(LIBBLADEBAR_COMPILATION)
#error "Only <libbladebar/libbladebar.h> can be included directly, this file may disappear or change contents"
#endif */

#ifndef __BLADE_BAR_CONVENIENCE_H__
#define __BLADE_BAR_CONVENIENCE_H__

#include <gtk/gtk.h>
#include <libbladebar/blade-bar-macros-46.h>

G_BEGIN_DECLS

GtkWidget   *blade_bar_create_button              (void) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

GtkWidget   *blade_bar_create_toggle_button       (void) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

const gchar *blade_bar_get_channel_name           (void);

GdkPixbuf   *blade_bar_pixbuf_from_source_at_size (const gchar  *source,
                                                    GtkIconTheme *icon_theme,
                                                    gint          dest_width,
                                                    gint          dest_height) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

GdkPixbuf   *blade_bar_pixbuf_from_source         (const gchar  *source,
                                                    GtkIconTheme *icon_theme,
                                                    gint          size) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS

#endif /* !__BLADE_BAR_CONVENIENCE_H__ */
