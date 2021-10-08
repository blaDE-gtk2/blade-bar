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

#ifndef __BAR_PREFERENCES_DIALOG_H__
#define __BAR_PREFERENCES_DIALOG_H__

#include <gtk/gtk.h>
#include <bar/bar-application.h>
#include <bar/bar-window.h>

G_BEGIN_DECLS

typedef struct _BarPreferencesDialogClass BarPreferencesDialogClass;
typedef struct _BarPreferencesDialog      BarPreferencesDialog;

#define BAR_TYPE_PREFERENCES_DIALOG            (bar_preferences_dialog_get_type ())
#define BAR_PREFERENCES_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BAR_TYPE_PREFERENCES_DIALOG, BarPreferencesDialog))
#define BAR_PREFERENCES_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BAR_TYPE_PREFERENCES_DIALOG, BarPreferencesDialogClass))
#define BAR_IS_PREFERENCES_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAR_TYPE_PREFERENCES_DIALOG))
#define BAR_IS_PREFERENCES_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BAR_TYPE_PREFERENCES_DIALOG))
#define BAR_PREFERENCES_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BAR_TYPE_PREFERENCES_DIALOG, BarPreferencesDialogClass))

GType      bar_preferences_dialog_get_type     (void) G_GNUC_CONST;

void       bar_preferences_dialog_show         (BarWindow *active);

void       bar_preferences_dialog_show_from_id (gint         bar_id,
                                                  const gchar *socket_id);

gboolean   bar_preferences_dialog_visible      (void);

G_END_DECLS

#endif /* !__BAR_PREFERENCES_DIALOG_H__ */
