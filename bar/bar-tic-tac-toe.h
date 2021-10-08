/*
 * Copyright (C) 2010 Nick Schermer <nick@xfce.org>
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

#ifndef __BAR_TIC_TAC_TOE_H__
#define __BAR_TIC_TAC_TOE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _BarTicTacToeClass BarTicTacToeClass;
typedef struct _BarTicTacToe      BarTicTacToe;

#define BAR_TYPE_TIC_TAC_TOE            (bar_tic_tac_toe_get_type ())
#define BAR_TIC_TAC_TOE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BAR_TYPE_TIC_TAC_TOE, BarTicTacToe))
#define BAR_TIC_TAC_TOE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BAR_TYPE_TIC_TAC_TOE, BarTicTacToeClass))
#define BAR_IS_TIC_TAC_TOE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BAR_TYPE_TIC_TAC_TOE))
#define BAR_IS_TIC_TAC_TOE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BAR_TYPE_TIC_TAC_TOE))
#define BAR_TIC_TAC_TOE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BAR_TYPE_TIC_TAC_TOE, BarTicTacToeClass))

GType      bar_tic_tac_toe_get_type (void) G_GNUC_CONST;

void       bar_tic_tac_toe_show     (void);

G_END_DECLS

#endif /* !__BAR_TIC_TAC_TOE_H__ */

