/*
 *  Copyright (C) 2009, 2010
 *      Authors (alphabetical) :
 *		Tom "TAsn" Hacohen <tom@stosb.com>
 *		Klaus 'mrmoku' Kurzmann <mok@fluxnetz.de>
 *		David Kozub <zub@linux.fjfi.cvut.cz>
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


#ifndef _UI_UTILS_H
#define _UI_UTILS_H

#include <Elementary.h>
#include <Evas.h>
#include <glib.h>

#define VIEW_PTR(x) ((struct View *) &(x))
struct View {
	Evas_Object *win, *background, *layout;

	void (*show_cb) (struct View *view);
	void (*hide_cb) (struct View *view);

	void (*destroy_cb)(struct View *view);
};

enum {
	DIALOG_OK = 1,
	DIALOG_YES = 2,
	DIALOG_NO = 4,
	DIALOG_CANCEL = 8
};

struct View *
ui_utils_view_new(const char *title);

int
ui_utils_view_init(struct View *view, Elm_Win_Type type, const char *title,
			void (*show_cb) (struct View *view),
		 	void (*hide_cb) (struct View *view),
		 	void (*destroy_cb)(struct View *view));

int
ui_utils_view_is_init(struct View *view);

int
ui_utils_view_is_visible(struct View *view);

void
ui_utils_view_show(struct View *view);

void
ui_utils_view_hide(struct View *view);

void
ui_utils_view_toggle(struct View *view);

void
ui_utils_view_layout_set(struct View *view, const char *file, const char *part);

Evas_Object *
ui_utils_view_layout_get(struct View *view);

Evas_Object *
ui_utils_view_window_get(struct View *view);

void
ui_utils_view_delete_callback_set(struct View *view,
					void (*cb) (struct View *viewdow, Evas_Object *view,
					void *event_info));

void
ui_utils_view_text_set(struct View *view, const char *key, const char *value);

void
ui_utils_view_swallow(struct View *view, const char *key, Evas_Object * object);

void
ui_utils_view_unswallow(struct View *view, Evas_Object *object);

void
ui_utils_view_deinit(struct View *view);

/* Think about this a bit more... Looks sane */
Evas_Object *
ui_utils_view_inwin_dialog(struct View *view, const char *label, GList *buttons,
		    		void *data);
void
ui_utils_dialog(struct View *view, const char *label, int buttonflags,
		      void (*callback)(int, void *), void *data);

void
ui_utils_dialog_check(struct View *view, const char *label, const char *check_label,
		      Eina_Bool *check_value, int buttonflags,
		      void (*callback)(int, void *), void *data);

Evas_Object *
ui_utils_notify(Evas_Object *parent, const char *label, int timeout);

Evas_Object *
ui_utils_view_inwin_list(struct View *view, GList *list,
			 void (*callback)(const char *, void *),
			 void *data);

char *
ui_utils_entry_utf8_get(Evas_Object *entry);

void
ui_utils_error_message_show(struct View *parent, const char *error_msg,
	const char *detail_msg);
void
ui_utils_error_message_from_gerror_show(struct View *parent, const char *msg,
	const GError *err);

#endif
