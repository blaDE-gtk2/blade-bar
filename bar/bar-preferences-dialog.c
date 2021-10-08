/*
 * Copyright (C) 2008-2010 Nick Schermer <nick@xfce.org>
 * Copyright (C) 2014 Jannis Pohlmann <jannis@xfce.org>
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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <blxo/blxo.h>
#include <libbladeutil/libbladeutil.h>
#include <libbladeui/libbladeui.h>

#include <common/bar-private.h>
#include <common/bar-utils.h>

#include <libbladebar/libbladebar.h>
#include <libbladebar/blade-bar-plugin-provider.h>

#include <bar/bar-window.h>
#include <bar/bar-application.h>
#include <bar/bar-dialogs.h>
#include <bar/bar-module.h>
#include <bar/bar-itembar.h>
#include <bar/bar-item-dialog.h>
#include <bar/bar-preferences-dialog.h>
#include <bar/bar-preferences-dialog-ui.h>
#include <bar/bar-plugin-external.h>

#define PREFERENCES_HELP_URL "http://www.xfce.org"



static void                     bar_preferences_dialog_finalize               (GObject                *object);
static void                     bar_preferences_dialog_response               (GtkWidget              *window,
                                                                                 gint                    response_id,
                                                                                 BarPreferencesDialog *dialog);
static void                     bar_preferences_dialog_bindings_unbind        (BarPreferencesDialog *dialog);
static void                     bar_preferences_dialog_bindings_add           (BarPreferencesDialog *dialog,
                                                                                 const gchar            *property1,
                                                                                 const gchar            *property2);
static void                     bar_preferences_dialog_bindings_update        (BarPreferencesDialog *dialog);
static void                     bar_preferences_dialog_output_changed         (GtkComboBox            *combobox,
                                                                                 BarPreferencesDialog *dialog);
static void                     bar_preferences_dialog_autohide_changed       (GtkComboBox            *combobox,
                                                                                 BarPreferencesDialog *dialog);
static void                     bar_preferences_dialog_bg_style_changed       (BarPreferencesDialog *dialog);
static void                     bar_preferences_dialog_bg_image_file_set      (GtkFileChooserButton   *button,
                                                                                 BarPreferencesDialog *dialog);
static void                     bar_preferences_dialog_bg_image_notified      (BarPreferencesDialog *dialog);
static void                     bar_preferences_dialog_bar_combobox_changed (GtkComboBox            *combobox,
                                                                                 BarPreferencesDialog *dialog);
static gboolean                 bar_preferences_dialog_bar_combobox_rebuild (BarPreferencesDialog *dialog,
                                                                                 gint                    bar_id);
static void                     bar_preferences_dialog_bar_add              (GtkWidget              *widget,
                                                                                 BarPreferencesDialog *dialog);
static void                     bar_preferences_dialog_bar_remove           (GtkWidget              *widget,
                                                                                 BarPreferencesDialog *dialog);
static void                     bar_preferences_dialog_bar_switch           (GtkWidget              *widget,
                                                                                 BarPreferencesDialog *dialog);
static BladeBarPluginProvider *bar_preferences_dialog_item_get_selected      (BarPreferencesDialog *dialog,
                                                                                 GtkTreeIter            *return_iter);
static void                     bar_preferences_dialog_item_store_rebuild     (GtkWidget              *itembar,
                                                                                 BarPreferencesDialog *dialog);
static void                     bar_preferences_dialog_item_move              (GtkWidget              *button,
                                                                                 BarPreferencesDialog *dialog);
static void                     bar_preferences_dialog_item_remove            (GtkWidget              *button,
                                                                                 BarPreferencesDialog *dialog);
static void                     bar_preferences_dialog_item_add               (GtkWidget              *button,
                                                                                 BarPreferencesDialog *dialog);
static void                     bar_preferences_dialog_item_properties        (GtkWidget              *button,
                                                                                 BarPreferencesDialog *dialog);
static void                     bar_preferences_dialog_item_about             (GtkWidget              *button,
                                                                                 BarPreferencesDialog *dialog);
static gboolean                 bar_preferences_dialog_treeview_clicked       (GtkTreeView            *treeview,
                                                                                 GdkEventButton         *event,
                                                                                 BarPreferencesDialog *dialog);
static void                     bar_preferences_dialog_item_row_changed       (GtkTreeModel           *model,
                                                                                 GtkTreePath            *path,
                                                                                 GtkTreeIter            *iter,
                                                                                 BarPreferencesDialog *dialog);
static void                     bar_preferences_dialog_item_selection_changed (GtkTreeSelection       *selection,
                                                                                 BarPreferencesDialog *dialog);



enum
{
  ITEM_COLUMN_ICON_NAME,
  ITEM_COLUMN_DISPLAY_NAME,
  ITEM_COLUMN_TOOLTIP,
  ITEM_COLUMN_PROVIDER,
  N_ITEM_COLUMNS
};

enum
{
  OUTPUT_NAME,
  OUTPUT_TITLE
};

struct _BarPreferencesDialogClass
{
  GtkBuilderClass __parent__;
};

struct _BarPreferencesDialog
{
  GtkBuilder  __parent__;

  BarApplication *application;

  /* currently selected window in the selector */
  BarWindow      *active;

  /* BlxoMutualBinding's between dialog <-> window */
  GSList           *bindings;

  /* store for the items list */
  GtkListStore     *store;

  /* changed signal for the active bar's itembar */
  gulong            items_changed_handler_id;

  /* background image watch */
  gulong            bg_image_notify_handler_id;

  /* changed signal for the output selector */
  gulong            output_changed_handler_id;

  /* plug in which the dialog is embedded */
  GtkWidget        *socket_plug;
};



G_DEFINE_TYPE (BarPreferencesDialog, bar_preferences_dialog, GTK_TYPE_BUILDER)



static BarPreferencesDialog *dialog_singleton = NULL;



static void
bar_preferences_dialog_class_init (BarPreferencesDialogClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = bar_preferences_dialog_finalize;
}



