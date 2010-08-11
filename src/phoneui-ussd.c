/*
 *  Copyright (C) 2009, 2010
 *      Authors (alphabetical) :
 * 		Tom "TAsn" Hacohen <tom@stosb.com>
 * 		Klaus 'mrmoku' Kurzmann <mok@fluxnetz.de>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 */


#include "phoneui-ussd.h"
#include <glib.h>
#include "phoneui-shr.h"
#include "window.h"
#include "views.h"


void
phoneui_backend_ussd_show(int mode, const char *message)
{
	g_debug("phoneui_backend_ussd_show(mode=%d, message=%s)", mode,
		message);
	struct Window *win = window_new(D_("Service Data"));

	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(options, "mode", GINT_TO_POINTER(mode));
	g_hash_table_insert(options, "message", g_strdup((char *) message));	/* we lose the const here */
	window_init(win);
	window_view_show(win, options, ussd_view_show, ussd_view_hide, NULL);
}

