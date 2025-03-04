/*
 * Copyright (C) 2008-2010 Nick Schermer <nick@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __BAR_PRIVATE_H__
#define __BAR_PRIVATE_H__

/* support macros for debugging (improved macro for better position indication) */
/*#ifndef NDEBUG*/
#define bar_assert(expr)                 g_assert (expr)
#define bar_assert_not_reached()         g_assert_not_reached ()
#define bar_return_if_fail(expr)         G_STMT_START { \
  if (G_UNLIKELY (!(expr))) \
    { \
      g_log (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, \
             "%s (%s): expression '%s' failed.", G_STRLOC, G_STRFUNC, \
             #expr); \
      return; \
    }; }G_STMT_END
#define bar_return_val_if_fail(expr,val) G_STMT_START { \
  if (G_UNLIKELY (!(expr))) \
    { \
      g_log (G_LOG_DOMAIN, G_LOG_LEVEL_CRITICAL, \
             "%s (%s): expression '%s' failed.", G_STRLOC, G_STRFUNC, \
             #expr); \
      return (val); \
    }; }G_STMT_END
/*#else
#define bar_assert(expr)                 G_STMT_START{ (void)0; }G_STMT_END
#define bar_assert_not_reached()         G_STMT_START{ (void)0; }G_STMT_END
#define bar_return_if_fail(expr)         G_STMT_START{ (void)0; }G_STMT_END
#define bar_return_val_if_fail(expr,val) G_STMT_START{ (void)0; }G_STMT_END
#endif*/

/* handling flags */
#define BAR_SET_FLAG(flags,flag) G_STMT_START{ ((flags) |= (flag)); }G_STMT_END
#define BAR_UNSET_FLAG(flags,flag) G_STMT_START{ ((flags) &= ~(flag)); }G_STMT_END
#define BAR_HAS_FLAG(flags,flag) (((flags) & (flag)) != 0)

/* relative path to the plugin directory */
#define BAR_PLUGIN_RELATIVE_PATH "xfce4" G_DIR_SEPARATOR_S "bar"

/* relative plugin's rc filename (printf format) */
#define BAR_PLUGIN_RC_RELATIVE_PATH BAR_PLUGIN_RELATIVE_PATH G_DIR_SEPARATOR_S "%s-%d.rc"

/* blconf property base (printf format) */
#define BAR_PLUGIN_PROPERTY_BASE "/plugins/plugin-%d"

/* minimum time in seconds between automatic restarts of bar plugins
 * without asking the user what to do */
#define BAR_PLUGIN_AUTO_RESTART (60)

/* integer swap functions */
#define SWAP_INTEGER(a,b) G_STMT_START { gint swp = a; a = b; b = swp; } G_STMT_END
#define TRANSPOSE_AREA(area) G_STMT_START { SWAP_INTEGER (area.width, area.height); \
                                            SWAP_INTEGER (area.x, area.y); } G_STMT_END

/* quick GList and GSList counting without traversing */
#define LIST_HAS_ONE_ENTRY(l)           ((l) != NULL && (l)->next == NULL)
#define LIST_HAS_ONE_OR_NO_ENTRIES(l)   ((l) == NULL || (l)->next == NULL)
#define LIST_HAS_TWO_OR_MORE_ENTRIES(l) ((l) != NULL && (l)->next != NULL)

/* make this easier to read */
#define BAR_GDKCOLOR_TO_DOUBLE(gdk_color) gdk_color->red / 65535.00, \
                                            gdk_color->green / 65535.00, \
                                            gdk_color->blue / 65535.00

#endif /* !__BAR_PRIVATE_H__ */