static void
bar_preferences_dialog_init (BarPreferencesDialog *dialog)
{
  GObject           *window;
  GObject           *object;
  GObject           *info;
  GObject           *treeview;
  GtkTreeViewColumn *column;
  GtkCellRenderer   *renderer;
  GtkTreeSelection  *selection;
  gchar             *path;

  dialog->bindings = NULL;
  dialog->application = bar_application_get ();

  /* block autohide */
  bar_application_windows_blocked (dialog->application, TRUE);

  /* load the builder data into the object */
  gtk_builder_add_from_string (GTK_BUILDER (dialog), bar_preferences_dialog_ui,
                               bar_preferences_dialog_ui_length, NULL);

  /* get the dialog */
  window = gtk_builder_get_object (GTK_BUILDER (dialog), "dialog");
  bar_return_if_fail (GTK_IS_WIDGET (window));
  g_signal_connect (G_OBJECT (window), "response",
      G_CALLBACK (bar_preferences_dialog_response), dialog);

#define connect_signal(name,detail_signal,c_handler) \
  object = gtk_builder_get_object (GTK_BUILDER (dialog), name); \
  bar_return_if_fail (G_IS_OBJECT (object)); \
  g_signal_connect (G_OBJECT (object), detail_signal, G_CALLBACK (c_handler), dialog);

  /* bar selector buttons and combobox */
  connect_signal ("bar-add", "clicked", bar_preferences_dialog_bar_add);
  connect_signal ("bar-remove", "clicked", bar_preferences_dialog_bar_remove);
  connect_signal ("bar-combobox", "changed", bar_preferences_dialog_bar_combobox_changed);

  /* check if bar-switch is installed and if so show button */
  object = gtk_builder_get_object (GTK_BUILDER (dialog), "bar-switch");
  path = g_find_program_in_path ("xfbar-switch");
  if (path == NULL)
    gtk_widget_set_visible (GTK_WIDGET (object), FALSE);

  connect_signal ("bar-switch", "clicked", bar_preferences_dialog_bar_switch);

  /* style tab */
  object = gtk_builder_get_object (GTK_BUILDER (dialog), "background-style");
  bar_return_if_fail (G_IS_OBJECT (object));
  g_signal_connect_swapped (G_OBJECT (object), "changed",
      G_CALLBACK (bar_preferences_dialog_bg_style_changed), dialog);

  object = gtk_builder_get_object (GTK_BUILDER (dialog), "composited");
  bar_return_if_fail (G_IS_OBJECT (object));
  g_signal_connect_swapped (G_OBJECT (object), "notify::visible",
      G_CALLBACK (bar_preferences_dialog_bg_style_changed), dialog);

  info = gtk_builder_get_object (GTK_BUILDER (dialog), "composited-info");
  bar_return_if_fail (G_IS_OBJECT (info));
  blxo_binding_new_with_negation (G_OBJECT (object), "sensitive",
                                 G_OBJECT (info), "visible");

  object = gtk_builder_get_object (GTK_BUILDER (dialog), "background-image");
  bar_return_if_fail (GTK_IS_FILE_CHOOSER_BUTTON (object));
  g_signal_connect (G_OBJECT (object), "file-set",
    G_CALLBACK (bar_preferences_dialog_bg_image_file_set), dialog);

  /* items treeview and buttons */
  connect_signal ("item-up", "clicked", bar_preferences_dialog_item_move);
  connect_signal ("item-down", "clicked", bar_preferences_dialog_item_move);
  connect_signal ("item-remove", "clicked", bar_preferences_dialog_item_remove);
  connect_signal ("item-add", "clicked", bar_preferences_dialog_item_add);
  connect_signal ("item-properties", "clicked", bar_preferences_dialog_item_properties);
  connect_signal ("item-about", "clicked", bar_preferences_dialog_item_about);

  /* create store for bar items */
  dialog->store = gtk_list_store_new (N_ITEM_COLUMNS,
                                      G_TYPE_STRING, /* ITEM_COLUMN_ICON_NAME */
                                      G_TYPE_STRING, /* ITEM_COLUMN_DISPLAY_NAME */
                                      G_TYPE_STRING, /* ITEM_COLUMN_TOOLTIP */
                                      G_TYPE_OBJECT); /* ITEM_COLUMN_PROVIDER */

  /* build tree for bar items */
  treeview = gtk_builder_get_object (GTK_BUILDER (dialog), "item-treeview");
  bar_return_if_fail (GTK_IS_WIDGET (treeview));
  gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (dialog->store));
  gtk_tree_view_set_tooltip_column (GTK_TREE_VIEW (treeview), ITEM_COLUMN_TOOLTIP);
  g_signal_connect (G_OBJECT (treeview), "button-press-event",
      G_CALLBACK (bar_preferences_dialog_treeview_clicked), dialog);

  gtk_tree_view_set_reorderable (GTK_TREE_VIEW (treeview), TRUE);
  g_signal_connect (G_OBJECT (dialog->store), "row-changed",
      G_CALLBACK (bar_preferences_dialog_item_row_changed), dialog);

  /* setup tree selection */
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);
  g_signal_connect (G_OBJECT (selection), "changed",
      G_CALLBACK (bar_preferences_dialog_item_selection_changed), dialog);

  /* icon renderer */
  renderer = gtk_cell_renderer_pixbuf_new ();
  column = gtk_tree_view_column_new_with_attributes ("", renderer, "icon-name",
                                                     ITEM_COLUMN_ICON_NAME, NULL);
  g_object_set (G_OBJECT (renderer), "stock-size", GTK_ICON_SIZE_BUTTON, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

  /* text renderer */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new ();
  gtk_tree_view_column_pack_start (column, renderer, TRUE);
  gtk_tree_view_column_set_attributes (column, renderer, "markup",
                                       ITEM_COLUMN_DISPLAY_NAME, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

  /* connect the output changed signal */
  object = gtk_builder_get_object (GTK_BUILDER (dialog), "output-name");
  bar_return_if_fail (GTK_IS_COMBO_BOX (object));
  dialog->output_changed_handler_id =
      g_signal_connect (G_OBJECT (object), "changed",
                        G_CALLBACK (bar_preferences_dialog_output_changed),
                        dialog);

  /* connect the autohide behavior changed signal */
  object = gtk_builder_get_object (GTK_BUILDER (dialog), "autohide-behavior");
  bar_return_if_fail (GTK_IS_COMBO_BOX (object));
  g_signal_connect (G_OBJECT (object), "changed",
      G_CALLBACK (bar_preferences_dialog_autohide_changed), dialog);
}



static void
bar_preferences_dialog_finalize (GObject *object)
{
  BarPreferencesDialog *dialog = BAR_PREFERENCES_DIALOG (object);
  GtkWidget              *itembar;

  /* unblock autohide */
  bar_application_windows_blocked (dialog->application, FALSE);

  /* free bindings list */
  g_slist_free (dialog->bindings);

  /* destroy possible pluggable dialog */
  if (dialog->socket_plug != NULL)
    gtk_widget_destroy (dialog->socket_plug);

  if (dialog->active != NULL)
    {
      if (dialog->items_changed_handler_id != 0)
        {
          /* disconnect changed signal */
          itembar = gtk_bin_get_child (GTK_BIN (dialog->active));
          g_signal_handler_disconnect (G_OBJECT (itembar),
              dialog->items_changed_handler_id);
        }

      if (dialog->bg_image_notify_handler_id != 0)
        {
          g_signal_handler_disconnect (G_OBJECT (dialog->active),
              dialog->bg_image_notify_handler_id);
        }
    }

  /* deselect all windows */
  if (!bar_item_dialog_visible ())
    bar_application_window_select (dialog->application, NULL);

  g_object_unref (G_OBJECT (dialog->application));
  g_object_unref (G_OBJECT (dialog->store));

  (*G_OBJECT_CLASS (bar_preferences_dialog_parent_class)->finalize) (object);
}



static void
bar_preferences_dialog_response (GtkWidget              *window,
                                   gint                    response_id,
                                   BarPreferencesDialog *dialog)
{
  bar_return_if_fail (GTK_IS_DIALOG (window));
  bar_return_if_fail (BAR_IS_PREFERENCES_DIALOG (dialog));

  if (G_UNLIKELY (response_id == 1))
    {
      bar_utils_show_help (GTK_WINDOW (window), "preferences", NULL);
    }
  else
    {
      gtk_widget_destroy (window);
      g_object_unref (G_OBJECT (dialog));
    }
}



static void
bar_preferences_dialog_bindings_unbind (BarPreferencesDialog *dialog)
{
  GSList *li;

  if (dialog->bindings != NULL)
    {
      /* remove all bindings */
      for (li = dialog->bindings; li != NULL; li = li->next)
        blxo_mutual_binding_unbind (li->data);

      g_slist_free (dialog->bindings);
      dialog->bindings = NULL;
    }

  /* disconnect image watch */
  if (dialog->bg_image_notify_handler_id != 0)
    {
      if (dialog->active != NULL)
        {
          g_signal_handler_disconnect (G_OBJECT (dialog->active),
              dialog->bg_image_notify_handler_id);
        }

      dialog->bg_image_notify_handler_id = 0;
    }
}



static void
bar_preferences_dialog_bindings_add (BarPreferencesDialog *dialog,
                                       const gchar            *property1,
                                       const gchar            *property2)
{
  BlxoMutualBinding *binding;
  GObject          *object;

  /* get the object from the builder */
  object = gtk_builder_get_object (GTK_BUILDER (dialog), property1);
  bar_return_if_fail (G_IS_OBJECT (object));

  /* create the binding and prepend to the list */
  binding = blxo_mutual_binding_new (G_OBJECT (dialog->active), property1, object, property2);
  dialog->bindings = g_slist_prepend (dialog->bindings, binding);
}



static void
bar_preferences_dialog_bindings_update (BarPreferencesDialog *dialog)
{
  GdkScreen   *screen;
  GdkDisplay  *display;
  gint         n_screens, n_monitors = 1;
  GObject     *object;
  GObject     *store;
  gchar       *output_name = NULL;
  gboolean     selector_visible = TRUE;
  GtkTreeIter  iter;
  gboolean     output_selected = FALSE;
  gint         n = 0, i;
  gchar       *name, *title;
  gboolean     span_monitors_sensitive = FALSE;

  /* leave when there is no active bar */
  bar_return_if_fail (G_IS_OBJECT (dialog->active));
  if (dialog->active == NULL)
    return;

  /* hook up the bindings */
  bar_preferences_dialog_bindings_add (dialog, "mode", "active");
  bar_preferences_dialog_bindings_add (dialog, "span-monitors", "active");
  bar_preferences_dialog_bindings_add (dialog, "position-locked", "active");
  bar_preferences_dialog_bindings_add (dialog, "autohide-behavior", "active");
  bar_preferences_dialog_bindings_add (dialog, "disable-struts", "active");
  bar_preferences_dialog_bindings_add (dialog, "size", "value");
  bar_preferences_dialog_bindings_add (dialog, "nrows", "value");
  bar_preferences_dialog_bindings_add (dialog, "length", "value");
  bar_preferences_dialog_bindings_add (dialog, "length-adjust", "active");
  bar_preferences_dialog_bindings_add (dialog, "background-alpha", "value");
  bar_preferences_dialog_bindings_add (dialog, "enter-opacity", "value");
  bar_preferences_dialog_bindings_add (dialog, "leave-opacity", "value");
  bar_preferences_dialog_bindings_add (dialog, "composited", "sensitive");
  bar_preferences_dialog_bindings_add (dialog, "background-style", "active");
  bar_preferences_dialog_bindings_add (dialog, "background-color", "color");

  /* watch image changes from the bar */
  dialog->bg_image_notify_handler_id = g_signal_connect_swapped (G_OBJECT (dialog->active),
      "notify::background-image", G_CALLBACK (bar_preferences_dialog_bg_image_notified), dialog);
  bar_preferences_dialog_bg_image_notified (dialog);

  /* get run mode of the driver (multiple screens or randr) */
  screen = gtk_widget_get_screen (GTK_WIDGET (dialog->active));
  display = gtk_widget_get_display (GTK_WIDGET (dialog->active));
  n_screens = gdk_display_get_n_screens (display);
  n_monitors = 1;
  if (G_LIKELY (n_screens <= 1))
    {
      n_monitors = gdk_screen_get_n_monitors (screen);
    }

  /* update the output selector */
  object = gtk_builder_get_object (GTK_BUILDER (dialog), "output-name");
  bar_return_if_fail (GTK_IS_COMBO_BOX (object));

  g_signal_handler_block (G_OBJECT (object), dialog->output_changed_handler_id);

  store = gtk_builder_get_object (GTK_BUILDER (dialog), "output-store");
  bar_return_if_fail (GTK_IS_LIST_STORE (store));
  gtk_list_store_clear (GTK_LIST_STORE (store));

  g_object_get (G_OBJECT (dialog->active), "output-name", &output_name, NULL);

  if (n_screens > 1
      || n_monitors > 1
      || !blxo_str_is_empty (output_name))
    {
      gtk_list_store_insert_with_values (GTK_LIST_STORE (store), &iter, n++,
                                         OUTPUT_NAME, "Automatic",
                                         OUTPUT_TITLE, _("Automatic"), -1);
      if (blxo_str_is_empty (output_name) ||
          g_strcmp0 (output_name, "Automatic") == 0)
        {
          gtk_combo_box_set_active_iter (GTK_COMBO_BOX (object), &iter);
          output_selected = TRUE;
          span_monitors_sensitive = TRUE;
        }
      gtk_list_store_insert_with_values (GTK_LIST_STORE (store), &iter, n++,
                                         OUTPUT_NAME, "Primary",
                                         OUTPUT_TITLE, _("Primary"), -1);
      if (g_strcmp0 (output_name, "Primary") == 0)
        {
          gtk_combo_box_set_active_iter (GTK_COMBO_BOX (object), &iter);
          output_selected = TRUE;
          span_monitors_sensitive = TRUE;
        }

      if (n_screens > 1)
        {
          for (i = 0; i < n_screens; i++)
            {
              /* warn the user about layouts we don't support */
              screen = gdk_display_get_screen (display, i);
              if (gdk_screen_get_n_monitors (screen) > 1)
                g_message ("Screen %d has multiple monitors, the bar does not "
                           "support such a configuration", i + 1);

              /* I18N: screen name in the output selector */
              title = g_strdup_printf (_("Screen %d"), i + 1);
              name = g_strdup_printf ("screen-%d", i);
              gtk_list_store_insert_with_values (GTK_LIST_STORE (store), &iter, n++,
                                                 OUTPUT_NAME, name,
                                                 OUTPUT_TITLE, title, -1);

              if (!output_selected && blxo_str_is_equal (name, output_name))
                {
                  gtk_combo_box_set_active_iter  (GTK_COMBO_BOX (object), &iter);
                  output_selected = TRUE;
                }

              g_free (name);
              g_free (title);
            }
        }
      else if (n_monitors >= 1)
        {
          for (i = 0; i < n_monitors; i++)
            {
              name = gdk_screen_get_monitor_plug_name (screen, i);
              if (blxo_str_is_empty (name))
                {
                  g_free (name);

                  /* I18N: monitor name in the output selector */
                  title = g_strdup_printf (_("Monitor %d"), i + 1);
                  name = g_strdup_printf ("monitor-%d", i);
                }
              else
                {
                  /* use the randr name for the title */
                  title = g_strdup (name);
                }

              gtk_list_store_insert_with_values (GTK_LIST_STORE (store), &iter, n++,
                                                 OUTPUT_NAME, name,
                                                 OUTPUT_TITLE, title, -1);
              if (!output_selected && blxo_str_is_equal (name, output_name))
                {
                  gtk_combo_box_set_active_iter  (GTK_COMBO_BOX (object), &iter);
                  output_selected = TRUE;
                }

              g_free (name);
              g_free (title);
            }
        }

      /* add the output from the config if still nothing has been selected */
      if (!output_selected && !blxo_str_is_empty (output_name))
        {
          gtk_list_store_insert_with_values (GTK_LIST_STORE (store), &iter, n++,
                                             OUTPUT_NAME, output_name,
                                             OUTPUT_TITLE, output_name, -1);
          gtk_combo_box_set_active_iter  (GTK_COMBO_BOX (object), &iter);
        }
    }
  else
    {
      /* hide the selector */
      selector_visible = FALSE;
      span_monitors_sensitive = TRUE;
    }

  g_signal_handler_unblock (G_OBJECT (object), dialog->output_changed_handler_id);

  /* update visibility of the output selector */
  object = gtk_builder_get_object (GTK_BUILDER (dialog), "output-box");
  bar_return_if_fail (GTK_IS_WIDGET (object));
  g_object_set (G_OBJECT (object), "visible", selector_visible, NULL);

  /* monitor spanning is only active when no output is selected */
  object = gtk_builder_get_object (GTK_BUILDER (dialog), "span-monitors");
  bar_return_if_fail (GTK_IS_WIDGET (object));
  gtk_widget_set_sensitive (GTK_WIDGET (object), span_monitors_sensitive);
  g_object_set (G_OBJECT (object), "visible", n_monitors > 1, NULL);

  g_free (output_name);

  /* update sensitivity of "don't reserve space on borders" option */
  object = gtk_builder_get_object (GTK_BUILDER (dialog), "autohide-behavior");
  bar_return_if_fail (GTK_IS_COMBO_BOX (object));
  bar_preferences_dialog_autohide_changed (GTK_COMBO_BOX (object), dialog);
}



static void
bar_preferences_dialog_output_changed (GtkComboBox            *combobox,
                                         BarPreferencesDialog *dialog)
{
  GtkTreeIter   iter;
  GtkTreeModel *model;
  gchar        *output_name = NULL;
  GObject      *object;

  bar_return_if_fail (GTK_IS_COMBO_BOX (combobox));
  bar_return_if_fail (BAR_IS_PREFERENCES_DIALOG (dialog));
  bar_return_if_fail (BAR_WINDOW (dialog->active));

  if (gtk_combo_box_get_active_iter (combobox, &iter))
    {
      model = gtk_combo_box_get_model (combobox);
      gtk_tree_model_get (model, &iter, OUTPUT_NAME, &output_name, -1);
      g_object_set (G_OBJECT (dialog->active), "output-name", output_name, NULL);

      /* monitor spanning does not work when an output is selected */
      object = gtk_builder_get_object (GTK_BUILDER (dialog), "span-monitors");
      bar_return_if_fail (GTK_IS_WIDGET (object));
      gtk_widget_set_sensitive (GTK_WIDGET (object), output_name == NULL);

      g_free (output_name);
    }
}



static void
bar_preferences_dialog_autohide_changed (GtkComboBox            *combobox,
                                           BarPreferencesDialog *dialog)
{
  GObject *object;

  bar_return_if_fail (GTK_IS_COMBO_BOX (combobox));
  bar_return_if_fail (BAR_IS_PREFERENCES_DIALOG (dialog));
  bar_return_if_fail (BAR_WINDOW (dialog->active));

  object = gtk_builder_get_object (GTK_BUILDER (dialog), "disable-struts");
  bar_return_if_fail (GTK_IS_WIDGET (object));

  /* make "don't reserve space on borders" sensitive only when autohide is disabled */
  if (gtk_combo_box_get_active (combobox) == 0)
    gtk_widget_set_sensitive (GTK_WIDGET (object), TRUE);
  else
    gtk_widget_set_sensitive (GTK_WIDGET (object), FALSE);
}



static void
bar_preferences_dialog_bg_style_changed (BarPreferencesDialog *dialog)
{
  gint      active;
  GObject  *object;
  gboolean  composited;

  bar_return_if_fail (BAR_IS_PREFERENCES_DIALOG (dialog));
  bar_return_if_fail (BAR_WINDOW (dialog->active));

  object = gtk_builder_get_object (GTK_BUILDER (dialog), "background-style");
  bar_return_if_fail (GTK_IS_COMBO_BOX (object));
  active = gtk_combo_box_get_active (GTK_COMBO_BOX (object));

  object = gtk_builder_get_object (GTK_BUILDER (dialog), "bg-alpha-box");
  bar_return_if_fail (GTK_IS_WIDGET (object));
  g_object_get (G_OBJECT (dialog->active), "composited", &composited, NULL);
  g_object_set (G_OBJECT (object), "visible", active < 2,
                "sensitive", composited, NULL);

  object = gtk_builder_get_object (GTK_BUILDER (dialog), "bg-color-box");
  bar_return_if_fail (GTK_IS_WIDGET (object));
  g_object_set (G_OBJECT (object), "visible", active == 1, NULL);

  object = gtk_builder_get_object (GTK_BUILDER (dialog), "bg-image-box");
  bar_return_if_fail (GTK_IS_WIDGET (object));
  g_object_set (G_OBJECT (object), "visible", active == 2, NULL);
}



static void
bar_preferences_dialog_bg_image_file_set (GtkFileChooserButton   *button,
                                            BarPreferencesDialog *dialog)
{
  gchar *filename;

  bar_return_if_fail (GTK_IS_FILE_CHOOSER_BUTTON (button));
  bar_return_if_fail (BAR_IS_PREFERENCES_DIALOG (dialog));
  bar_return_if_fail (BAR_IS_WINDOW (dialog->active));

  g_signal_handler_block (G_OBJECT (dialog->active),
      dialog->bg_image_notify_handler_id);

  filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (button));
  g_object_set (G_OBJECT (dialog->active), "background-image", filename, NULL);
  g_free (filename);

  g_signal_handler_unblock (G_OBJECT (dialog->active),
      dialog->bg_image_notify_handler_id);
}



