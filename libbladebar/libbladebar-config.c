/*
 * Copyright (C) 2009-2010 Nick Schermer <nick@xfce.org>
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

#include <libbladebar/libbladebar-config.h>
#include <libbladebar/libbladebar-alias.h>



/**
 * SECTION: config
 * @title: Version Information
 * @short_description: Information about the bar version in use.
 * @include: libbladebar/libbladebar.h
 *
 * The bar library provides version information, which could be used
 * by plugins to handle new API.
 **/



/**
 * libbladebar_major_version:
 *
 * The major version number of the libbladebar library (e.g. in
 * version 4.8.0 this is 4).
 *
 * This variable is in the library, so represents the
 * libbladebar library you have linked against. Contrast with the
 * #LIBBLADEBAR_MAJOR_VERSION macro, which represents the major
 * version of the libbladebar headers you have included.
 *
 * Since: 4.8
 **/
const guint libbladebar_major_version = LIBBLADEBAR_MAJOR_VERSION;



/**
 * libbladebar_minor_version:
 *
 * The minor version number of the libbladebar library (e.g. in
 * version 4.8.0 this is 8).
 *
 * This variable is in the library, so represents the
 * libbladebar library you have linked against. Contrast with the
 * #LIBBLADEBAR_MINOR_VERSION macro, which represents the minor
 * version of the libbladebar headers you have included.
 *
 * Since: 4.8
 **/
const guint libbladebar_minor_version = LIBBLADEBAR_MINOR_VERSION;



/**
 * libbladebar_micro_version:
 *
 * The micro version number of the libbladebar library (e.g. in
 * version 4.8.0 this is 0).
 *
 * This variable is in the library, so represents the
 * libbladebar library you have linked against. Contrast with the
 * #LIBBLADEBAR_MICRO_VERSION macro, which represents the micro
 * version of the libbladebar headers you have included.
 *
 * Since: 4.8
 **/
const guint libbladebar_micro_version = LIBBLADEBAR_MICRO_VERSION;



/**
 * libbladebar_check_version:
 * @required_major: the required major version.
 * @required_minor: the required minor version.
 * @required_micro: the required micro version.
 *
 * Checks that the libbladebar library in use is compatible with
 * the given version. Generally you would pass in the constants
 * #LIBBLADEBAR_MAJOR_VERSION, #LIBBLADEBAR_MINOR_VERSION and
 * #LIBBLADEBAR_MICRO_VERSION as the three arguments to this
 * function; that produces a check that the library in use is
 * compatible with the version of libbladebar the extension was
 * compiled against.
 *
 * <example>
 * <title>Checking the runtime version of the Libbladebar library</title>
 * <programlisting>
 * const gchar *mismatch;
 * mismatch = libbladebar_check_version (LIBBLADEBAR_MAJOR_VERSION,
 *                                      LIBBLADEBAR_MINOR_VERSION,
 *                                      LIBBLADEBAR_MICRO_VERSION);
 * if (G_UNLIKELY (mismatch != NULL))
 *   g_error ("Version mismatch: %<!---->s", mismatch);
 * </programlisting>
 * </example>
 *
 * Returns: %NULL if the library is compatible with the given version,
 *          or a string describing the version mismatch. The returned
 *          string is owned by the library and must not be freed or
 *          modified by the caller.
 *
 * Since: 4.8
 **/
const gchar *
libbladebar_check_version (guint required_major,
                             guint required_minor,
                             guint required_micro)
{
  if (required_major > LIBBLADEBAR_MAJOR_VERSION)
    return "Blade Bar version too old (major mismatch)";
  if (required_major < LIBBLADEBAR_MAJOR_VERSION)
    return "Blade Bar version too new (major mismatch)";
  if (required_minor > LIBBLADEBAR_MINOR_VERSION)
    return "Blade Bar version too old (minor mismatch)";
  if (required_minor == LIBBLADEBAR_MINOR_VERSION
      && required_micro > LIBBLADEBAR_MICRO_VERSION)
    return "Blade Bar version too old (micro mismatch)";
  return NULL;
}



#define __LIBBLADEBAR_CONFIG_C__
#include <libbladebar/libbladebar-aliasdef.c>
