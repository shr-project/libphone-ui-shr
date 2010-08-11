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


#ifndef _IDLE_VIEW_H
#define _IDLE_VIEW_H
#include <phoneui/phoneui.h>

void idle_screen_view_show();
void idle_screen_view_hide();
void idle_screen_view_toggle();

int idle_screen_view_is_init();
int idle_screen_view_init();
void idle_screen_view_deinit();

void idle_screen_view_update_unfinished_tasks(const int amount);
void idle_screen_view_update_call(enum PhoneuiCallState state,
		const char *name, const char *number);
void idle_screen_view_update_alarm(const int alarm);

#endif

