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

#if !defined(LIBBLADEBAR_INSIDE_LIBBLADEBAR_H) && !defined(LIBBLADEBAR_COMPILATION)
#error "Only <libbladebar/libbladebar.h> can be included directly, this file may disappear or change contents"
#endif

#ifndef __BLADE_BAR_IMAGE_H__
#define __BLADE_BAR_IMAGE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _BladeBarImageClass   BladeBarImageClass;
typedef struct _BladeBarImage        BladeBarImage;
typedef struct _BladeBarImagePrivate BladeBarImagePrivate;

struct _BladeBarImageClass
{
  /*< private >*/
  GtkWidgetClass __parent__;

  /*< private >*/
  void (*reserved1) (void);
  void (*reserved2) (void);
  void (*reserved3) (void);
  void (*reserved4) (void);
};

/**
 * BladeBarImage:
 *
 * This struct contain private data only and should be accessed by
 * the functions below.
 **/
struct _BladeBarImage
{
  /*< private >*/
  GtkWidget __parent__;

  /*< private >*/
  BladeBarImagePrivate *priv;
};

#define XFCE_TYPE_BAR_IMAGE            (blade_bar_image_get_type ())
#define BLADE_BAR_IMAGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XFCE_TYPE_BAR_IMAGE, BladeBarImage))
#define BLADE_BAR_IMAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), XFCE_TYPE_BAR_IMAGE, BladeBarImageClass))
#define BLADE_IS_BAR_IMAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XFCE_TYPE_BAR_IMAGE))
#define BLADE_IS_BAR_IMAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XFCE_TYPE_BAR_IMAGE))
#define BLADE_BAR_IMAGE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XFCE_TYPE_BAR_IMAGE, BladeBarImageClass))

GType      blade_bar_image_get_type        (void) G_GNUC_CONST;

GtkWidget *blade_bar_image_new             (void) G_GNUC_MALLOC;

GtkWidget *blade_bar_image_new_from_pixbuf (GdkPixbuf      *pixbuf) G_GNUC_MALLOC;

GtkWidget *blade_bar_image_new_from_source (const gchar    *source) G_GNUC_MALLOC;

void       blade_bar_image_set_from_pixbuf (BladeBarImage *image,
                                             GdkPixbuf      *pixbuf);

void       blade_bar_image_set_from_source (BladeBarImage *image,
                                             const gchar    *source);

void       blade_bar_image_set_size        (BladeBarImage *image,
                                             gint            size);

gint       blade_bar_image_get_size        (BladeBarImage *image);

void       blade_bar_image_clear           (BladeBarImage *image);

G_END_DECLS

#endif /* !__BLADE_BAR_IMAGE_H__ */
