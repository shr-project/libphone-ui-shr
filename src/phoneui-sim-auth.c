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


#include <glib.h>
#include "phoneui-shr.h"
#include "ui-utils.h"
#include "views.h"
#include "phoneui-sim-auth.h"
#include <sim-auth-input-view.h>


void
phoneui_backend_sim_auth_show(const int status)
{
	// FIXME: remove status from the specs
	(void) status;

	if (!sim_auth_input_view_is_init()) {
		if (sim_auth_input_view_init()) {
			return;
		}
	}
	sim_auth_input_view_show();
}

void
phoneui_backend_sim_auth_hide(const int status)
{
	// FIXME: remove status from the specs
	(void) status;
	sim_auth_input_view_hide();
}
