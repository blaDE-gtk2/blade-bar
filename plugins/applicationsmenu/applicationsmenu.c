/*
 * Copyright (C) 2010 Nick Schermer <nick@xfce.org>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <blxo/blxo.h>
#include <pojk/pojk.h>
#include <pojk-gtk/pojk-gtk.h>
#include <libbladeui/libbladeui.h>
#include <libbladeutil/libbladeutil.h>
#include <libbladebar/libbladebar.h>
#include <common/bar-blconf.h>
#include <common/bar-utils.h>
#include <common/bar-private.h>
#include <common/bar-debug.h>

#include "applicationsmenu.h"
#include "applicationsmenu-dialog_ui.h"


/* I18N: default tooltip of the application menu */
#define DEFAULT_TITLE     _("Applications")
#define DEFAULT_ICON_NAME "blade-bar-menu"
#define DEFAULT_ICON_SIZE (16)



struct _ApplicationsMenuPluginClass
{
  BladeBarPluginClass __parent__;
};

struct _ApplicationsMenuPlugin
{
  BladeBarPlugin __parent__;

  GtkWidget       *button;
  GtkWidget       *box;
  GtkWidget       *icon;
  GtkWidget       *label;
  GtkWidget       *menu;

  guint            is_constructed : 1;

  guint            show_button_title : 1;
  gchar           *button_title;
  gchar           *button_icon;
  gboolean         custom_menu;
  gchar           *custom_menu_file;

  /* temp item we store here when the
   * properties dialog is opened */
  GtkWidget       *dialog_icon;

  gulong           style_set_id;
  gulong           screen_changed_id;
};

enum
{
  PROP_0,
  PROP_SHOW_GENERIC_NAMES,
  PROP_SHOW_MENU_ICONS,
  PROP_SHOW_TOOLTIPS,
  PROP_SHOW_BUTTON_TITLE,
  PROP_BUTTON_TITLE,
  PROP_BUTTON_ICON,
  PROP_CUSTOM_MENU,
  PROP_CUSTOM_MENU_FILE
};



static void      applications_menu_plugin_get_property         (GObject                *object,
                                                                guint                   prop_id,
                                                                GValue                 *value,
                                                                GParamSpec             *pspec);
static void      applications_menu_plugin_set_property         (GObject                *object,
                                                                guint                   prop_id,
                                                                const GValue           *value,
                                                                GParamSpec             *pspec);
static void      applications_menu_plugin_construct            (BladeBarPlugin        *bar_plugin);
static void      applications_menu_plugin_free_data            (BladeBarPlugin        *bar_plugin);
static gboolean  applications_menu_plugin_size_changed         (BladeBarPlugin        *bar_plugin,
                                                                gint                    size);
static void      applications_menu_plugin_mode_changed         (BladeBarPlugin        *bar_plugin,
                                                                BladeBarPluginMode     mode);
static void      applications_menu_plugin_configure_plugin     (BladeBarPlugin        *bar_plugin);
static gboolean  applications_menu_plugin_remote_event         (BladeBarPlugin        *bar_plugin,
                                                                const gchar            *name,
                                                                const GValue           *value);
static gboolean  applications_menu_plugin_menu                 (GtkWidget              *button,
                                                                GdkEventButton         *event,
                                                                ApplicationsMenuPlugin *plugin);
static void      applications_menu_plugin_menu_deactivate      (GtkWidget              *menu,
                                                                GtkWidget              *button);
static void      applications_menu_plugin_set_pojk_menu      (ApplicationsMenuPlugin *plugin);
static void      applications_menu_button_theme_changed        (ApplicationsMenuPlugin *plugin);



/* define the plugin */
BLADE_BAR_DEFINE_PLUGIN (ApplicationsMenuPlugin, applications_menu_plugin)