static void
bar_preferences_dialog_bg_image_notified (BarPreferencesDialog *dialog)
{
  gchar   *filename;
  GObject *button;

  bar_return_if_fail (BAR_IS_PREFERENCES_DIALOG (dialog));
  bar_return_if_fail (BAR_IS_WINDOW (dialog->active));

  button = gtk_builder_get_object (GTK_BUILDER (dialog), "background-image");
  bar_return_if_fail (GTK_IS_FILE_CHOOSER_BUTTON (button));

  g_signal_handlers_block_by_func (G_OBJECT (button),
      G_CALLBACK (bar_preferences_dialog_bg_image_file_set), dialog);

  g_object_get (G_OBJECT (dialog->active), "background-image", &filename, NULL);
  gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (button), filename != NULL ? filename : "");
  g_free (filename);

  g_signal_handlers_unblock_by_func (G_OBJECT (button),
      G_CALLBACK (bar_preferences_dialog_bg_image_file_set), dialog);
}



static void
bar_preferences_dialog_bar_sensitive (BarPreferencesDialog *dialog)
{

  GObject  *object;
  gboolean  locked = TRUE;
  GSList   *windows;

  bar_return_if_fail (BAR_IS_PREFERENCES_DIALOG (dialog));

  if (G_LIKELY (dialog->active != NULL))
    locked = bar_window_get_locked (dialog->active);

  object = gtk_builder_get_object (GTK_BUILDER (dialog), "bar-remove");
  bar_return_if_fail (GTK_IS_WIDGET (object));
  windows = bar_application_get_windows (dialog->application);
  gtk_widget_set_sensitive (GTK_WIDGET (object),
      !locked && g_slist_length (windows) > 1);

  object = gtk_builder_get_object (GTK_BUILDER (dialog), "bar-add");
  bar_return_if_fail (GTK_IS_WIDGET (object));
  gtk_widget_set_sensitive (GTK_WIDGET (object),
      !bar_application_get_locked (dialog->application));

  object = gtk_builder_get_object (GTK_BUILDER (dialog), "notebook");
  bar_return_if_fail (GTK_IS_WIDGET (object));
  gtk_widget_set_sensitive (GTK_WIDGET (object), !locked);

  object = gtk_builder_get_object (GTK_BUILDER (dialog), "item-add");
  bar_return_if_fail (GTK_IS_WIDGET (object));
  gtk_widget_set_sensitive (GTK_WIDGET (object), !locked);
}



