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

#ifndef __LIBBLADEBAR__
#define __LIBBLADEBAR__

#include <gtk/gtk.h>

#define LIBBLADEBAR_INSIDE_LIBBLADEBAR_H

#include <libbladebar/libbladebar-config.h>
#include <libbladebar/libbladebar-enums.h>
#include <libbladebar/libbladebar-enum-types.h>
#include <libbladebar/blade-bar-macros.h>
#include <libbladebar/blade-bar-macros-46.h>
#include <libbladebar/xfce-arrow-button.h>
#if !GTK_CHECK_VERSION (3, 0, 0)
#include <libbladebar/xfce-hvbox.h>
#endif
#include <libbladebar/blade-bar-convenience.h>
#include <libbladebar/blade-bar-plugin.h>
#include <libbladebar/blade-bar-image.h>

#undef LIBBLADEBAR_INSIDE_LIBBLADEBAR_H

#endif /* !__LIBBLADEBAR__ */
