/*
 * Copyright (C) 2010 Nick Schermer <nick@xfce.org>
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

#ifndef __BAR_DEBUG_H__
#define __BAR_DEBUG_H__

#define BAR_DEBUG_BOOL(bool) ((bool) ? "true" : "false")

typedef enum
{
  BAR_DEBUG_YES              = 1 << 0, /* always enabled if BAR_DEBUG is not %NULL */

  /* external plugin proxy modes */
  BAR_DEBUG_GDB              = 1 << 1, /* run external plugins in gdb */
  BAR_DEBUG_VALGRIND         = 1 << 2, /* run external plugins in valgrind */

  /* filter domains */
  BAR_DEBUG_APPLICATION      = 1 << 3,
  BAR_DEBUG_APPLICATIONSMENU = 1 << 4,
  BAR_DEBUG_BASE_WINDOW      = 1 << 5,
  BAR_DEBUG_DISPLAY_LAYOUT   = 1 << 6,
  BAR_DEBUG_EXTERNAL         = 1 << 7,
  BAR_DEBUG_EXTERNAL46       = 1 << 8,
  BAR_DEBUG_MAIN             = 1 << 9,
  BAR_DEBUG_MODULE           = 1 << 10,
  BAR_DEBUG_MODULE_FACTORY   = 1 << 11,
  BAR_DEBUG_POSITIONING      = 1 << 12,
  BAR_DEBUG_STRUTS           = 1 << 13,
  BAR_DEBUG_SYSTRAY          = 1 << 14,
  BAR_DEBUG_TASKLIST         = 1 << 15
}
BarDebugFlag;

gboolean bar_debug_has_domain   (BarDebugFlag  domain);

void     bar_debug              (BarDebugFlag  domain,
                                   const gchar    *message,
                                   ...) G_GNUC_PRINTF (2, 3);

void     bar_debug_filtered     (BarDebugFlag  domain,
                                   const gchar    *message,
                                   ...) G_GNUC_PRINTF (2, 3);

#endif /* !__BAR_DEBUG_H__ */
