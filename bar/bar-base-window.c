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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_MATH_H
#include <math.h>
#endif

#include <blxo/blxo.h>
#include <libbladebar/libbladebar.h>
#include <libbladebar/blade-bar-plugin-provider.h>
#include <common/bar-private.h>
#include <common/bar-debug.h>
#include <bar/bar-base-window.h>
#include <bar/bar-window.h>
#include <bar/bar-plugin-external.h>
#include <bar/bar-plugin-external-46.h>



static void     bar_base_window_get_property                (GObject              *object,
                                                               guint                 prop_id,
                                                               GValue               *value,
                                                               GParamSpec           *pspec);
static void     bar_base_window_set_property                (GObject              *object,
                                                               guint                 prop_id,
                                                               const GValue         *value,
                                                               GParamSpec           *pspec);
static void     bar_base_window_finalize                    (GObject              *object);
static void     bar_base_window_screen_changed              (GtkWidget            *widget,
                                                               GdkScreen            *previous_screen);
static gboolean bar_base_window_expose_event                (GtkWidget            *widget,
                                                               GdkEventExpose       *event);
static gboolean bar_base_window_enter_notify_event          (GtkWidget            *widget,
                                                               GdkEventCrossing     *event);
static gboolean bar_base_window_leave_notify_event          (GtkWidget            *widget,
                                                               GdkEventCrossing     *event);
static void     bar_base_window_composited_changed          (GtkWidget            *widget);
static gboolean bar_base_window_active_timeout              (gpointer              user_data);
static void     bar_base_window_active_timeout_destroyed    (gpointer              user_data);
static void     bar_base_window_set_plugin_data             (BarBaseWindow      *window,
                                                               GtkCallback           func);
static void     bar_base_window_set_plugin_background_alpha (GtkWidget            *widget,
                                                               gpointer              user_data);
static void     bar_base_window_set_plugin_background_color (GtkWidget            *widget,
                                                               gpointer              user_data);
static void     bar_base_window_set_plugin_background_image (GtkWidget            *widget,
                                                               gpointer              user_data);



enum
{
  PROP_0,
  PROP_ENTER_OPACITY,
  PROP_LEAVE_OPACITY,
  PROP_BACKGROUND_ALPHA,
  PROP_BORDERS,
  PROP_ACTIVE,
  PROP_COMPOSITED,
  PROP_BACKGROUND_STYLE,
  PROP_BACKGROUND_COLOR,
  PROP_BACKGROUND_IMAGE
};

struct _BarBaseWindowPrivate
{
  BarBorders     borders;

  /* background image cache */
  cairo_pattern_t *bg_image_cache;

  /* transparency settings */
  gdouble          enter_opacity;
  gdouble          leave_opacity;

  /* active window timeout id */
  guint            active_timeout_id;
};



G_DEFINE_TYPE (BarBaseWindow, bar_base_window, GTK_TYPE_WINDOW)