static void
bar_preferences_dialog_bar_combobox_changed (GtkComboBox            *combobox,
                                                 BarPreferencesDialog *dialog)
{
  gint          bar_id;
  GtkWidget    *itembar;
  GtkTreeModel *model;
  GtkTreeIter   iter;

  bar_return_if_fail (GTK_IS_COMBO_BOX (combobox));
  bar_return_if_fail (BAR_IS_PREFERENCES_DIALOG (dialog));

  /* disconnect signal we used to monitor changes in the itembar */
  if (dialog->active != NULL && dialog->items_changed_handler_id != 0)
    {
      itembar = gtk_bin_get_child (GTK_BIN (dialog->active));
      g_signal_handler_disconnect (G_OBJECT (itembar), dialog->items_changed_handler_id);
    }

  /* remove all the active bindings */
  bar_preferences_dialog_bindings_unbind (dialog);

  /* set the selected window */
  if (gtk_combo_box_get_active_iter (combobox, &iter))
    {
      model = gtk_combo_box_get_model (combobox);
      gtk_tree_model_get (model, &iter, 0, &bar_id, -1);

      dialog->active = bar_application_get_window (dialog->application, bar_id);
    }
  else
    {
      dialog->active = NULL;
    }

  bar_application_window_select (dialog->application, dialog->active);

  if (G_LIKELY (dialog->active != NULL))
    {
      itembar = gtk_bin_get_child (GTK_BIN (dialog->active));
      dialog->items_changed_handler_id =
          g_signal_connect (G_OBJECT (itembar), "changed",
                            G_CALLBACK (bar_preferences_dialog_item_store_rebuild),
                            dialog);

      /* rebind the dialog bindings */
      bar_preferences_dialog_bindings_update (dialog);

      /* update the items treeview */
      bar_preferences_dialog_item_store_rebuild (itembar, dialog);
    }

  bar_preferences_dialog_bar_sensitive (dialog);
}



