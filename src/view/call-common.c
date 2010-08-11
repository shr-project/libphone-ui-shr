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


#include "phoneui-shr.h"
#include "views.h"
#include "call-common.h"
#include "common-utils.h"
#include "widget/elm_keypad.h"

#include <phoneui/phoneui-utils-calls.h>
#include <phoneui/phoneui-utils-contacts.h>




static GQueue *active_calls_list = NULL;

static void
_call_activate_callback(GError * error, struct CallActiveViewData *win)
{
	if (!error) {
		g_debug("%s:%d activated call (id=%d)", __FILE__, __LINE__,
			win->parent.id);
		call_common_window_to_active(win);
	}
	else {
		g_prefix_error(&error, " activating call failed (id=%d)",
			       win->parent.id);
		g_error_free(error);
	}
}

void
call_common_activate_call(struct CallActiveViewData *win)
{
	g_debug("%s:%d attempting to set last call as active (id=%d)", __FILE__,
		__LINE__, win->parent.id);
#if 0
	phoneui_utils_call_activate(win->parent.id, _call_activate_callback, win);
#else
	(void) _call_activate_callback;
	phoneui_utils_call_activate(win->parent.id, NULL, NULL);
	call_common_window_to_active(win);
#endif
}

void
call_common_contact_callback(GError *error, GHashTable *contact, void *_data)
{
	(void) error;
	// FIXME: show notification on error
	struct CallViewData *data =
		(struct CallViewData *) _data;
	if (data->number_state == CALL_NUMBER_NULL) {
		common_utils_object_unref_free(data);
		return;
	}
	if (contact) {
		g_debug("call_common_contact_callback... got a contact");
		GValue *tmp;
		const char *s;
		char *s2;

		tmp = g_hash_table_lookup(contact, "Photo");
		if (tmp) {
			s = g_value_get_string(tmp);
			if (!strncmp(s, "file://", 7))
				s += 7;
		}
		else {
			s = CONTACT_DEFAULT_PHOTO;
		}
		data->photo = g_strdup(s);

		s2 = phoneui_utils_contact_display_name_get(contact);
		if (s2) {
			window_text_set(data->win, "name", s2);
			data->name = s2;
		}
		else {
			data->name = strdup(CONTACT_NAME_UNDEFINED_STRING);
		}
	}
	else {
		g_debug("call_common_contact_callback... got NO contact");
		data->photo = g_strdup(CONTACT_DEFAULT_PHOTO);
		data->name = g_strdup(CONTACT_NAME_UNDEFINED_STRING);
	}

	elm_icon_file_set(data->elmphoto, data->photo, NULL);
	window_text_set(data->win, "name", data->name);
	data->number_state = CALL_NUMBER_CONTACT;
	common_utils_object_unref_free(data);
}



void
call_common_window_update_state(struct CallActiveViewData *win,
				enum SoundState state, enum SoundStateType type)
{
	const char *state_string = "";
	int speaker_state = 0;

	switch (state) {
	case SOUND_STATE_SPEAKER:
		speaker_state = 1;
		switch (type) {
		case SOUND_STATE_TYPE_BLUETOOTH:
			state_string = D_("Bluetooth");
			break;
		case SOUND_STATE_TYPE_HANDSET:
			state_string = D_("Handset");
			break;
		case SOUND_STATE_TYPE_HEADSET:
			state_string = D_("Headset");
			break;
		default:
			speaker_state = 0; /*rollback*/
			break;
		}
		break;
	case SOUND_STATE_IDLE:
	case SOUND_STATE_CALL:
		state_string = D_("Speaker");
		speaker_state = 0;
		break;
	default:
		break;
	}

	elm_toggle_state_set(win->speaker_toggle, speaker_state);
}

static void
_foreach_new_active(struct CallActiveViewData *win, int id)
{
	if (id != win->parent.id) {
		call_common_window_to_pending(win);
	}
	else {
		call_common_window_to_active(win);
		g_queue_remove(active_calls_list, win);
		g_queue_push_head(active_calls_list, win);
	}
}

void
call_common_window_new_active(int id)
{
	g_debug("%s:%d setting new active call (id=%d)", __FILE__, __LINE__,
		id);

	if (active_calls_list) {
		g_queue_foreach(active_calls_list, (GFunc) _foreach_new_active,
				GINT_TO_POINTER(id));
	}
}

