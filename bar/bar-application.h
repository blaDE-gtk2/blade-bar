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

#ifndef __BAR_APPLICATION_H__
#define __BAR_APPLICATION_H__

#include <gtk/gtk.h>
#include <bar/bar-window.h>

G_BEGIN_DECLS

typedef struct _BarApplicationClass BarApplicationClass;
typedef struct _BarApplication      BarApplication;

#define BAR_TYPE_APPLICATION            (bar_application_get_type ())
#define BAR_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BAR_TYPE_APPLICATION, BarApplication))
#define BAR_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BAR_TYPE_APPLICATION, BarApplicationClass))
#define BAR_IS_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAR_TYPE_APPLICATION))
#define BAR_IS_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BAR_TYPE_APPLICATION))
#define BAR_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BAR_TYPE_APPLICATION, BarApplicationClass))

typedef enum
{
  SAVE_PLUGIN_PROVIDERS = 1 << 1,
  SAVE_PLUGIN_IDS       = 1 << 2,
  SAVE_BAR_IDS        = 1 << 3,
}
BarSaveTypes;
#define SAVE_EVERYTHING (SAVE_PLUGIN_PROVIDERS | SAVE_PLUGIN_IDS | SAVE_BAR_IDS)


GType             bar_application_get_type          (void) G_GNUC_CONST;

BarApplication *bar_application_get               (void);

void              bar_application_load              (BarApplication  *application,
                                                       gboolean           disable_wm_check);

void              bar_application_save              (BarApplication  *application,
                                                       BarSaveTypes     save_types);

void              bar_application_save_window       (BarApplication  *application,
                                                       BarWindow       *window,
                                                       BarSaveTypes     save_types);

void              bar_application_take_dialog       (BarApplication  *application,
                                                       GtkWindow         *dialog);

void              bar_application_destroy_dialogs   (BarApplication  *application);

void              bar_application_add_new_item      (BarApplication  *application,
                                                       BarWindow       *window,
                                                       const gchar       *plugin_name,
                                                       gchar            **arguments);

BarWindow      *bar_application_new_window        (BarApplication  *application,
                                                       GdkScreen         *screen,
                                                       gint               id,
                                                       gboolean           new_window);

void              bar_application_remove_window     (BarApplication  *application,
                                                       BarWindow       *window);

GSList           *bar_application_get_windows       (BarApplication  *application);

BarWindow      *bar_application_get_window        (BarApplication  *application,
                                                       gint               bar_id);

void              bar_application_window_select     (BarApplication  *application,
                                                       BarWindow       *window);

void              bar_application_windows_blocked   (BarApplication  *application,
                                                       gboolean           blocked);

gboolean          bar_application_get_locked        (BarApplication  *application);

void              bar_application_logout            (void);

G_END_DECLS

#endif /* !__BAR_APPLICATION_H__ */
