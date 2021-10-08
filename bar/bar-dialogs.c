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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <blxo/blxo.h>
#include <libbladeutil/libbladeutil.h>
#include <libbladeui/libbladeui.h>

#include <common/bar-private.h>
#include <libbladebar/libbladebar.h>

#include <bar/bar-dialogs.h>
#include <bar/bar-application.h>
#include <bar/bar-tic-tac-toe.h>



static void
bar_dialogs_show_about_email_hook (GtkAboutDialog *dialog,
                                     const gchar    *uri,
                                     gpointer        data)
{
  if (g_strcmp0 ("tictactoe@xfce.org", uri) == 0)
    {
      /* open tic-tac-toe */
      bar_tic_tac_toe_show ();
    }
  else
    {
      blxo_gtk_url_about_dialog_hook (dialog, uri, data);
    }
}



void
bar_dialogs_show_about (void)
{
  gchar **authors;

  authors = g_new0 (gchar *, 4);
  authors[0] = g_strdup_printf ("%s:\n%s\n",
                                _("Maintainers"),
                                "Nick Schermer <nick@xfce.org>");
  authors[1] = g_strdup_printf ("%s:\n%s\n",
                                _("Deskbar Mode"),
                                "Andrzej Radecki <ndrwrdck@gmail.com>");
  authors[2] = g_strdup_printf ("%s:\n%s\n%s\n",
                                _("Inactive Maintainers"),
                                "Jasper Huijsmans <jasper@xfce.org>",
                                "Tic-tac-toe <tictactoe@xfce.org>");

  gtk_about_dialog_set_email_hook (bar_dialogs_show_about_email_hook, NULL, NULL);
#if !GTK_CHECK_VERSION (2, 18, 0)
  gtk_about_dialog_set_url_hook (blxo_gtk_url_about_dialog_hook, NULL, NULL);
#endif

  gtk_show_about_dialog (NULL,
                         "authors", authors,
                         "comments", _("The bar of the Xfce Desktop Environment"),
                         "copyright", "Copyright \302\251 2004-2012 Xfce Development Team",
                         "destroy-with-parent", TRUE,
                         "license", XFCE_LICENSE_GPL,
                         "program-name", PACKAGE_NAME,
                         "translator-credits", _("translator-credits"),
                         "version", PACKAGE_VERSION,
                         "website", "http://www.xfce.org/",
                         "logo-icon-name", PACKAGE_NAME,
                         NULL);

  g_strfreev (authors);
}



enum
{
  CHOOSER_COLUMN_ID,
  CHOOSER_COLUMN_TEXT,
  N_CHOOSER_COLUMNS
};



static gint
bar_dialogs_choose_bar_combo_get_id (GtkComboBox *combo)
{
  gint          bar_id = -1;
  GtkTreeIter   iter;
  GtkTreeModel *model;

  if (gtk_combo_box_get_active_iter (combo, &iter))
    {
      model = gtk_combo_box_get_model (combo);
      gtk_tree_model_get (model, &iter, CHOOSER_COLUMN_ID, &bar_id, -1);
    }

  return bar_id;
}



static void
bar_dialogs_choose_bar_combo_changed (GtkComboBox      *combo,
                                          BarApplication *application)
{
  gint bar_id;

  bar_return_if_fail (BAR_IS_APPLICATION (application));
  bar_return_if_fail (GTK_IS_COMBO_BOX (combo));

  bar_id = bar_dialogs_choose_bar_combo_get_id (combo);
  bar_application_window_select (application,
      bar_application_get_window (application, bar_id));
}



gint
bar_dialogs_choose_bar (BarApplication *application)
{
  GtkWidget       *dialog;
  GtkWidget       *vbox;
  GtkWidget       *label;
  GtkWidget       *combo;
  gchar           *name;
  GtkListStore    *store;
  GtkCellRenderer *renderer;
  GSList          *windows, *li;
  gint             i;
  gint             bar_id;

  bar_return_val_if_fail (BAR_IS_APPLICATION (application), -1);

  /* setup the dialog */
  dialog = gtk_dialog_new_with_buttons (_("Add New Item"), NULL,
                                        GTK_DIALOG_NO_SEPARATOR,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_ADD, GTK_RESPONSE_OK, NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
  gtk_window_set_icon_name (GTK_WINDOW (dialog), GTK_STOCK_ADD);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);

  /* create widgets */
  vbox = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), vbox, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);
  gtk_widget_show (vbox);

  label = gtk_label_new (_("Please choose a bar for the new plugin:"));
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  store = gtk_list_store_new (N_CHOOSER_COLUMNS, G_TYPE_INT, G_TYPE_STRING);
  combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (store));
  gtk_box_pack_start (GTK_BOX (vbox), combo, FALSE, FALSE, 0);
  gtk_widget_show (combo);
  g_object_unref (G_OBJECT (store));

  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), renderer,
                                  "text", CHOOSER_COLUMN_TEXT, NULL);

  /* insert the bars */
  windows = bar_application_get_windows (application);
  for (li = windows, i = 0; li != NULL; li = li->next, i++)
    {
      bar_id = bar_window_get_id (li->data);
      name = g_strdup_printf (_("Bar %d"), bar_id);
      gtk_list_store_insert_with_values (store, NULL, i,
                                         CHOOSER_COLUMN_ID, bar_id,
                                         CHOOSER_COLUMN_TEXT, name, -1);
      g_free (name);
    }

  /* select first bar (changed will start marching ants) */
  g_signal_connect (G_OBJECT (combo), "changed",
       G_CALLBACK (bar_dialogs_choose_bar_combo_changed), application);
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);

  /* run the dialog */
  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_OK)
    bar_id = bar_dialogs_choose_bar_combo_get_id (GTK_COMBO_BOX (combo));
  else
    bar_id = -1;
  gtk_widget_destroy (dialog);

  /* unset bar selections */
  bar_application_window_select (application, NULL);

  return bar_id;
}



gboolean
bar_dialogs_kiosk_warning (void)
{
  BarApplication *application;
  gboolean          locked;

  application = bar_application_get ();
  locked = bar_application_get_locked (application);
  g_object_unref (G_OBJECT (application));

  if (locked)
    {
      xfce_dialog_show_warning (NULL,
          _("Because the bar is running in kiosk mode, you are not allowed "
            "to make changes to the bar configuration as a regular user"),
          _("Modifying the bar is not allowed"));
    }

  return locked;
}
