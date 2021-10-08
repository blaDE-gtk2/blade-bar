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

#ifndef __BAR_WINDOW_H__
#define __BAR_WINDOW_H__

#include <gtk/gtk.h>
#include <blconf/blconf.h>

G_BEGIN_DECLS

typedef struct _BarWindowClass BarWindowClass;
typedef struct _BarWindow      BarWindow;

#define BAR_TYPE_WINDOW            (bar_window_get_type ())
#define BAR_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BAR_TYPE_WINDOW, BarWindow))
#define BAR_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BAR_TYPE_WINDOW, BarWindowClass))
#define BAR_IS_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAR_TYPE_WINDOW))
#define BAR_IS_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BAR_TYPE_WINDOW))
#define BAR_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BAR_TYPE_WINDOW, BarWindowClass))

GType      bar_window_get_type                  (void) G_GNUC_CONST;

GtkWidget *bar_window_new                       (GdkScreen     *screen,
                                                   gint           id) G_GNUC_MALLOC;

gint       bar_window_get_id                    (BarWindow   *window);

gboolean   bar_window_has_position              (BarWindow   *window);

void       bar_window_set_povider_info          (BarWindow   *window,
                                                   GtkWidget     *provider,
                                                   gboolean       moving_to_other_bar);

void       bar_window_freeze_autohide           (BarWindow   *window);

void       bar_window_thaw_autohide             (BarWindow   *window);

void       bar_window_set_locked                (BarWindow   *window,
                                                   gboolean       locked);

gboolean   bar_window_get_locked                (BarWindow   *window);

void       bar_window_focus                     (BarWindow   *window);

void       bar_window_migrate_autohide_property (BarWindow   *window,
                                                   BlconfChannel *blconf,
                                                   const gchar   *property_base);

G_END_DECLS

#endif /* !__BAR_WINDOW_H__ */
