/*
 * Copyright (C) 2010-2011 Nick Schermer <nick@xfce.org>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <glib.h>
#include <common/bar-debug.h>
#include <common/bar-private.h>



static BarDebugFlag bar_debug_flags = 0;



/* additional debug levels */
static const GDebugKey bar_debug_keys[] =
{
  /* external plugin proxy modes */
  { "gdb", BAR_DEBUG_GDB },
  { "valgrind", BAR_DEBUG_VALGRIND },

  /* domains for debug messages in the code */
  { "application", BAR_DEBUG_APPLICATION },
  { "applicationsmenu", BAR_DEBUG_APPLICATIONSMENU },
  { "base-window", BAR_DEBUG_BASE_WINDOW },
  { "display-layout", BAR_DEBUG_DISPLAY_LAYOUT },
  { "external46", BAR_DEBUG_EXTERNAL46 },
  { "external", BAR_DEBUG_EXTERNAL },
  { "main", BAR_DEBUG_MAIN },
  { "module-factory", BAR_DEBUG_MODULE_FACTORY },
  { "module", BAR_DEBUG_MODULE },
  { "positioning", BAR_DEBUG_POSITIONING },
  { "struts", BAR_DEBUG_STRUTS },
  { "systray", BAR_DEBUG_SYSTRAY },
  { "tasklist", BAR_DEBUG_TASKLIST }
};



static BarDebugFlag
bar_debug_init (void)
{
  static volatile gsize  inited__volatile = 0;
  const gchar           *value;

  if (g_once_init_enter (&inited__volatile))
    {
      value = g_getenv ("BAR_DEBUG");
      if (value != NULL && *value != '\0')
        {
          bar_debug_flags = g_parse_debug_string (value, bar_debug_keys,
                                                    G_N_ELEMENTS (bar_debug_keys));

          /* always enable (unfiltered) debugging messages */
          BAR_SET_FLAG (bar_debug_flags, BAR_DEBUG_YES);

          /* unset gdb and valgrind in 'all' mode */
          if (g_ascii_strcasecmp (value, "all") == 0)
            BAR_UNSET_FLAG (bar_debug_flags, BAR_DEBUG_GDB | BAR_DEBUG_VALGRIND);
        }

      g_once_init_leave (&inited__volatile, 1);
    }

  return bar_debug_flags;
}



static void
bar_debug_print (BarDebugFlag  domain,
                   const gchar    *message,
                   va_list         args)
{
  gchar       *string;
  const gchar *domain_name = NULL;
  guint        i;

  /* lookup domain name */
  for (i = 0; i < G_N_ELEMENTS (bar_debug_keys); i++)
    {
      if (bar_debug_keys[i].value == domain)
        {
          domain_name = bar_debug_keys[i].key;
          break;
        }
    }

  bar_assert (domain_name != NULL);

  string = g_strdup_vprintf (message, args);
  g_printerr (PACKAGE_NAME "(%s): %s\n", domain_name, string);
  g_free (string);
}



gboolean
bar_debug_has_domain (BarDebugFlag domain)
{
  return BAR_HAS_FLAG (bar_debug_flags, domain);
}



void
bar_debug (BarDebugFlag  domain,
             const gchar    *message,
             ...)
{
  va_list args;

  bar_return_if_fail (domain > 0);
  bar_return_if_fail (message != NULL);

  /* leave when debug is disabled */
  if (bar_debug_init () == 0)
    return;

  va_start (args, message);
  bar_debug_print (domain, message, args);
  va_end (args);
}



void
bar_debug_filtered (BarDebugFlag  domain,
                      const gchar    *message,
                      ...)
{
  va_list args;

  bar_return_if_fail (domain > 0);
  bar_return_if_fail (message != NULL);

  /* leave when the filter does not match */
  if (!BAR_HAS_FLAG (bar_debug_init (), domain))
    return;

  va_start (args, message);
  bar_debug_print (domain, message, args);
  va_end (args);
}
