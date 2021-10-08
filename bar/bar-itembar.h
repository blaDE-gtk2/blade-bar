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

#ifndef __BAR_ITEMBAR_H__
#define __BAR_ITEMBAR_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _BarItembarClass BarItembarClass;
typedef struct _BarItembar      BarItembar;

#define BAR_TYPE_ITEMBAR            (bar_itembar_get_type ())
#define BAR_ITEMBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BAR_TYPE_ITEMBAR, BarItembar))
#define BAR_ITEMBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BAR_TYPE_ITEMBAR, BarItembarClass))
#define BAR_IS_ITEMBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAR_TYPE_ITEMBAR))
#define BAR_IS_ITEMBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BAR_TYPE_ITEMBAR))
#define BAR_ITEMBAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BAR_TYPE_ITEMBAR, BarItembarClass))

GType           bar_itembar_get_type                (void) G_GNUC_CONST;

GtkWidget      *bar_itembar_new                     (void) G_GNUC_MALLOC;

void            bar_itembar_insert                  (BarItembar *itembar,
                                                       GtkWidget    *widget,
                                                       gint          position);

void            bar_itembar_reorder_child           (BarItembar *itembar,
                                                       GtkWidget    *widget,
                                                       gint          position);

gint            bar_itembar_get_child_index         (BarItembar *itembar,
                                                       GtkWidget    *widget);

guint           bar_itembar_get_n_children          (BarItembar *itembar);

guint           bar_itembar_get_drop_index          (BarItembar *itembar,
                                                       gint          x,
                                                       gint          y);

void            bar_itembar_set_drop_highlight_item (BarItembar *itembar,
                                                       gint          idx);

G_END_DECLS

#endif /* !__BAR_ITEMBAR_H__ */
