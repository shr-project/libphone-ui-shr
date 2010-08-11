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


#ifndef _MESSAGE_NEW_VIEW_H
#define _MESSAGE_NEW_VIEW_H

#include <glib.h>
#include <Evas.h>

#include "contact-list-common.h"

struct MessageNewViewData {
	struct View view;
	int mode;
	char *content;
	char number[65];
	int number_length;
	Evas_Object *pager, *left_button, *right_button;
	Evas_Object *layout_content, *layout_recipients;
	Evas_Object *layout_contacts, *layout_number;
	Evas_Object *content_entry, *number_keypad, *number_label, *hv;
	Evas_Object *list_recipients;
	Evas_Object *notify;

	GPtrArray *recipients;
	struct ContactListData contact_list_data;

	int messages_sent;
};

struct MessageNewViewData *message_new_view_init(GHashTable *options);
void message_new_view_deinit(struct MessageNewViewData *view);
void message_new_view_show(struct MessageNewViewData *view);

#endif
