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


#ifndef _WINDOW_H
#define _WINDOW_H

#include <Ecore_X.h>
#include <Elementary.h>
#include <Evas.h>
#include <glib.h>


// TODO: Make the Window structure private!
struct Window {
	Evas_Object *win, *bg, *layout;
	char *title;

	void *view_data;
	void (*view_hide_cb) (void *data);

	void (*frame_hide_cb) ();

	void (*exit_cb)();
};

struct InwinButton {
	char *label;
	void (*callback) (void *, Evas_Object *, void *);
};


typedef enum {
	KEYBOARD_OFF = ECORE_X_VIRTUAL_KEYBOARD_STATE_OFF,
	KEYBOARD_PIN = ECORE_X_VIRTUAL_KEYBOARD_STATE_PIN,
	KEYBOARD_ALPHA = ECORE_X_VIRTUAL_KEYBOARD_STATE_ALPHA,
	KEYBOARD_NUMERIC = ECORE_X_VIRTUAL_KEYBOARD_STATE_NUMERIC
} KeyboardMode;

struct Window *window_new(char *title);
void window_init(struct Window *win);
void window_show(struct Window *win);
void window_layout_set(struct Window *win, const char *file, const char *part);
Evas_Object *window_evas_object_get(struct Window *win);
Evas_Object *window_layout_get(struct Window *win);
void window_delete_callback_set(struct Window *win,
				void (*cb) (void *data, Evas_Object * win,
					    void *event_info));
void window_text_set(struct Window *win, const char *key, const char *value);
void window_swallow(struct Window *win, const char *key, Evas_Object * object);
void window_unswallow(struct Window *win, Evas_Object * object);
void window_view_show(struct Window *win, void *options,
		      void *(*show_cb) (struct Window * win, void *options),
		      void (*hide_cb) (void *data),
		      void (*exit_cb) ());
void window_view_hide(struct Window *win, void *options);
void window_frame_show(struct Window *win, void *data,
		       void (*show_cb) (void *data),
		       void (*hide_cb) (void *data));
void window_frame_hide(struct Window *win, void *data);
void window_kbd_show(struct Window *win, KeyboardMode mode);
void window_kbd_hide(struct Window *win);
void window_destroy(struct Window *win, void *data);
Evas_Object *window_inwin_dialog(struct Window *win, const char *label,
				 GList * buttons, void *data);

#endif
