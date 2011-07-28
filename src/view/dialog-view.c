/*
 *  Copyright (C) 2009, 2010
 *      Authors (alphabetical) :
 *		Tom "TAsn" Hacohen <tom@stosb.com>
 *		Klaus 'mrmoku' Kurzmann <mok@fluxnetz.de>
 *		Sebastian Krzyszkowiak <seba.dos1@gmail.com>
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



#include "views.h"
#include <phoneui/phoneui.h>
#include <phoneui/phoneui-utils.h>
#include "phoneui-shr.h"

struct DialogViewData {
	struct Window *win;
	Evas_Object *bt_close;
	int type;
};



static void
  dialog_view_close_clicked(void *_data, Evas_Object * obj, void *event_info);


void *
dialog_view_show(struct Window *win, void *_options)
{
	GHashTable *options = (GHashTable *) _options;
	struct DialogViewData *data;
	g_debug("dialog_view_show()");
	// Check if type was provided
	gboolean type_exists =
		g_hash_table_lookup_extended(options, "type", NULL, NULL);
	if (!type_exists) {
		g_critical("Tried to pass a non-exisiting type (%s:%d)", __FUNCTION__, __LINE__);
		return NULL;
	}

	data = calloc(1, sizeof(struct DialogViewData));
	data->win = win;
	data->type = GPOINTER_TO_INT(g_hash_table_lookup(options, "type"));



	window_layout_set(win, phoneui_theme, "phoneui/notification/dialog");
	if (data->type == PHONEUI_DIALOG_MESSAGE_STORAGE_FULL)
		window_text_set(win, "content", D_
			("Your SIM storage is full. Please delete some messages or you may not receive them anymore!"));
	else if (data->type == PHONEUI_DIALOG_SIM_NOT_PRESENT)
		window_text_set(win, "content", D_
			("GSM is not available, because no SIM card is present."));
	else
		window_text_set(win, "content", D_("Unknown message."));

	data->bt_close = elm_button_add(win->win);
	elm_object_text_set(data->bt_close, D_("Close"));
	evas_object_smart_callback_add(data->bt_close, "clicked",
				       dialog_view_close_clicked, data);
	window_swallow(win, "button_close", data->bt_close);
	evas_object_show(data->bt_close);


	window_show(win);
	return data;
}

void
dialog_view_hide(void *_data)
{
	struct DialogViewData *data = (struct DialogViewData *) _data;
	evas_object_del(data->bt_close);
}

static void
dialog_view_close_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	struct DialogViewData *data = (struct DialogViewData *) _data;
	window_destroy(data->win, NULL);
}