static gboolean
bar_preferences_dialog_bar_combobox_rebuild (BarPreferencesDialog *dialog,
                                                 gint                    bar_id)
{
  GObject     *store, *combo;
  gint         i;
  GSList      *windows, *li;
  gchar       *name;
  gint         id;
  GtkTreeIter  iter;
  gboolean     selected = FALSE;

  /* get the combo box and model */
  store = gtk_builder_get_object (GTK_BUILDER (dialog), "bar-store");
  bar_return_val_if_fail (GTK_IS_LIST_STORE (store), FALSE);
  combo = gtk_builder_get_object (GTK_BUILDER (dialog), "bar-combobox");
  bar_return_val_if_fail (GTK_IS_COMBO_BOX (combo), FALSE);

  /* block signal */
  g_signal_handlers_block_by_func (combo,
      bar_preferences_dialog_bar_combobox_changed, dialog);

  /* empty the combo box */
  gtk_list_store_clear (GTK_LIST_STORE (store));

  /* add new names */
  windows = bar_application_get_windows (dialog->application);
  for (li = windows, i = 0; li != NULL; li = li->next, i++)
    {
      /* I18N: bar combo box in the preferences dialog */
      id = bar_window_get_id (li->data);
      name = g_strdup_printf (_("Bar %d"), id);
      gtk_list_store_insert_with_values (GTK_LIST_STORE (store), &iter, i,
                                         0, id, 1, name, -1);
      g_free (name);

      if (id == bar_id)
        {
          /* select bar id */
          gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);
          selected = TRUE;
        }
    }

  /* unblock signal */
  g_signal_handlers_unblock_by_func (combo,
      bar_preferences_dialog_bar_combobox_changed, dialog);

  if (selected)
    bar_preferences_dialog_bar_combobox_changed (GTK_COMBO_BOX (combo), dialog);

  bar_preferences_dialog_bar_sensitive (dialog);

  return selected;
}



static void
bar_preferences_dialog_bar_add (GtkWidget              *widget,
                                    BarPreferencesDialog *dialog)
{
  BarWindow *window;
  gint         bar_id;

  /* create new window */
  window = bar_application_new_window (dialog->application,
      gtk_widget_get_screen (widget), -1, TRUE);

  /* block autohide */
  bar_window_freeze_autohide (window);

  /* show window */
  gtk_widget_show (GTK_WIDGET (window));

  /* rebuild the selector */
  bar_id = bar_window_get_id (window);
  bar_preferences_dialog_bar_combobox_rebuild (dialog, bar_id);
}



static void
bar_preferences_dialog_bar_remove (GtkWidget              *widget,
                                       BarPreferencesDialog *dialog)
{
  gint       idx;
  GtkWidget *toplevel;
  GSList    *windows;
  gint       n_windows;

  /* leave if the window is locked */
  if (bar_window_get_locked (dialog->active))
    return;

  toplevel = gtk_widget_get_toplevel (widget);
  if (xfce_dialog_confirm (GTK_WINDOW (toplevel), GTK_STOCK_REMOVE, NULL,
          _("The bar and plugin configurations will be permanently removed"),
          _("Are you sure you want to remove bar %d?"),
          bar_window_get_id (dialog->active)))
    {
      /* release the bindings */
      bar_preferences_dialog_bindings_unbind (dialog);

      /* get position of the bar */
      windows = bar_application_get_windows (dialog->application);
      idx = g_slist_index (windows, dialog->active);
      n_windows = g_slist_length (windows) - 2;

      /* remove the bar, plugins and configuration */
      bar_application_remove_window (dialog->application,
                                       dialog->active);
      dialog->active = NULL;

      /* rebuild the selector */
      bar_preferences_dialog_bar_combobox_rebuild (dialog, CLAMP (idx, 0, n_windows));
    }
}