void
call_common_window_to_pending(struct CallActiveViewData *win)
{
	if (win->state == CALL_STATE_ACTIVE) {
		elm_button_label_set(win->bt_call_state, D_("Pickup"));
	}
	else if (win->state == CALL_STATE_PENDING) {
		/*Do nothing as we want it to be pending */
		g_debug("Found a pending call while expecting none! (%d)",
			win->parent.id);
	}
	else {
		g_debug("Bad state (%d) for an active call!", win->state);
	}
	win->state = CALL_STATE_PENDING;
}

void
call_common_window_to_active(struct CallActiveViewData *win)
{
	if (win->state == CALL_STATE_ACTIVE) {
		/*Do nothing as we want it to be active */
		g_debug("Found an active call while expecting none! (%d)",
			win->parent.id);
	}
	else if (win->state == CALL_STATE_PENDING) {
		elm_button_label_set(win->bt_call_state, D_("Release"));
	}
	else {
		g_debug("Bad state (%d) for an active call!", win->state);
	}
	win->state = CALL_STATE_ACTIVE;
}

int
call_common_active_call_get_last_id()
{
	struct CallActiveViewData *win;
	if (active_calls_list) {
		win = g_queue_peek_head(active_calls_list);
		return win->parent.id;
	}
	else {
		return 0;
	}
}

int
call_common_set_sound_state(enum SoundState state, enum SoundStateType type)
{
	phoneui_utils_sound_state_set(state, type);
	if (active_calls_list) {
		g_queue_foreach(active_calls_list,
				(GFunc) call_common_window_update_state,
				GINT_TO_POINTER(state));
	}
	return 0;
}

int
call_common_active_call_add(struct CallActiveViewData *win)
{
	/* if it's not the first call, update all the windows */
	if (active_calls_list) {
		g_queue_foreach(active_calls_list, (GFunc) _foreach_new_active,
				GINT_TO_POINTER(-1));
	}
	/*init */
	/* if first, init state */
	else {
		call_common_set_sound_state(SOUND_STATE_CALL, SOUND_STATE_TYPE_NULL);
		g_debug("Initialized active calls list");
		active_calls_list = g_queue_new();
	}

	g_queue_push_head(active_calls_list, win);
	g_debug("%s:%d adding a call to active list (id=%d)", __FILE__,
		__LINE__, win->parent.id);

	return 0;
}

static int
_queue_find_by_id(struct CallActiveViewData *win, int id)
{
	return !(win->parent.id == id);
}

int
call_common_active_call_remove(int id)
{
	struct CallActiveViewData *win = NULL;
	if (active_calls_list) {
		/*FIXME: cast id - bad */
		GList *link = g_queue_find_custom(active_calls_list, GINT_TO_POINTER(id),
						(GCompareFunc)  _queue_find_by_id);
		win = g_queue_peek_nth(active_calls_list,
				       g_queue_link_index(active_calls_list,
							  link));

		g_queue_delete_link(active_calls_list, link);
	}

	/* if we haven't found abort */
	if (!win) {
		g_debug("%s:%d no such id! was it active? (id=%d)", __FILE__,
			__LINE__, id);
		return 1;
	}


	g_debug("%s:%d removing a call from active list (id=%d)", __FILE__,
		__LINE__, win->parent.id);

	/* if was active, get a new active */
	if (win->state == CALL_STATE_ACTIVE) {
		win = g_queue_peek_head(active_calls_list);
		if (win) {
			call_common_activate_call(win);
		}
	}
	if (g_queue_get_length(active_calls_list) == 0) {
		g_debug("Freed active calls list");
		g_queue_free(active_calls_list);
		active_calls_list = NULL;
		call_common_set_sound_state(SOUND_STATE_IDLE, SOUND_STATE_TYPE_NULL);
	}
	return 0;
}

void
call_button_keypad_clicked(void *data, Evas_Object * obj,
			   void *event_info)
{
	(void) data;
	(void) obj;
	char string[2];
	string[0] = ((char *) event_info)[0];
	string[1] = '\0';
	g_debug("call_button_keypad_clicked(): %s", string);
	phoneui_utils_call_send_dtmf(string, NULL, NULL);
}

void
call_dtmf_enable(struct CallViewData *data)
{
	g_debug("call_dtmf_enable()");
	data->keypad = elm_keypad_add(window_evas_object_get(data->win));
	evas_object_smart_callback_add(data->keypad, "clicked",
				       call_button_keypad_clicked, data);
	window_swallow(data->win, "keypad", data->keypad);
	evas_object_show(data->keypad);
}

void
call_dtmf_disable(struct CallViewData *data)
{
	g_debug("call_dtmf_disable()");
	evas_object_smart_callback_del(data->keypad, "clicked",
				       call_button_keypad_clicked);
	evas_object_del(data->keypad);
}