static void
bar_base_window_class_init (BarBaseWindowClass *klass)
{
  GObjectClass   *gobject_class;
  GtkWidgetClass *gtkwidget_class;

  /* add private data */
  g_type_class_add_private (klass, sizeof (BarBaseWindowPrivate));

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->get_property = bar_base_window_get_property;
  gobject_class->set_property = bar_base_window_set_property;
  gobject_class->finalize = bar_base_window_finalize;

  gtkwidget_class = GTK_WIDGET_CLASS (klass);
  gtkwidget_class->expose_event = bar_base_window_expose_event;
  gtkwidget_class->enter_notify_event = bar_base_window_enter_notify_event;
  gtkwidget_class->leave_notify_event = bar_base_window_leave_notify_event;
  gtkwidget_class->composited_changed = bar_base_window_composited_changed;
  gtkwidget_class->screen_changed = bar_base_window_screen_changed;

  g_object_class_install_property (gobject_class,
                                   PROP_ENTER_OPACITY,
                                   g_param_spec_uint ("enter-opacity",
                                                      NULL, NULL,
                                                      0, 100, 100,
                                                      BLXO_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_LEAVE_OPACITY,
                                   g_param_spec_uint ("leave-opacity",
                                                      NULL, NULL,
                                                      0, 100, 100,
                                                      BLXO_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_BACKGROUND_ALPHA,
                                   g_param_spec_uint ("background-alpha",
                                                      NULL, NULL,
                                                      0, 100, 100,
                                                      BLXO_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_BACKGROUND_STYLE,
                                   g_param_spec_uint ("background-style",
                                                      NULL, NULL,
                                                      BAR_BG_STYLE_NONE,
                                                      BAR_BG_STYLE_IMAGE,
                                                      BAR_BG_STYLE_NONE,
                                                      BLXO_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_BACKGROUND_COLOR,
                                   g_param_spec_boxed ("background-color",
                                                       NULL, NULL,
                                                       GDK_TYPE_COLOR,
                                                       BLXO_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_BACKGROUND_IMAGE,
                                   g_param_spec_string ("background-image",
                                                        NULL, NULL,
                                                        NULL,
                                                        BLXO_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_BORDERS,
                                   g_param_spec_uint ("borders",
                                                      NULL, NULL,
                                                      0, G_MAXUINT,
                                                      BAR_BORDER_NONE,
                                                      BLXO_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_ACTIVE,
                                   g_param_spec_boolean ("active",
                                                         NULL, NULL,
                                                         FALSE,
                                                         BLXO_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_COMPOSITED,
                                   g_param_spec_boolean ("composited",
                                                         NULL, NULL,
                                                         FALSE,
                                                         BLXO_PARAM_READABLE));
}



static void
bar_base_window_init (BarBaseWindow *window)
{
  window->priv = G_TYPE_INSTANCE_GET_PRIVATE (window, BAR_TYPE_BASE_WINDOW, BarBaseWindowPrivate);

  window->is_composited = FALSE;
  window->background_alpha = 1.00;
  window->background_style = BAR_BG_STYLE_NONE;
  window->background_image = NULL;
  window->background_color = NULL;

  window->priv->bg_image_cache = NULL;
  window->priv->enter_opacity = 1.00;
  window->priv->leave_opacity = 1.00;
  window->priv->borders = BAR_BORDER_NONE;
  window->priv->active_timeout_id = 0;

  /* some wm require stick to show the window on all workspaces, on xfwm4
   * the type-hint already takes care of that */
  gtk_window_stick (GTK_WINDOW (window));

  /* set colormap */
  bar_base_window_screen_changed (GTK_WIDGET (window), NULL);
}



static void
bar_base_window_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  BarBaseWindow        *window = BAR_BASE_WINDOW (object);
  BarBaseWindowPrivate *priv = window->priv;
  GdkColor               *color;

  switch (prop_id)
    {
    case PROP_ENTER_OPACITY:
      g_value_set_uint (value, rint (priv->enter_opacity * 100.00));
      break;

    case PROP_LEAVE_OPACITY:
      g_value_set_uint (value, rint (priv->leave_opacity * 100.00));
      break;

    case PROP_BACKGROUND_ALPHA:
      g_value_set_uint (value, rint (window->background_alpha * 100.00));
      break;

    case PROP_BACKGROUND_STYLE:
      g_value_set_uint (value, window->background_style);
      break;

    case PROP_BACKGROUND_COLOR:
      if (window->background_color != NULL)
        color = window->background_color;
      else
        color = &(GTK_WIDGET (window)->style->bg[GTK_STATE_NORMAL]);
      g_value_set_boxed (value, color);
      break;

    case PROP_BACKGROUND_IMAGE:
      g_value_set_string (value, window->background_image);
      break;

    case PROP_BORDERS:
      g_value_set_uint (value, priv->borders);
      break;

    case PROP_ACTIVE:
      g_value_set_boolean (value, !!(priv->active_timeout_id != 0));
      break;

    case PROP_COMPOSITED:
      g_value_set_boolean (value, window->is_composited);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
bar_base_window_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  BarBaseWindow        *window = BAR_BASE_WINDOW (object);
  BarBaseWindowPrivate *priv = window->priv;
  BarBgStyle            bg_style;

  switch (prop_id)
    {
    case PROP_ENTER_OPACITY:
      /* set the new enter opacity */
      priv->enter_opacity = g_value_get_uint (value) / 100.00;
      break;

    case PROP_LEAVE_OPACITY:
      /* set the new leave opacity */
      priv->leave_opacity = g_value_get_uint (value) / 100.00;
      if (window->is_composited)
        gtk_window_set_opacity (GTK_WINDOW (object), priv->leave_opacity);
      break;

    case PROP_BACKGROUND_ALPHA:
      /* set the new background alpha */
      window->background_alpha = g_value_get_uint (value) / 100.00;
      if (window->is_composited)
        gtk_widget_queue_draw (GTK_WIDGET (object));

      /* send the new background alpha to the external plugins */
      bar_base_window_set_plugin_data (window,
          bar_base_window_set_plugin_background_alpha);
      break;

    case PROP_BACKGROUND_STYLE:
      bg_style = g_value_get_uint (value);
      if (window->background_style != bg_style)
        {
          window->background_style = bg_style;

          if (priv->bg_image_cache != NULL)
            {
              /* destroy old image cache */
              cairo_pattern_destroy (priv->bg_image_cache);
              priv->bg_image_cache = NULL;
            }

          /* send information to external plugins */
          if (window->background_style == BAR_BG_STYLE_IMAGE
              && window->background_image != NULL)
            {
              bar_base_window_set_plugin_data (window,
                  bar_base_window_set_plugin_background_image);
            }
          else if (window->background_style == BAR_BG_STYLE_NONE
                   || (window->background_style == BAR_BG_STYLE_COLOR
                       && window->background_color != NULL))
            {
              bar_base_window_set_plugin_data (window,
                  bar_base_window_set_plugin_background_color);
            }

          /* resize to update border size too */
          gtk_widget_queue_resize (GTK_WIDGET (window));
        }
      break;

    case PROP_BACKGROUND_COLOR:
      if (window->background_color != NULL)
        gdk_color_free (window->background_color);
      window->background_color = g_value_dup_boxed (value);

      if (window->background_style == BAR_BG_STYLE_COLOR)
        {
          bar_base_window_set_plugin_data (window,
              bar_base_window_set_plugin_background_color);
          gtk_widget_queue_draw (GTK_WIDGET (window));
        }
      break;

    case PROP_BACKGROUND_IMAGE:
      /* store new filename */
      g_free (window->background_image);
      window->background_image = g_value_dup_string (value);

      /* drop old cache */
      if (priv->bg_image_cache != NULL)
        {
          cairo_pattern_destroy (priv->bg_image_cache);
          priv->bg_image_cache = NULL;
        }

      if (window->background_style == BAR_BG_STYLE_IMAGE)
        {
          bar_base_window_set_plugin_data (window,
              bar_base_window_set_plugin_background_image);
          gtk_widget_queue_draw (GTK_WIDGET (window));
        }
      break;

    case PROP_BORDERS:
      /* set new window borders and redraw the widget */
      bar_base_window_set_borders (BAR_BASE_WINDOW (object),
                                     g_value_get_uint (value));
      break;

    case PROP_ACTIVE:
      if (g_value_get_boolean (value))
        {
          if (priv->active_timeout_id == 0)
            {
              /* start timeout for the marching ants selection */
              priv->active_timeout_id = g_timeout_add_seconds_full (
                  G_PRIORITY_DEFAULT_IDLE, 1,
                  bar_base_window_active_timeout, object,
                  bar_base_window_active_timeout_destroyed);
            }
        }
      else if (priv->active_timeout_id != 0)
        {
          /* stop timeout */
          g_source_remove (priv->active_timeout_id);
        }

      /* queue a draw for first second */
      gtk_widget_queue_resize (GTK_WIDGET (object));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
bar_base_window_finalize (GObject *object)
{
  BarBaseWindow *window = BAR_BASE_WINDOW (object);

  /* stop running marching ants timeout */
  if (window->priv->active_timeout_id != 0)
    g_source_remove (window->priv->active_timeout_id);

  /* release bg image data */
  g_free (window->background_image);
  if (window->priv->bg_image_cache != NULL)
    cairo_pattern_destroy (window->priv->bg_image_cache);
  if (window->background_color != NULL)
    gdk_color_free (window->background_color);

  (*G_OBJECT_CLASS (bar_base_window_parent_class)->finalize) (object);
}



static void
bar_base_window_screen_changed (GtkWidget *widget, GdkScreen *previous_screen)
{
  BarBaseWindow *window = BAR_BASE_WINDOW (widget);
  GdkColormap     *colormap;
  GdkScreen       *screen;

  if (GTK_WIDGET_CLASS (bar_base_window_parent_class)->screen_changed != NULL)
    GTK_WIDGET_CLASS (bar_base_window_parent_class)->screen_changed (widget, previous_screen);

  /* set the rgba colormap if supported by the screen */
  screen = gtk_window_get_screen (GTK_WINDOW (window));
  colormap = gdk_screen_get_rgba_colormap (screen);
  if (colormap != NULL)
    {
      gtk_widget_set_colormap (widget, colormap);
      window->is_composited = gtk_widget_is_composited (widget);
    }

   bar_debug (BAR_DEBUG_BASE_WINDOW,
               "%p: rgba colormap=%p, compositing=%s", window,
               colormap, BAR_DEBUG_BOOL (window->is_composited));
}



static gboolean
bar_base_window_expose_event (GtkWidget      *widget,
                                GdkEventExpose *event)
{
  cairo_t                *cr;
  const GdkColor         *color;
  BarBaseWindow        *window = BAR_BASE_WINDOW (widget);
  BarBaseWindowPrivate *priv = window->priv;
  gdouble                 alpha;
  gdouble                 width = widget->allocation.width;
  gdouble                 height = widget->allocation.height;
  const gdouble           dashes[] = { 4.00, 4.00 };
  GTimeVal                timeval;
  GdkPixbuf              *pixbuf;
  GError                 *error = NULL;
  cairo_matrix_t          matrix = { 1, 0, 0, 1, 0, 0 }; /* identity matrix */

  if (!GTK_WIDGET_DRAWABLE (widget))
    return FALSE;

  /* create cairo context and set some default properties */
  cr = gdk_cairo_create (widget->window);
  bar_return_val_if_fail (cr != NULL, FALSE);
  cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_set_line_width (cr, 1.00);

  /* set rectangle to clip the drawing area */
  gdk_cairo_rectangle (cr, &event->area);

  /* get background alpha */
  alpha = window->is_composited ? window->background_alpha : 1.00;

  if (window->background_style == BAR_BG_STYLE_IMAGE)
    {
      /* clip the drawing area */
      cairo_clip (cr);

      if (G_LIKELY (priv->bg_image_cache != NULL))
        {
          if (G_UNLIKELY (priv->active_timeout_id != 0))
            cairo_matrix_init_translate (&matrix, -1, -1);

          cairo_set_source (cr, priv->bg_image_cache);
          cairo_pattern_set_matrix (priv->bg_image_cache, &matrix);
          cairo_paint (cr);
        }
      else if (window->background_image != NULL)
        {
          /* load the image in a pixbuf */
          pixbuf = gdk_pixbuf_new_from_file (window->background_image, &error);

          if (G_LIKELY (pixbuf != NULL))
            {
              gdk_cairo_set_source_pixbuf (cr, pixbuf, 0, 0);
              g_object_unref (G_OBJECT (pixbuf));

              priv->bg_image_cache = cairo_get_source (cr);
              cairo_pattern_reference (priv->bg_image_cache);
              cairo_pattern_set_extend (priv->bg_image_cache, CAIRO_EXTEND_REPEAT);
              cairo_paint (cr);
            }
          else
            {
              /* print error message */
              g_warning ("Background image disabled, \"%s\" could not be loaded: %s",
                         window->background_image, error != NULL ? error->message : "No error");
              g_error_free (error);

              /* disable background image mode */
              window->background_style = BAR_BG_STYLE_NONE;
            }
        }
    }
  else
    {
      /* get the background color */
      if (window->background_style == BAR_BG_STYLE_COLOR
          && window->background_color != NULL)
        color = window->background_color;
      else
        color = &(widget->style->bg[GTK_STATE_NORMAL]);

      /* only do something with the background when compositing is enabled */
      if (G_UNLIKELY (alpha < 1.00
          || window->background_style != BAR_BG_STYLE_NONE))
        {
          /* clip the drawing area, but preserve the rectangle */
          cairo_clip_preserve (cr);

          /* draw the background */
          bar_util_set_source_rgba (cr, color, alpha);
          cairo_fill (cr);
        }
      else
        {
          /* clip the drawing area */
          cairo_clip (cr);
        }
    }

  /* draw marching ants selection if the timeout is running */
  if (G_UNLIKELY (priv->active_timeout_id != 0))
    {
      /* red color, no alpha */
      cairo_set_source_rgb (cr, 1.00, 0.00, 0.00);

      /* set dash based on time (odd/even) */
      g_get_current_time (&timeval);
      cairo_set_dash (cr, dashes, G_N_ELEMENTS (dashes),
                      (timeval.tv_sec % 4) * 2);

      /* draw rectangle */
      cairo_rectangle (cr, 0.5, 0.5, width - 1, height - 1);
      cairo_stroke (cr);
    }
  else if (window->background_style == BAR_BG_STYLE_NONE)
    {
      if (BAR_HAS_FLAG (priv->borders, BAR_BORDER_BOTTOM | BAR_BORDER_RIGHT))
        {
          /* use dark color for buttom and right line */
          color = &(widget->style->dark[GTK_STATE_NORMAL]);
          bar_util_set_source_rgba (cr, color, alpha);

          if (BAR_HAS_FLAG (priv->borders, BAR_BORDER_BOTTOM))
            {
              cairo_move_to (cr, 0.50, height - 1);
              cairo_rel_line_to (cr, width, 0.50);
            }

          if (BAR_HAS_FLAG (priv->borders, BAR_BORDER_RIGHT))
            {
              cairo_move_to (cr, width - 1, 0.50);
              cairo_rel_line_to (cr, 0.50, height);
            }

          cairo_stroke (cr);
        }

      if (BAR_HAS_FLAG (priv->borders, BAR_BORDER_TOP | BAR_BORDER_LEFT))
        {
          /* use light color for top and left line */
          color = &(widget->style->light[GTK_STATE_NORMAL]);
          bar_util_set_source_rgba (cr, color, alpha);

          if (BAR_HAS_FLAG (priv->borders, BAR_BORDER_LEFT))
            {
              cairo_move_to (cr, 0.50, 0.50);
              cairo_rel_line_to (cr, 0.50, height);
            }

          if (BAR_HAS_FLAG (priv->borders, BAR_BORDER_TOP))
            {
              cairo_move_to (cr, 0.50, 0.50);
              cairo_rel_line_to (cr, width, 0.50);
            }

          cairo_stroke (cr);
        }
    }

  cairo_destroy (cr);

  return FALSE;
}



static gboolean
bar_base_window_enter_notify_event (GtkWidget        *widget,
                                      GdkEventCrossing *event)
{
  BarBaseWindowPrivate *priv = BAR_BASE_WINDOW (widget)->priv;

  /* switch to enter opacity when compositing is enabled
   * and the two values are different */
  if (event->detail != GDK_NOTIFY_INFERIOR
      && BAR_BASE_WINDOW (widget)->is_composited
      && priv->leave_opacity != priv->enter_opacity)
    gtk_window_set_opacity (GTK_WINDOW (widget), priv->enter_opacity);

  return FALSE;
}



static gboolean
bar_base_window_leave_notify_event (GtkWidget        *widget,
                                      GdkEventCrossing *event)
{
  BarBaseWindowPrivate *priv = BAR_BASE_WINDOW (widget)->priv;

  /* switch to leave opacity when compositing is enabled
   * and the two values are different */
  if (event->detail != GDK_NOTIFY_INFERIOR
      && BAR_BASE_WINDOW (widget)->is_composited
      && priv->leave_opacity != priv->enter_opacity)
    gtk_window_set_opacity (GTK_WINDOW (widget), priv->leave_opacity);

  return FALSE;
}



static void
bar_base_window_composited_changed (GtkWidget *widget)
{
  BarBaseWindow *window = BAR_BASE_WINDOW (widget);
  gboolean         was_composited = window->is_composited;
  GdkWindow       *gdkwindow;

  /* set new compositing state */
  window->is_composited = gtk_widget_is_composited (widget);
  if (window->is_composited == was_composited)
    return;

  if (window->is_composited)
    gtk_window_set_opacity (GTK_WINDOW (widget), window->priv->leave_opacity);

  bar_debug (BAR_DEBUG_BASE_WINDOW,
               "%p: compositing=%s", window,
               BAR_DEBUG_BOOL (window->is_composited));

  /* clear cairo image cache */
  if (window->priv->bg_image_cache != NULL)
    {
      cairo_pattern_destroy (window->priv->bg_image_cache);
      window->priv->bg_image_cache = NULL;
    }

  if (window->is_composited != was_composited)
    g_object_notify (G_OBJECT (widget), "composited");

  /* make sure the entire window is redrawn */
  gdkwindow = gtk_widget_get_window (widget);
  if (gdkwindow != NULL)
    gdk_window_invalidate_rect (gdkwindow, NULL, TRUE);

  /* HACK: invalid the geometry, so the wm notices it */
  gtk_window_move (GTK_WINDOW (window),
                   widget->allocation.x,
                   widget->allocation.y);
  gtk_widget_queue_resize (widget);
}



static gboolean
bar_base_window_active_timeout (gpointer user_data)
{
  /* redraw to update marching ants */
  GDK_THREADS_ENTER ();
  gtk_widget_queue_draw (GTK_WIDGET (user_data));
  GDK_THREADS_LEAVE ();

  return TRUE;
}



static void
bar_base_window_active_timeout_destroyed (gpointer user_data)
{
  BAR_BASE_WINDOW (user_data)->priv->active_timeout_id = 0;
}



static void
bar_base_window_set_plugin_data (BarBaseWindow *window,
                                   GtkCallback      func)
{
  GtkWidget *itembar;

  itembar = gtk_bin_get_child (GTK_BIN (window));
  if (G_LIKELY (itembar != NULL))
    gtk_container_foreach (GTK_CONTAINER (itembar), func, window);
}



static void
bar_base_window_set_plugin_background_alpha (GtkWidget *widget,
                                               gpointer   user_data)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (widget));
  bar_return_if_fail (BAR_IS_BASE_WINDOW (user_data));

  /* if the plugin is external, send the new alpha value to the wrapper/socket  */
  if (BAR_IS_PLUGIN_EXTERNAL (widget))
    bar_plugin_external_set_background_alpha (BAR_PLUGIN_EXTERNAL (widget),
        BAR_BASE_WINDOW (user_data)->background_alpha);
}



static void
bar_base_window_set_plugin_background_color (GtkWidget *widget,
                                               gpointer   user_data)
{
  BarBaseWindow *window = BAR_BASE_WINDOW (user_data);
  GdkColor        *color;

  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (widget));
  bar_return_if_fail (BAR_IS_BASE_WINDOW (user_data));

  /* send null if the style is not a bg color */
  color = window->background_style == BAR_BG_STYLE_COLOR ? window->background_color : NULL;

  if (BAR_IS_PLUGIN_EXTERNAL (widget))
    bar_plugin_external_set_background_color (BAR_PLUGIN_EXTERNAL (widget), color);
}



static void
bar_base_window_set_plugin_background_image (GtkWidget *widget,
                                               gpointer   user_data)
{
  bar_return_if_fail (BLADE_IS_BAR_PLUGIN_PROVIDER (widget));
  bar_return_if_fail (BAR_IS_BASE_WINDOW (user_data));

  if (BAR_IS_PLUGIN_EXTERNAL (widget))
    bar_plugin_external_set_background_image (BAR_PLUGIN_EXTERNAL (widget),
        BAR_BASE_WINDOW (user_data)->background_image);
}



void
bar_base_window_move_resize (BarBaseWindow *window,
                               gint             x,
                               gint             y,
                               gint             width,
                               gint             height)
{
  bar_return_if_fail (BAR_IS_BASE_WINDOW (window));

  if (width > 0 && height > 0)
    gtk_window_resize (GTK_WINDOW (window), width, height);

  gtk_window_move (GTK_WINDOW (window), x, y);
}



void
bar_base_window_set_borders (BarBaseWindow *window,
                               BarBorders     borders)
{
  BarBaseWindowPrivate *priv = window->priv;

  bar_return_if_fail (BAR_IS_BASE_WINDOW (window));

  if (priv->borders != borders)
    {
      priv->borders = borders;
      gtk_widget_queue_resize (GTK_WIDGET (window));
    }
}



BarBorders
bar_base_window_get_borders (BarBaseWindow *window)
{
  BarBaseWindowPrivate *priv = window->priv;

  bar_return_val_if_fail (BAR_IS_BASE_WINDOW (window), BAR_BORDER_NONE);

  /* show all borders for the marching ants */
  if (priv->active_timeout_id != 0)
    return BAR_BORDER_TOP | BAR_BORDER_BOTTOM
           | BAR_BORDER_LEFT | BAR_BORDER_RIGHT;
  else if (window->background_style != BAR_BG_STYLE_NONE)
    return BAR_BORDER_NONE;

  return priv->borders;
}



void
bar_util_set_source_rgba (cairo_t        *cr,
                            const GdkColor *color,
                            gdouble         alpha)
{
  bar_return_if_fail (alpha >= 0.00 && alpha <= 1.00);
  bar_return_if_fail (color != NULL);

  if (G_LIKELY (alpha == 1.00))
    cairo_set_source_rgb (cr, color->red / 65535.00,
                          color->green / 65535.00,
                          color->blue / 65535.00);
  else
    cairo_set_source_rgba (cr, color->red / 65535.00,
                           color->green / 65535.00,
                           color->blue / 65535.00, alpha);
}