static void
bar_preferences_dialog_bar_switch (GtkWidget *widget, BarPreferencesDialog *dialog)
{
  GtkWidget *toplevel;
  gchar     *path;
  GError    *error = NULL;

  path = g_find_program_in_path ("xfbar-switch");
  if (path == NULL)
    return;

  /* close the preferences dialog */
  toplevel = gtk_widget_get_toplevel (widget);
  bar_preferences_dialog_response (toplevel, 0, dialog);

  /* run xfbar-switch */
  g_spawn_command_line_async (path, &error);
}


static BladeBarPluginProvider *
bar_preferences_dialog_item_get_selected (BarPreferencesDialog *dialog,
                                            GtkTreeIter            *return_iter)
{
  GObject                 *treeview;
  BladeBarPluginProvider *provider = NULL;
  GtkTreeModel            *model;
  GtkTreeIter              iter;
  GtkTreeSelection        *selection;

  bar_return_val_if_fail (BAR_IS_PREFERENCES_DIALOG (dialog), NULL);

  /* get the treeview selection */
  treeview = gtk_builder_get_object (GTK_BUILDER (dialog), "item-treeview");
  bar_return_val_if_fail (GTK_IS_WIDGET (treeview), NULL);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));

  /* get the selection item */
  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      /* get the selected provider */
      gtk_tree_model_get (model, &iter, ITEM_COLUMN_PROVIDER, &provider, -1);
      bar_return_val_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider), NULL);

      if (return_iter)
        *return_iter = iter;
    }

  return provider;
}



static void
bar_preferences_dialog_item_store_rebuild (GtkWidget              *itembar,
                                             BarPreferencesDialog *dialog)
{
  GList                   *items, *li;
  guint                    i;
  BarModule             *module;
  gchar                   *tooltip, *display_name;
  BladeBarPluginProvider *selected_provider;
  GtkTreeIter              iter;
  GtkTreePath             *path;
  GObject                 *treeview;

  bar_return_if_fail (BAR_IS_PREFERENCES_DIALOG (dialog));
  bar_return_if_fail (GTK_IS_LIST_STORE (dialog->store));
  bar_return_if_fail (BAR_IS_ITEMBAR (itembar));

  /* memorize selected item */
  selected_provider = bar_preferences_dialog_item_get_selected (dialog, NULL);

  gtk_list_store_clear (dialog->store);

  g_signal_handlers_block_by_func (G_OBJECT (dialog->store),
      G_CALLBACK (bar_preferences_dialog_item_row_changed), dialog);

  /* add items to the store */
  items = gtk_container_get_children (GTK_CONTAINER (itembar));
  for (li = items, i = 0; li != NULL; li = li->next, i++)
    {
      /* get the bar module from the plugin */
      module = bar_module_get_from_plugin_provider (li->data);

      if (BAR_IS_PLUGIN_EXTERNAL (li->data))
        {
          /* I18N: append (external) in the preferences dialog if the plugin
           * runs external */
          display_name = g_strdup_printf (_("%s <span color=\"grey\" size=\"small\">(external)</span>"),
                                          bar_module_get_display_name (module));

          /* I18N: tooltip in preferences dialog when hovering an item in the list
           * for external plugins */
          tooltip = g_strdup_printf (_("Internal name: %s-%d\n"
                                       "PID: %d"),
                                     blade_bar_plugin_provider_get_name (li->data),
                                     blade_bar_plugin_provider_get_unique_id (li->data),
                                     bar_plugin_external_get_pid (BAR_PLUGIN_EXTERNAL (li->data)));
        }
      else
        {
          display_name = g_strdup (bar_module_get_display_name (module));

          /* I18N: tooltip in preferences dialog when hovering an item in the list
           * for internal plugins */
          tooltip = g_strdup_printf (_("Internal name: %s-%d"),
                                     blade_bar_plugin_provider_get_name (li->data),
                                     blade_bar_plugin_provider_get_unique_id (li->data));
        }

      gtk_list_store_insert_with_values (dialog->store, &iter, i,
                                         ITEM_COLUMN_ICON_NAME,
                                         bar_module_get_icon_name (module),
                                         ITEM_COLUMN_DISPLAY_NAME,
                                         display_name,
                                         ITEM_COLUMN_TOOLTIP,
                                         tooltip,
                                         ITEM_COLUMN_PROVIDER, li->data, -1);

      /* reconstruct selection */
      if (selected_provider == li->data)
        {
          path = gtk_tree_model_get_path (GTK_TREE_MODEL (dialog->store), &iter);
          treeview = gtk_builder_get_object (GTK_BUILDER (dialog), "item-treeview");
          if (GTK_IS_WIDGET (treeview))
            gtk_tree_view_set_cursor (GTK_TREE_VIEW (treeview), path, NULL, FALSE);
          gtk_tree_path_free (path);
        }

      g_free (tooltip);
      g_free (display_name);
    }

  g_list_free (items);

  g_signal_handlers_unblock_by_func (G_OBJECT (dialog->store),
      G_CALLBACK (bar_preferences_dialog_item_row_changed), dialog);
}



static void
bar_preferences_dialog_item_move (GtkWidget              *button,
                                    BarPreferencesDialog *dialog)
{
  GObject                 *treeview, *object;
  GtkTreeSelection        *selection;
  GtkTreeIter              iter_a, iter_b;
  BladeBarPluginProvider *provider;
  GtkWidget               *itembar;
  gint                     position;
  gint                     direction;
  GtkTreePath             *path;

  bar_return_if_fail (BAR_IS_PREFERENCES_DIALOG (dialog));

  /* direction */
  object = gtk_builder_get_object (GTK_BUILDER (dialog), "item-up");
  bar_return_if_fail (GTK_IS_WIDGET (object));
  direction = G_OBJECT (button) == object ? -1 : 1;

  provider = bar_preferences_dialog_item_get_selected (dialog, &iter_a);
  if (G_LIKELY (provider != NULL))
    {
      /* get the provider position on the bar */
      itembar = gtk_bin_get_child (GTK_BIN (dialog->active));
      position = bar_itembar_get_child_index (BAR_ITEMBAR (itembar),
                                                GTK_WIDGET (provider));
      path = gtk_tree_model_get_path (GTK_TREE_MODEL (dialog->store), &iter_a);

      if (G_LIKELY (position != -1))
        {
          /* block the changed signal */
          g_signal_handler_block (G_OBJECT (itembar), dialog->items_changed_handler_id);

          /* move the item on the bar */
          bar_itembar_reorder_child (BAR_ITEMBAR (itembar),
                                       GTK_WIDGET (provider),
                                       position + direction);

          /* save the new ids */
          bar_application_save_window (dialog->application,
                                         dialog->active,
                                         SAVE_PLUGIN_IDS);

          /* unblock the changed signal */
          g_signal_handler_unblock (G_OBJECT (itembar), dialog->items_changed_handler_id);

          /* move the item up or down in the list */
          if (direction == 1)
            {
              /* swap the items in the list */
              iter_b = iter_a;
              if (gtk_tree_model_iter_next (GTK_TREE_MODEL (dialog->store), &iter_b))
                {
                  gtk_list_store_swap (dialog->store, &iter_a, &iter_b);
                  gtk_tree_path_next (path);
                }
            }
          else
            {
              /* get the previous item in the list */
              if (gtk_tree_path_prev (path))
                {
                  /* swap the items in the list */
                  gtk_tree_model_get_iter (GTK_TREE_MODEL (dialog->store), &iter_b, path);
                  gtk_list_store_swap (dialog->store, &iter_a, &iter_b);
                }
            }

          /* fake update the selection */
          treeview = gtk_builder_get_object (GTK_BUILDER (dialog), "item-treeview");
          bar_return_if_fail (GTK_IS_WIDGET (treeview));
          selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
          bar_preferences_dialog_item_selection_changed (selection, dialog);

          /* make the new selected position visible if moved out of area */
          gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (treeview), path, NULL, FALSE, 0, 0);
          gtk_tree_view_set_cursor (GTK_TREE_VIEW (treeview), path, NULL, FALSE);

        }
      gtk_tree_path_free (path);
    }
}



