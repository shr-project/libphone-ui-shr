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


#ifndef _CALL_COMMON_H
#define _CALL_COMMON_H

#include <Evas.h>
#include <phoneui/phoneui-utils.h>

enum ActiveCallState {
	CALL_STATE_ACTIVE,
	CALL_STATE_PENDING
};

enum CallNumberState {
	CALL_NUMBER_NULL,
	CALL_NUMBER_CONTACT,
	CALL_NUMBER_NUMBER
};

struct CallViewData {
	struct Window *win;
	GHashTable *options;
	int id;
	char *number;
	char *name;
	char *photo;
	enum CallNumberState number_state;
	gboolean dtmf_active;
	Evas_Object *keypad;
	Evas_Object *elmphoto;
};

struct CallActiveViewData {
	struct CallViewData parent;
	Evas_Object *bt_call_state, *bt_keypad;
	Evas_Object *mute_toggle, *speaker_toggle;
	Evas_Object *volume_slider, *mic_slider;
	enum ActiveCallState state;
};

struct CallIncomingViewData {
	struct CallViewData parent;
	Evas_Object *bt_accept, *bt_reject;
	Evas_Object *number, *name, *photo;
};

void call_common_contact_callback( GError* error, GHashTable* contact, void* _data);
void call_common_contact_callback2(void *_data);

int call_common_set_sound_state(enum SoundState state, enum SoundStateType type);


int call_common_active_call_add(struct CallActiveViewData *win);
int call_common_active_call_remove(int id);
int call_common_active_call_get_last_id();

void call_common_window_to_pending(struct CallActiveViewData *win);
void call_common_window_to_active(struct CallActiveViewData *win);
void call_common_window_new_active(int id);
void call_common_window_update_state(struct CallActiveViewData *win,
				enum SoundState state, enum SoundStateType type);

void call_button_keypad_clicked(void *data, Evas_Object * obj,
				void *event_info);
void call_dtmf_enable(struct CallViewData *data);
void call_dtmf_disable(struct CallViewData *data);

#endif
