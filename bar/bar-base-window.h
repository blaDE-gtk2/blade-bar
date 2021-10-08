/*
 * Copyright (C) 2009-2010 Nick Schermer <nick@xfce.org>
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

#ifndef __BAR_BASE_WINDOW_H__
#define __BAR_BASE_WINDOW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _BarBaseWindowClass   BarBaseWindowClass;
typedef struct _BarBaseWindow        BarBaseWindow;
typedef struct _BarBaseWindowPrivate BarBaseWindowPrivate;
typedef enum   _BarBorders           BarBorders;
typedef enum   _BarBgStyle           BarBgStyle;

#define BAR_TYPE_BASE_WINDOW            (bar_base_window_get_type ())
#define BAR_BASE_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BAR_TYPE_BASE_WINDOW, BarBaseWindow))
#define BAR_BASE_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BAR_TYPE_BASE_WINDOW, BarBaseWindowClass))
#define BAR_IS_BASE_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAR_TYPE_BASE_WINDOW))
#define BAR_IS_BASE_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BAR_TYPE_BASE_WINDOW))
#define BAR_BASE_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BAR_TYPE_BASE_WINDOW, BarBaseWindowClass))

enum _BarBorders
{
  BAR_BORDER_NONE   = 0,
  BAR_BORDER_LEFT   = 1 << 0,
  BAR_BORDER_RIGHT  = 1 << 1,
  BAR_BORDER_TOP    = 1 << 2,
  BAR_BORDER_BOTTOM = 1 << 3
};

enum _BarBgStyle
{
  BAR_BG_STYLE_NONE,
  BAR_BG_STYLE_COLOR,
  BAR_BG_STYLE_IMAGE
};

struct _BarBaseWindowClass
{
  GtkWindowClass __parent__;
};

struct _BarBaseWindow
{
  GtkWindow __parent__;

  /*< private >*/
  BarBaseWindowPrivate *priv;

  guint                   is_composited : 1;

  gdouble                 background_alpha;
  BarBgStyle            background_style;
  GdkColor               *background_color;
  gchar                  *background_image;
};

GType        bar_base_window_get_type    (void) G_GNUC_CONST;

void         bar_base_window_move_resize (BarBaseWindow *window,
                                            gint             x,
                                            gint             y,
                                            gint             width,
                                            gint             height);

void         bar_base_window_set_borders (BarBaseWindow *window,
                                            BarBorders     borders);
BarBorders bar_base_window_get_borders (BarBaseWindow *window);

void         bar_util_set_source_rgba    (cairo_t         *cr,
                                            const GdkColor  *color,
                                            gdouble          alpha);

G_END_DECLS

#endif /* !__BAR_BASE_WINDOW_H__ */