static void
bar_preferences_dialog_item_remove (GtkWidget              *button,
                                      BarPreferencesDialog *dialog)
{
  BladeBarPluginProvider *provider;
  GtkWidget               *widget, *toplevel;
  BarModule             *module;

  bar_return_if_fail (BAR_IS_PREFERENCES_DIALOG (dialog));

  provider = bar_preferences_dialog_item_get_selected (dialog, NULL);
  if (G_LIKELY (provider != NULL))
    {
      module = bar_module_get_from_plugin_provider (provider);

      /* create question dialog (same code is also in blade-bar-plugin.c) */
      toplevel = gtk_widget_get_toplevel (button);
      widget = gtk_message_dialog_new (GTK_WINDOW (toplevel), GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
                                       _("Are you sure that you want to remove \"%s\"?"),
                                       bar_module_get_display_name (module));
      gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (widget),
                                                _("If you remove the item from the bar, "
                                                  "it is permanently lost."));
      gtk_dialog_add_buttons (GTK_DIALOG (widget), GTK_STOCK_CANCEL, GTK_RESPONSE_NO,
                              GTK_STOCK_REMOVE, GTK_RESPONSE_YES, NULL);
      gtk_dialog_set_default_response (GTK_DIALOG (widget), GTK_RESPONSE_NO);

      /* run the dialog */
      if (gtk_dialog_run (GTK_DIALOG (widget)) == GTK_RESPONSE_YES)
        {
          gtk_widget_hide (widget);
          blade_bar_plugin_provider_emit_signal (provider, PROVIDER_SIGNAL_REMOVE_PLUGIN);
        }

      gtk_widget_destroy (widget);
    }
}



static void
bar_preferences_dialog_item_add (GtkWidget              *button,
                                   BarPreferencesDialog *dialog)
{
  bar_return_if_fail (BAR_IS_PREFERENCES_DIALOG (dialog));

  bar_item_dialog_show (dialog->active);
}



static void
bar_preferences_dialog_item_properties (GtkWidget              *button,
                                          BarPreferencesDialog *dialog)
{
  BladeBarPluginProvider *provider;

  bar_return_if_fail (BAR_IS_PREFERENCES_DIALOG (dialog));

  provider = bar_preferences_dialog_item_get_selected (dialog, NULL);
  if (G_LIKELY (provider != NULL))
    blade_bar_plugin_provider_show_configure (provider);
}



static void
bar_preferences_dialog_item_about (GtkWidget              *button,
                                     BarPreferencesDialog *dialog)
{
  BladeBarPluginProvider *provider;

  bar_return_if_fail (BAR_IS_PREFERENCES_DIALOG (dialog));

  provider = bar_preferences_dialog_item_get_selected (dialog, NULL);
  if (G_LIKELY (provider != NULL))
    blade_bar_plugin_provider_show_about (provider);
}



static gboolean
bar_preferences_dialog_treeview_clicked (GtkTreeView            *treeview,
                                           GdkEventButton         *event,
                                           BarPreferencesDialog *dialog)
{
  gint x, y;

  bar_return_val_if_fail (BAR_IS_PREFERENCES_DIALOG (dialog), FALSE);
  bar_return_val_if_fail (GTK_IS_TREE_VIEW (treeview), FALSE);

  gtk_tree_view_convert_widget_to_bin_window_coords (treeview,
                                                     event->x, event->y,
                                                     &x, &y);

  /* open preferences on double-click on a row */
  if (event->type == GDK_2BUTTON_PRESS
      && event->button == 1
      && gtk_tree_view_get_path_at_pos (treeview, x, y, NULL, NULL, NULL, NULL))
    {
      bar_preferences_dialog_item_properties (NULL, dialog);
      return TRUE;
    }

  return FALSE;
}



static void
bar_preferences_dialog_item_row_changed (GtkTreeModel           *model,
                                           GtkTreePath            *path,
                                           GtkTreeIter            *iter,
                                           BarPreferencesDialog *dialog)
{
  BladeBarPluginProvider *provider = NULL;
  gint                     position;
  GtkWidget               *itembar;
  gint                     store_position;

  bar_return_if_fail (BAR_IS_PREFERENCES_DIALOG (dialog));
  bar_return_if_fail (GTK_TREE_MODEL (dialog->store) == model);
  bar_return_if_fail (BAR_IS_WINDOW (dialog->active));

  /* get the changed row */
  gtk_tree_model_get (model, iter, ITEM_COLUMN_PROVIDER, &provider, -1);
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (provider));
  store_position = gtk_tree_path_get_indices (path)[0];

  /* actual position on the bar */
  itembar = gtk_bin_get_child (GTK_BIN (dialog->active));
  position = bar_itembar_get_child_index (BAR_ITEMBAR (itembar),
                                            GTK_WIDGET (provider));

  /* correct position in the list */
  if (position < store_position)
    store_position--;

  /* move the item on the bar */
  if (position != store_position)
    {
      bar_itembar_reorder_child (BAR_ITEMBAR (itembar),
                                   GTK_WIDGET (provider),
                                   store_position);

      bar_application_save_window (dialog->application,
                                     dialog->active,
                                     SAVE_PLUGIN_IDS);
    }
}



