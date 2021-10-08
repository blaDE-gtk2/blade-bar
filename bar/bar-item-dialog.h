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

#ifndef __BAR_ITEM_DIALOG_H__
#define __BAR_ITEM_DIALOG_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _BarItemDialogClass BarItemDialogClass;
typedef struct _BarItemDialog      BarItemDialog;

#define BAR_TYPE_ITEM_DIALOG            (bar_item_dialog_get_type ())
#define BAR_ITEM_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BAR_TYPE_ITEM_DIALOG, BarItemDialog))
#define BAR_ITEM_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BAR_TYPE_ITEM_DIALOG, BarItemDialogClass))
#define BAR_IS_ITEM_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAR_TYPE_ITEM_DIALOG))
#define BAR_IS_ITEM_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BAR_TYPE_ITEM_DIALOG))
#define BAR_ITEM_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BAR_TYPE_ITEM_DIALOG, BarItemDialogClass))

GType      bar_item_dialog_get_type     (void) G_GNUC_CONST;

void       bar_item_dialog_show         (BarWindow *active);

void       bar_item_dialog_show_from_id (gint         bar_id);

gboolean   bar_item_dialog_visible      (void);

G_END_DECLS

#endif /* !__BAR_ITEM_DIALOG_H__ */

