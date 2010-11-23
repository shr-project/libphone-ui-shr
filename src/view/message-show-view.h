/*
 *  Copyright (C) 2009, 2010
 *      Authors (alphabetical) :
 *		Klaus 'mrmoku' Kurzmann <mok@fluxnetz.de>
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


#ifndef _MESSAGE_SHOW_VIEW_H
#define _MESSAGE_SHOW_VIEW_H

#include <glib.h>
#include <Elementary.h>
#include "ui-utils.h"

struct MessageShowViewData {
	struct View parent;
	char *path, *number, *name, *photopath;
	GHashTable *properties;
	Evas_Object *content, *photo, *sc_content, *hv;

	void (*callback) ();
	void *callback_data;
};

int message_show_view_init(char *path, GHashTable *properties);
int message_show_view_is_init(const char *path);
void message_show_view_deinit(struct MessageShowViewData *view);
void message_show_view_show(const char *path);
void message_show_view_hide(const char *path);

#endif