static void
bar_preferences_dialog_item_selection_changed (GtkTreeSelection       *selection,
                                                 BarPreferencesDialog *dialog)
{
  BladeBarPluginProvider *provider;
  GtkWidget               *itembar;
  gint                     position;
  gint                     items;
  gboolean                 active;
  GObject                 *object;
  guint                    i;
  const gchar             *button_names[] = { "item-remove", "item-up",
                                              "item-down", "item-about",
                                              "item-properties" };

  bar_return_if_fail (BAR_IS_PREFERENCES_DIALOG (dialog));

  provider = bar_preferences_dialog_item_get_selected (dialog, NULL);
  if (G_LIKELY (provider != NULL))
    {
      /* get the current position and the items on the bar */
      itembar = gtk_bin_get_child (GTK_BIN (dialog->active));
      position = bar_itembar_get_child_index (BAR_ITEMBAR (itembar), GTK_WIDGET (provider));
      items = bar_itembar_get_n_children (BAR_ITEMBAR (itembar)) - 1;

      /* update sensitivity of buttons */
      object = gtk_builder_get_object (GTK_BUILDER (dialog), "item-up");
      bar_return_if_fail (GTK_IS_WIDGET (object));
      gtk_widget_set_sensitive (GTK_WIDGET (object), !!(position > 0 && position <= items));

      object = gtk_builder_get_object (GTK_BUILDER (dialog), "item-down");
      bar_return_if_fail (GTK_IS_WIDGET (object));
      gtk_widget_set_sensitive (GTK_WIDGET (object), !!(position >= 0 && position < items));

      object = gtk_builder_get_object (GTK_BUILDER (dialog), "item-remove");
      bar_return_if_fail (GTK_IS_WIDGET (object));
      gtk_widget_set_sensitive (GTK_WIDGET (object), TRUE);

      object = gtk_builder_get_object (GTK_BUILDER (dialog), "item-properties");
      bar_return_if_fail (GTK_IS_WIDGET (object));
      active = blade_bar_plugin_provider_get_show_configure (provider);
      gtk_widget_set_sensitive (GTK_WIDGET (object), active);

      object = gtk_builder_get_object (GTK_BUILDER (dialog), "item-about");
      bar_return_if_fail (GTK_IS_WIDGET (object));
      active = blade_bar_plugin_provider_get_show_about (provider);
      gtk_widget_set_sensitive (GTK_WIDGET (object), active);
    }
  else
    {
      /* make all items insensitive, except for the add button */
      for (i = 0; i < G_N_ELEMENTS (button_names); i++)
        {
          object = gtk_builder_get_object (GTK_BUILDER (dialog), button_names[i]);
          bar_return_if_fail (GTK_IS_WIDGET (object));
          gtk_widget_set_sensitive (GTK_WIDGET (object), FALSE);
        }
    }
}



static void
bar_preferences_dialog_plug_deleted (GtkWidget *plug)
{
  g_signal_handlers_disconnect_by_func (G_OBJECT (plug),
      G_CALLBACK (bar_preferences_dialog_plug_deleted), NULL);

  g_object_unref (G_OBJECT (dialog_singleton));
}



static void
bar_preferences_dialog_show_internal (BarWindow     *active,
                                        GdkNativeWindow  socket_window)
{
  gint         bar_id = 0;
  GObject     *window, *combo;
  GdkScreen   *screen;
  GSList      *windows;
  GtkWidget   *plug;
  GObject     *plug_child;
  GtkWidget   *content_area;

  bar_return_if_fail (active == NULL || BAR_IS_WINDOW (active));

  /* check if not the entire application is locked */
  if (bar_dialogs_kiosk_warning ())
    return;

  if (dialog_singleton == NULL)
    {
      /* create new dialog singleton */
      dialog_singleton = g_object_new (BAR_TYPE_PREFERENCES_DIALOG, NULL);
      g_object_add_weak_pointer (G_OBJECT (dialog_singleton), (gpointer) &dialog_singleton);
    }

  if (active == NULL)
    {
      /* select first window */
      windows = bar_application_get_windows (dialog_singleton->application);
      if (windows != NULL)
        active = g_slist_nth_data (windows, 0);
    }

  /* select the active window in the dialog */
  combo = gtk_builder_get_object (GTK_BUILDER (dialog_singleton), "bar-combobox");
  bar_return_if_fail (GTK_IS_WIDGET (combo));
  bar_id = bar_window_get_id (active);
  if (!bar_preferences_dialog_bar_combobox_rebuild (dialog_singleton, bar_id))
    gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);

  window = gtk_builder_get_object (GTK_BUILDER (dialog_singleton), "dialog");
  bar_return_if_fail (GTK_IS_WIDGET (window));
  plug_child = gtk_builder_get_object (GTK_BUILDER (dialog_singleton), "plug-child");
  bar_return_if_fail (GTK_IS_WIDGET (plug_child));

  /* check if we need to remove the window from the plug */
  if (dialog_singleton->socket_plug != NULL)
    {
      bar_return_if_fail (GTK_IS_PLUG (dialog_singleton->socket_plug));

      /* move the vbox to the dialog */
      content_area = gtk_dialog_get_content_area (GTK_DIALOG (window));
      gtk_widget_reparent (GTK_WIDGET (plug_child), content_area);
      gtk_widget_show (GTK_WIDGET (plug_child));

      /* destroy the plug */
      plug = dialog_singleton->socket_plug;
      dialog_singleton->socket_plug = NULL;

      g_signal_handlers_disconnect_by_func (G_OBJECT (plug),
          G_CALLBACK (bar_preferences_dialog_plug_deleted), NULL);
      gtk_widget_destroy (plug);
    }

  if (socket_window == 0)
    {
      /* show the dialog on the same screen as the bar */
      if (G_LIKELY (active != NULL))
        screen = gtk_widget_get_screen (GTK_WIDGET (active));
      else
        screen = gdk_screen_get_default ();
      gtk_window_set_screen (GTK_WINDOW (window), screen);

      gtk_window_present (GTK_WINDOW (window));
      bar_application_take_dialog (dialog_singleton->application, GTK_WINDOW (window));
    }
  else
    {
      /* hide window */
      gtk_widget_hide (GTK_WIDGET (window));

      /* create a new plug */
      plug = gtk_plug_new (socket_window);
      g_signal_connect (G_OBJECT (plug), "delete-event",
          G_CALLBACK (bar_preferences_dialog_plug_deleted), NULL);
      dialog_singleton->socket_plug = plug;
      gtk_widget_show (plug);

      /* move the vbox in the plug */
      gtk_widget_reparent (GTK_WIDGET (plug_child), plug);
      gtk_widget_show (GTK_WIDGET (plug_child));
    }
}



void
bar_preferences_dialog_show (BarWindow *active)
{
  bar_return_if_fail (active == NULL || BAR_IS_WINDOW (active));
  bar_preferences_dialog_show_internal (active, 0);
}



void
bar_preferences_dialog_show_from_id (gint         bar_id,
                                       const gchar *socket_id)
{
  BarApplication *application;
  BarWindow      *window;
  GdkNativeWindow   socket_window = 0;

  /* x11 windows are ulong on 64 bit platforms
   * or uint32 on other platforms */
  if (socket_id != NULL)
    socket_window = (GdkNativeWindow) strtoul (socket_id, NULL, 0);

  application = bar_application_get ();
  window = bar_application_get_window (application, bar_id);
  bar_preferences_dialog_show_internal (window, socket_window);
  g_object_unref (G_OBJECT (application));
}



gboolean
bar_preferences_dialog_visible (void)
{
  return !!(dialog_singleton != NULL);
}
