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
#include <phoneui/phoneui-utils-contacts.h>
#include "phoneui-contacts.h"
#include "view/contact-list-view.h"
#include "view/contact-view.h"
#include "view/views.h"

void
phoneui_backend_contacts_show()
{
	if (!contact_list_view_is_init()) {
		if (contact_list_view_init()) {
			return;
		}
	}
	contact_list_view_show();
}

static void
_contact_get_cb(GError *error, GHashTable *content, gpointer data)
{
	char *path = (char *)data;

	if (error || !content) {
		g_warning("Failed aquiring data for contact %s", path);
		// TODO: show some message dialog showing it did not work
		free (path);
		return;
	}

	if (contact_view_init(path, content))
		return;
	contact_view_show(path);
}


void
phoneui_backend_contacts_contact_show(const char *contact_path)
{
	if (!contact_path)
		return;

	g_debug("showing contact %s", contact_path);
	if (!contact_view_is_init(contact_path)) {
		phoneui_utils_contact_get(contact_path, _contact_get_cb,
				  strdup(contact_path));
		return;
	}
	contact_view_show(contact_path);
}


void
phoneui_backend_contacts_contact_new(GHashTable *options)
{
	g_debug("phoneui_backend_contacts_contact_new()");
	if (!contact_view_is_init("")) {
		if (contact_view_init(strdup(""), options)) {
			return;
		}
	}
	contact_view_show("");
}


void
phoneui_backend_contacts_contact_edit(const char *path)
{
	phoneui_backend_contacts_contact_show(path);
}

void
phoneui_backend_contacts_deinit()
{
	/*FIXME: also clean all the contact screens*/
	contact_list_view_deinit();
}

