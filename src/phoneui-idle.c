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


#include <phoneui/phoneui.h>

#include "phoneui-idle.h"
#include "view/idle-view.h"

void
phoneui_backend_idle_screen_show()
{
	if (!idle_screen_view_is_init()) {
		if (idle_screen_view_init()) {
			return;
		}
	}
	idle_screen_view_show();
}

void
phoneui_backend_idle_screen_deinit()
{
	idle_screen_view_deinit();
}

void
phoneui_backend_idle_screen_hide()
{
	idle_screen_view_hide();
}

void
phoneui_backend_idle_screen_toggle()
{
	if (!idle_screen_view_is_init()) {
		if (idle_screen_view_init()) {
			return;
		}
	}
	idle_screen_view_toggle();
}
