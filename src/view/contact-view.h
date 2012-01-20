/*
 *  Copyright (C) 2009, 2010
 *      Authors (alphabetical) :
 *		Tom "TAsn" Hacohen <tom@stosb.com>
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


#ifndef _CONTACT_VIEW_H
#define _CONTACT_VIEW_H

#include <glib.h>
#include "ui-utils.h"

struct ContactViewData {
	struct View parent;
	Evas_Object *pager, *pager_layout;
	Evas_Object *fields, *name, *number, *photo;
	Evas_Object *btn_call, *btn_sms, *btn_delete, *btn_addfield;
	Evas_Object *btn_save, *btn_cancel;
	char *path;
	int entryid;
	GHashTable *properties, *changes;
};

struct ContactFieldData {
	char *type;
	char *name;
	char *value;
	Evas_Object *field_button, *value_entry, *slide_buttons;
	Elm_Object_Item *item;
	struct ContactViewData *view;
/*temporary*/
	Evas_Object *edit_widget;
	int isnew;
/* hack because the signals from edje don't work */
	int edit_on;
};


int contact_view_init(char *path, GHashTable *options);
int contact_view_is_init(const char *path);
void contact_view_deinit(struct ContactViewData *view);
void contact_view_show(const char *path);
void contact_view_hide(const char *path);

#endif