static void
applications_menu_plugin_class_init (ApplicationsMenuPluginClass *klass)
{
  BladeBarPluginClass *plugin_class;
  GObjectClass         *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->get_property = applications_menu_plugin_get_property;
  gobject_class->set_property = applications_menu_plugin_set_property;

  plugin_class = BLADE_BAR_PLUGIN_CLASS (klass);
  plugin_class->construct = applications_menu_plugin_construct;
  plugin_class->free_data = applications_menu_plugin_free_data;
  plugin_class->size_changed = applications_menu_plugin_size_changed;
  plugin_class->mode_changed = applications_menu_plugin_mode_changed;
  plugin_class->configure_plugin = applications_menu_plugin_configure_plugin;
  plugin_class->remote_event = applications_menu_plugin_remote_event;

  g_object_class_install_property (gobject_class,
                                   PROP_SHOW_GENERIC_NAMES,
                                   g_param_spec_boolean ("show-generic-names",
                                                         NULL, NULL,
                                                         FALSE,
                                                         BLXO_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_SHOW_MENU_ICONS,
                                   g_param_spec_boolean ("show-menu-icons",
                                                         NULL, NULL,
                                                         TRUE,
                                                         BLXO_PARAM_READWRITE));
  g_object_class_install_property (gobject_class,
                                   PROP_SHOW_TOOLTIPS,
                                   g_param_spec_boolean ("show-tooltips",
                                                         NULL, NULL,
                                                         FALSE,
                                                         BLXO_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_SHOW_BUTTON_TITLE,
                                   g_param_spec_boolean ("show-button-title",
                                                         NULL, NULL,
                                                         TRUE,
                                                         BLXO_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_BUTTON_TITLE,
                                   g_param_spec_string ("button-title",
                                                        NULL, NULL,
                                                        DEFAULT_TITLE,
                                                        BLXO_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_BUTTON_ICON,
                                   g_param_spec_string ("button-icon",
                                                        NULL, NULL,
                                                        DEFAULT_ICON_NAME,
                                                        BLXO_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_CUSTOM_MENU,
                                   g_param_spec_boolean ("custom-menu",
                                                         NULL, NULL,
                                                         FALSE,
                                                         BLXO_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_CUSTOM_MENU_FILE,
                                   g_param_spec_string ("custom-menu-file",
                                                        NULL, NULL,
                                                        NULL,
                                                        BLXO_PARAM_READWRITE));
}



static void
applications_menu_plugin_init (ApplicationsMenuPlugin *plugin)
{
  /* init pojk environment */
  pojk_set_environment_xdg (POJK_ENVIRONMENT_XFCE);

  plugin->button = blade_bar_create_toggle_button ();
  blade_bar_plugin_add_action_widget (BLADE_BAR_PLUGIN (plugin), plugin->button);
  gtk_container_add (GTK_CONTAINER (plugin), plugin->button);
  gtk_widget_set_name (plugin->button, "applicationmenu-button");
  gtk_button_set_relief (GTK_BUTTON (plugin->button), GTK_RELIEF_NONE);
  gtk_widget_set_tooltip_text (plugin->button, DEFAULT_TITLE);
  g_signal_connect (G_OBJECT (plugin->button), "button-press-event",
      G_CALLBACK (applications_menu_plugin_menu), plugin);

  plugin->box = xfce_hvbox_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 2);
  gtk_container_set_border_width (GTK_CONTAINER (plugin->box), 0);
  gtk_container_add (GTK_CONTAINER (plugin->button), plugin->box);
  gtk_widget_show (plugin->box);

  plugin->icon = gtk_image_new ();
  gtk_box_pack_start (GTK_BOX (plugin->box), plugin->icon, FALSE, FALSE, 0);
  gtk_widget_show (plugin->icon);

  plugin->label = gtk_label_new (DEFAULT_TITLE);
  gtk_box_pack_start (GTK_BOX (plugin->box), plugin->label, FALSE, FALSE, 0);
  plugin->show_button_title = TRUE;
  gtk_widget_show (plugin->label);

  /* prepare the menu */
  plugin->menu = pojk_gtk_menu_new (NULL);
  g_signal_connect (G_OBJECT (plugin->menu), "selection-done",
      G_CALLBACK (applications_menu_plugin_menu_deactivate), plugin->button);

  plugin->style_set_id = g_signal_connect_swapped (G_OBJECT (plugin->button), "style-set",
                                                   G_CALLBACK (applications_menu_button_theme_changed), plugin);
  plugin->screen_changed_id = g_signal_connect_swapped (G_OBJECT (plugin->button), "screen-changed",
                                                        G_CALLBACK (applications_menu_button_theme_changed), plugin);
}



static void
applications_menu_plugin_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  ApplicationsMenuPlugin *plugin = XFCE_APPLICATIONS_MENU_PLUGIN (object);

  switch (prop_id)
    {
    case PROP_SHOW_GENERIC_NAMES:
      g_value_set_boolean (value,
          pojk_gtk_menu_get_show_generic_names (POJK_GTK_MENU (plugin->menu)));
      break;

    case PROP_SHOW_MENU_ICONS:
      g_value_set_boolean (value,
          pojk_gtk_menu_get_show_menu_icons (POJK_GTK_MENU (plugin->menu)));
      break;

    case PROP_SHOW_TOOLTIPS:
      g_value_set_boolean (value,
          pojk_gtk_menu_get_show_tooltips (POJK_GTK_MENU (plugin->menu)));
      break;

    case PROP_SHOW_BUTTON_TITLE:
      g_value_set_boolean (value, plugin->show_button_title);
      break;

    case PROP_BUTTON_TITLE:
      g_value_set_string (value, plugin->button_title == NULL ?
          DEFAULT_TITLE : plugin->button_title);
      break;

    case PROP_BUTTON_ICON:
      g_value_set_string (value, blxo_str_is_empty (plugin->button_icon) ?
          DEFAULT_ICON_NAME : plugin->button_icon);
      break;

    case PROP_CUSTOM_MENU:
      g_value_set_boolean (value, plugin->custom_menu);
      break;

    case PROP_CUSTOM_MENU_FILE:
      g_value_set_string (value, plugin->custom_menu_file);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
applications_menu_plugin_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  ApplicationsMenuPlugin *plugin = XFCE_APPLICATIONS_MENU_PLUGIN (object);
  gboolean                force_a_resize = FALSE;

  switch (prop_id)
    {
    case PROP_SHOW_GENERIC_NAMES:
      pojk_gtk_menu_set_show_generic_names (POJK_GTK_MENU (plugin->menu),
                                              g_value_get_boolean (value));
      break;

    case PROP_SHOW_MENU_ICONS:
      pojk_gtk_menu_set_show_menu_icons (POJK_GTK_MENU (plugin->menu),
                                           g_value_get_boolean (value));
       break;

    case PROP_SHOW_TOOLTIPS:
      pojk_gtk_menu_set_show_tooltips (POJK_GTK_MENU (plugin->menu),
                                         g_value_get_boolean (value));
      break;

    case PROP_SHOW_BUTTON_TITLE:
      plugin->show_button_title = g_value_get_boolean (value);
      if (plugin->show_button_title)
        gtk_widget_show (plugin->label);
      else
        gtk_widget_hide (plugin->label);
      applications_menu_plugin_size_changed (BLADE_BAR_PLUGIN (plugin),
          blade_bar_plugin_get_size (BLADE_BAR_PLUGIN (plugin)));
      return;

    case PROP_BUTTON_TITLE:
      g_free (plugin->button_title);
      plugin->button_title = g_value_dup_string (value);
      gtk_label_set_text (GTK_LABEL (plugin->label),
          plugin->button_title != NULL ? plugin->button_title : "");
      gtk_widget_set_tooltip_text (plugin->button,
          blxo_str_is_empty (plugin->button_title) ? NULL : plugin->button_title);

      /* check if the label still fits */
      if (blade_bar_plugin_get_mode (BLADE_BAR_PLUGIN (plugin)) == BLADE_BAR_PLUGIN_MODE_DESKBAR
          && plugin->show_button_title)
        {
          force_a_resize = TRUE;
        }
      break;

    case PROP_BUTTON_ICON:
      g_free (plugin->button_icon);
      plugin->button_icon = g_value_dup_string (value);

      force_a_resize = TRUE;
      break;

    case PROP_CUSTOM_MENU:
      plugin->custom_menu = g_value_get_boolean (value);

      if (plugin->is_constructed)
        applications_menu_plugin_set_pojk_menu (plugin);
      break;

    case PROP_CUSTOM_MENU_FILE:
      g_free (plugin->custom_menu_file);
      plugin->custom_menu_file = g_value_dup_string (value);

      if (plugin->is_constructed)
        applications_menu_plugin_set_pojk_menu (plugin);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }

  if (force_a_resize)
    {
      applications_menu_plugin_size_changed (BLADE_BAR_PLUGIN (plugin),
          blade_bar_plugin_get_size (BLADE_BAR_PLUGIN (plugin)));
    }
}



static void
applications_menu_plugin_construct (BladeBarPlugin *bar_plugin)
{
  ApplicationsMenuPlugin *plugin = XFCE_APPLICATIONS_MENU_PLUGIN (bar_plugin);
  const BarProperty  properties[] =
  {
    { "show-generic-names", G_TYPE_BOOLEAN },
    { "show-menu-icons", G_TYPE_BOOLEAN },
    { "show-button-title", G_TYPE_BOOLEAN },
    { "show-tooltips", G_TYPE_BOOLEAN },
    { "button-title", G_TYPE_STRING },
    { "button-icon", G_TYPE_STRING },
    { "custom-menu", G_TYPE_BOOLEAN },
    { "custom-menu-file", G_TYPE_STRING },
    { NULL }
  };

  blade_bar_plugin_menu_show_configure (BLADE_BAR_PLUGIN (plugin));

  /* bind all properties */
  bar_properties_bind (NULL, G_OBJECT (plugin),
                         blade_bar_plugin_get_property_base (bar_plugin),
                         properties, FALSE);

  /* make sure the menu is set */
  applications_menu_plugin_set_pojk_menu (plugin);

  gtk_widget_show (plugin->button);

  applications_menu_plugin_size_changed (bar_plugin,
      blade_bar_plugin_get_size (bar_plugin));
  plugin->is_constructed = TRUE;
}



static void
applications_menu_plugin_free_data (BladeBarPlugin *bar_plugin)
{
  ApplicationsMenuPlugin *plugin = XFCE_APPLICATIONS_MENU_PLUGIN (bar_plugin);

  if (plugin->menu != NULL)
    gtk_widget_destroy (plugin->menu);

  if (plugin->style_set_id != 0)
    {
      g_signal_handler_disconnect (plugin->button, plugin->style_set_id);
      plugin->style_set_id = 0;
    }

  if (plugin->screen_changed_id != 0)
    {
      g_signal_handler_disconnect (plugin->button, plugin->screen_changed_id);
      plugin->screen_changed_id = 0;
    }

  g_free (plugin->button_title);
  g_free (plugin->button_icon);
  g_free (plugin->custom_menu_file);
}



static gboolean
applications_menu_plugin_size_changed (BladeBarPlugin *bar_plugin,
                                       gint             size)
{
  ApplicationsMenuPlugin *plugin = XFCE_APPLICATIONS_MENU_PLUGIN (bar_plugin);
  gint                    row_size;
  GtkStyle               *style;
  BladeBarPluginMode     mode;
  GtkRequisition          label_size;
  GtkOrientation          orientation;
  gint                    border_thickness;
  GdkPixbuf              *icon;
  gint                    icon_width_max, icon_height_max;
  gint                    icon_width = 0;
  GdkScreen              *screen;
  GtkIconTheme           *icon_theme = NULL;
  gchar                  *icon_name;

  gtk_box_set_child_packing (GTK_BOX (plugin->box), plugin->icon,
                             !plugin->show_button_title,
                             !plugin->show_button_title,
                             0, GTK_PACK_START);

  mode = blade_bar_plugin_get_mode (bar_plugin);

  if (mode == BLADE_BAR_PLUGIN_MODE_HORIZONTAL)
    orientation = GTK_ORIENTATION_HORIZONTAL;
  else
    orientation = GTK_ORIENTATION_VERTICAL;

  row_size = size / blade_bar_plugin_get_nrows (bar_plugin);
  style = gtk_widget_get_style (plugin->button);
  border_thickness = 2 * MAX (style->xthickness, style->ythickness) + 2;

  /* arbitrary limit on non-square icon width in horizontal bar */
  icon_width_max = (mode == BLADE_BAR_PLUGIN_MODE_HORIZONTAL) ?
    6 * row_size - border_thickness :
    size - border_thickness;
  icon_height_max = row_size - border_thickness;

  screen = gtk_widget_get_screen (GTK_WIDGET (plugin));
  if (G_LIKELY (screen != NULL))
    icon_theme = gtk_icon_theme_get_for_screen (screen);

  icon_name = blxo_str_is_empty (plugin->button_icon) ?
    DEFAULT_ICON_NAME : plugin->button_icon;

  icon = blade_bar_pixbuf_from_source_at_size (icon_name,
                                                icon_theme,
                                                icon_width_max,
                                                icon_height_max);

  if (G_LIKELY (icon != NULL))
    {
      gtk_image_set_from_pixbuf (GTK_IMAGE (plugin->icon), icon);
      icon_width = gdk_pixbuf_get_width (icon);
      g_object_unref (G_OBJECT (icon));
    }

  if (plugin->show_button_title &&
      mode == BLADE_BAR_PLUGIN_MODE_DESKBAR)
    {
      /* check if the label fits next to the icon */
      gtk_widget_size_request (GTK_WIDGET (plugin->label), &label_size);
      if (label_size.width <= size - border_thickness - icon_width)
        orientation = GTK_ORIENTATION_HORIZONTAL;
    }

  gtk_orientable_set_orientation (GTK_ORIENTABLE (plugin->box), orientation);

  return TRUE;
}



static void
applications_menu_plugin_mode_changed (BladeBarPlugin     *bar_plugin,
                                       BladeBarPluginMode  mode)
{
  ApplicationsMenuPlugin *plugin = XFCE_APPLICATIONS_MENU_PLUGIN (bar_plugin);
  gint                    angle;

  angle = (mode == BLADE_BAR_PLUGIN_MODE_VERTICAL) ? 270 : 0;
  gtk_label_set_angle (GTK_LABEL (plugin->label), angle);

  applications_menu_plugin_size_changed (bar_plugin,
      blade_bar_plugin_get_size (bar_plugin));
}



static void
applications_menu_plugin_configure_plugin_file_set (GtkFileChooserButton   *button,
                                                    ApplicationsMenuPlugin *plugin)
{
  gchar *filename;

  bar_return_if_fail (GTK_IS_FILE_CHOOSER_BUTTON (button));
  bar_return_if_fail (XFCE_IS_APPLICATIONS_MENU_PLUGIN (plugin));

  filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (button));
  g_object_set (G_OBJECT (plugin), "custom-menu-file", filename, NULL);
  g_free (filename);
}



static void
applications_menu_plugin_configure_plugin_icon_chooser (GtkWidget              *button,
                                                        ApplicationsMenuPlugin *plugin)
{
  GtkWidget *chooser;
  gchar     *icon;

  bar_return_if_fail (XFCE_IS_APPLICATIONS_MENU_PLUGIN (plugin));
  bar_return_if_fail (BLADE_IS_BAR_IMAGE (plugin->dialog_icon));

  chooser = blxo_icon_chooser_dialog_new (_("Select An Icon"),
                                         GTK_WINDOW (gtk_widget_get_toplevel (button)),
                                         GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                         NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (chooser), GTK_RESPONSE_ACCEPT);
  gtk_dialog_set_alternative_button_order (GTK_DIALOG (chooser),
                                           GTK_RESPONSE_ACCEPT,
                                           GTK_RESPONSE_CANCEL, -1);

  blxo_icon_chooser_dialog_set_icon (BLXO_ICON_CHOOSER_DIALOG (chooser),
      blxo_str_is_empty (plugin->button_icon) ? DEFAULT_ICON_NAME : plugin->button_icon);

  if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
    {
      icon = blxo_icon_chooser_dialog_get_icon (BLXO_ICON_CHOOSER_DIALOG (chooser));
      g_object_set (G_OBJECT (plugin), "button-icon", icon, NULL);
      blade_bar_image_set_from_source (BLADE_BAR_IMAGE (plugin->dialog_icon),
                                        blxo_str_is_empty (plugin->button_icon) ?
                                        DEFAULT_ICON_NAME : plugin->button_icon);
      g_free (icon);
    }

  gtk_widget_destroy (chooser);
}



static void
applications_menu_plugin_configure_plugin_edit (GtkWidget              *button,
                                                ApplicationsMenuPlugin *plugin)
{
  GError      *error = NULL;
  const gchar  command[] = "alacarte";

  bar_return_if_fail (XFCE_IS_APPLICATIONS_MENU_PLUGIN (plugin));
  bar_return_if_fail (GTK_IS_WIDGET (button));

  if (!xfce_spawn_command_line_on_screen (gtk_widget_get_screen (button), command,
                                          FALSE, FALSE, &error))
    {
      xfce_dialog_show_error (NULL, error, _("Failed to execute command \"%s\"."), command);
      g_error_free (error);
    }
}



static void
applications_menu_plugin_configure_plugin (BladeBarPlugin *bar_plugin)
{
  ApplicationsMenuPlugin *plugin = XFCE_APPLICATIONS_MENU_PLUGIN (bar_plugin);
  GtkBuilder             *builder;
  GObject                *dialog, *object, *object2;
  guint                   i;
  gchar                  *path;
  const gchar            *check_names[] = { "show-generic-names", "show-menu-icons",
                                            "show-tooltips", "show-button-title" };

  /* setup the dialog */
  BAR_UTILS_LINK_4UI
  builder = bar_utils_builder_new (bar_plugin, applicationsmenu_dialog_ui,
                                     applicationsmenu_dialog_ui_length, &dialog);
  if (G_UNLIKELY (builder == NULL))
    return;

  for (i = 0; i < G_N_ELEMENTS (check_names); ++i)
    {
      object = gtk_builder_get_object (builder, check_names[i]);
      bar_return_if_fail (GTK_IS_CHECK_BUTTON (object));
      blxo_mutual_binding_new (G_OBJECT (plugin), check_names[i],
                              G_OBJECT (object), "active");
    }

  object = gtk_builder_get_object (builder, "button-title");
  bar_return_if_fail (GTK_IS_ENTRY (object));
  blxo_mutual_binding_new (G_OBJECT (plugin), "button-title",
                          G_OBJECT (object), "text");

  object = gtk_builder_get_object (builder, "icon-button");
  bar_return_if_fail (GTK_IS_BUTTON (object));
  g_signal_connect (G_OBJECT (object), "clicked",
     G_CALLBACK (applications_menu_plugin_configure_plugin_icon_chooser), plugin);

  plugin->dialog_icon = blade_bar_image_new_from_source (
      blxo_str_is_empty (plugin->button_icon) ? DEFAULT_ICON_NAME : plugin->button_icon);
  blade_bar_image_set_size (BLADE_BAR_IMAGE (plugin->dialog_icon), 48);
  gtk_container_add (GTK_CONTAINER (object), plugin->dialog_icon);
  g_object_add_weak_pointer (G_OBJECT (plugin->dialog_icon), (gpointer) &plugin->dialog_icon);
  gtk_widget_show (plugin->dialog_icon);

  /* whether we show the edit menu button */
  object = gtk_builder_get_object (builder, "edit-menu-button");
  bar_return_if_fail (GTK_IS_BUTTON (object));
  path = g_find_program_in_path ("alacarte");
  if (path != NULL)
    {
      object2 = gtk_builder_get_object (builder, "use-default-menu");
      bar_return_if_fail (GTK_IS_RADIO_BUTTON (object2));
      blxo_binding_new (G_OBJECT (object2), "active", G_OBJECT (object), "sensitive");
      g_signal_connect (G_OBJECT (object), "clicked",
          G_CALLBACK (applications_menu_plugin_configure_plugin_edit), plugin);
    }
  else
    {
      gtk_widget_hide (GTK_WIDGET (object));
    }
  g_free (path);

  object = gtk_builder_get_object (builder, "use-custom-menu");
  bar_return_if_fail (GTK_IS_RADIO_BUTTON (object));
  blxo_mutual_binding_new (G_OBJECT (plugin), "custom-menu",
                          G_OBJECT (object), "active");

  /* sensitivity of custom file selector */
  object2 = gtk_builder_get_object (builder, "custom-box");
  bar_return_if_fail (GTK_IS_WIDGET (object2));
  blxo_binding_new (G_OBJECT (object), "active", G_OBJECT (object2), "sensitive");

  object = gtk_builder_get_object (builder, "custom-file");
  bar_return_if_fail (GTK_IS_FILE_CHOOSER_BUTTON (object));
  if (!blxo_str_is_empty (plugin->custom_menu_file))
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (object), plugin->custom_menu_file);
  g_signal_connect (G_OBJECT (object), "file-set",
     G_CALLBACK (applications_menu_plugin_configure_plugin_file_set), plugin);

  gtk_widget_show (GTK_WIDGET (dialog));
}



static gboolean
applications_menu_plugin_remote_event (BladeBarPlugin *bar_plugin,
                                       const gchar     *name,
                                       const GValue    *value)
{
  ApplicationsMenuPlugin *plugin = XFCE_APPLICATIONS_MENU_PLUGIN (bar_plugin);

  bar_return_val_if_fail (value == NULL || G_IS_VALUE (value), FALSE);

  if (strcmp (name, "popup") == 0
      && GTK_WIDGET_VISIBLE (bar_plugin)
      && !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (plugin->button))
      && bar_utils_grab_available ())
    {
      if (value != NULL
          && G_VALUE_HOLDS_BOOLEAN (value)
          && g_value_get_boolean (value))
        {
          /* show menu under cursor */
          applications_menu_plugin_menu (NULL, NULL, plugin);
        }
      else
        {
          /* show the menu at the button */
          applications_menu_plugin_menu (plugin->button, NULL, plugin);
        }

      /* don't popup another menu */
      return TRUE;
    }

  return FALSE;
}



static void
applications_menu_plugin_menu_deactivate (GtkWidget *menu,
                                          GtkWidget *button)
{
  bar_return_if_fail (button == NULL || GTK_IS_TOGGLE_BUTTON (button));
  bar_return_if_fail (GTK_IS_MENU (menu));

  /* button is NULL when we popup the menu under the cursor position */
  if (button != NULL)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);

  gtk_menu_popdown (GTK_MENU (menu));
}



static void
applications_menu_plugin_set_pojk_menu (ApplicationsMenuPlugin *plugin)
{
  PojkMenu *menu = NULL;
  gchar      *filename;
  GFile      *file;

  bar_return_if_fail (XFCE_IS_APPLICATIONS_MENU_PLUGIN (plugin));
  bar_return_if_fail (POJK_GTK_IS_MENU (plugin->menu));

  /* load the custom menu if set */
  if (plugin->custom_menu
      && plugin->custom_menu_file != NULL)
    menu = pojk_menu_new_for_path (plugin->custom_menu_file);

  /* use the applications menu, this also respects the
   * XDG_MENU_PREFIX environment variable */
  if (G_LIKELY (menu == NULL))
    menu = pojk_menu_new_applications ();

  /* set the menu */
  pojk_gtk_menu_set_menu (POJK_GTK_MENU (plugin->menu), menu);

  /* debugging information */
  if (0)
    {
  file = pojk_menu_get_file (menu);
  filename = g_file_get_parse_name (file);
  g_object_unref (G_OBJECT (file));

  bar_debug (BAR_DEBUG_APPLICATIONSMENU,
               "menu from \"%s\"", filename);
  g_free (filename);
    }

  g_object_unref (G_OBJECT (menu));
}



static gboolean
applications_menu_plugin_menu (GtkWidget              *button,
                               GdkEventButton         *event,
                               ApplicationsMenuPlugin *plugin)
{
  bar_return_val_if_fail (XFCE_IS_APPLICATIONS_MENU_PLUGIN (plugin), FALSE);
  bar_return_val_if_fail (button == NULL || plugin->button == button, FALSE);

  if (event != NULL /* remove event */
      && !(event->button == 1
           && event->type == GDK_BUTTON_PRESS
           && !BAR_HAS_FLAG (event->state, GDK_CONTROL_MASK)))
    return FALSE;

  if (button != NULL)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);

  /* show the menu */
  gtk_menu_popup (GTK_MENU (plugin->menu), NULL, NULL,
                  button != NULL ? blade_bar_plugin_position_menu : NULL,
                  plugin, 1,
                  event != NULL ? event->time : gtk_get_current_event_time ());

  return TRUE;
}



static void
applications_menu_button_theme_changed (ApplicationsMenuPlugin *plugin)
{
  BladeBarPlugin *bar_plugin = BLADE_BAR_PLUGIN (plugin);

  applications_menu_plugin_size_changed (bar_plugin,
      blade_bar_plugin_get_size (bar_plugin));
}

