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


#include "views.h"
#include "call-common.h"
#include "common-utils.h"
#include <phoneui/phoneui-utils-calls.h>
#include <phoneui/phoneui-utils-contacts.h>
#include <phoneui/phoneui-utils-sound.h>
#include "phoneui-shr.h"

static void call_button_dtmf_clicked(struct CallActiveViewData *data,
				     Evas_Object * obj, void *event_info);
static void call_button_state_clicked(struct CallActiveViewData *data,
				      Evas_Object * obj, void *event_info);
static void
_speaker_toggle_change(void *data, Evas_Object *obj, void *event_info)
{
	(void) data;
	(void) event_info;
	if (elm_check_state_get(obj))
		call_common_set_sound_state(SOUND_STATE_SPEAKER, SOUND_STATE_TYPE_NULL);
	else
		call_common_set_sound_state(SOUND_STATE_CALL, SOUND_STATE_TYPE_NULL);
}

static void
_mute_toggle_change(void *data, Evas_Object *obj, void *event_info)
{
	(void) data;
	(void) event_info;
	phoneui_utils_sound_volume_mute_set(CONTROL_MICROPHONE,
			elm_check_state_get(obj));
}

static void
_volume_slider_change(void *data, Evas_Object *obj, void *event_info)
{
	(void) data;
	(void) event_info;
	int vol = (int)elm_slider_value_get(obj);
	g_debug("volume changed to %d", vol);
	phoneui_utils_sound_volume_set(CONTROL_SPEAKER, vol);
}

static void
_mic_slider_change(void *data, Evas_Object *obj, void *event_info)
{
	(void) data;
	(void) event_info;
	int vol = (int)elm_slider_value_get(obj);
	g_debug("mic changed to %d", vol);
	phoneui_utils_sound_volume_set(CONTROL_MICROPHONE, vol);
}

static void
_volume_changed(enum SoundControlType type, int value, void *_data)
{
	struct CallActiveViewData *data = (struct CallActiveViewData *)_data;
	switch (type) {
	case CONTROL_SPEAKER:
		g_debug("new speaker volume is %d", value);
		elm_slider_value_set(data->volume_slider, (double)value);
		break;
	case CONTROL_MICROPHONE:
		g_debug("new mic sensitivity is %d", value);
		elm_slider_value_set(data->mic_slider, (double)value);
		break;
	case CONTROL_END:
		break;
	}
}

static void
_mute_changed(enum SoundControlType type, int mute, void *_data)
{
	struct CallActiveViewData *data = (struct CallActiveViewData *)_data;
	g_debug("Mute changed: type %d value %d", type, mute);
	if (type == CONTROL_MICROPHONE) {
		elm_check_state_set(data->mute_toggle, mute);
	}
}


struct CallActiveViewData *
call_active_view_show(struct Window *win, GHashTable * options)
{
	g_debug("call_active_show()");

	struct CallActiveViewData *data =
		common_utils_object_ref(calloc(1, sizeof(struct CallActiveViewData)));
	data->parent.options = options;
	data->parent.win = win;
	data->parent.id = GPOINTER_TO_INT(g_hash_table_lookup(options, "id"));
	data->parent.number = g_hash_table_lookup(options, "number");
	data->parent.name = g_hash_table_lookup(options, "name");
	data->parent.photo = g_hash_table_lookup(options, "photo");
	data->parent.dtmf_active = FALSE;
	data->state = CALL_STATE_ACTIVE;
	data->parent.number_state =
		GPOINTER_TO_INT(g_hash_table_lookup(options, "number_state"));

	/* INIT a call */
	call_common_active_call_add(data);

	g_debug("active call: id=%d, number_state=%d, number='%s'", data->parent.id,
			data->parent.number_state, data->parent.number);

	window_layout_set(win, phoneui_theme, "phoneui/call_management/active_call");

	data->parent.elmphoto = elm_icon_add(window_evas_object_get(win));
	window_swallow(win, "photo", data->parent.elmphoto);
	evas_object_show(data->parent.elmphoto);

	window_text_set(win, "number", data->parent.number);
	if (data->parent.number_state == CALL_NUMBER_NUMBER) {
		phoneui_utils_contact_lookup(data->parent.number,
					call_common_contact_callback,
					common_utils_object_ref(
						(struct CallViewData *) data));
	}
	else {
		window_text_set(win, "name", data->parent.name);
		elm_image_file_set(data->parent.elmphoto, data->parent.photo, NULL);
	}

	//g_debug("adding the hangup toggle...");
	//data->hangup_toggle = elm_toggle_add(window_evas_object_get(win));
	//evas_object_smart_callback_add(data->hangup_toggle, "changed",
	//		_hangup_toggle_change, data);
	//elm_object_style_set(data->hangup_toggle, "hangup");
	//window_swallow(win, "hangup_toggle", data->hangup_toggle);
	//evas_object_show(data->hangup_toggle);

	//Evas_Object *ico = elm_icon_add(window_evas_object_get(win));
	//elm_image_file_set(ico, "speaker.png", "phoneui/images");
	//evas_object_show(ico);

	g_debug("adding the speaker toggle...");
	data->speaker_toggle = elm_check_add(window_evas_object_get(win));
	elm_object_style_set(data->speaker_toggle, "toggle");
	elm_object_part_text_set(data->speaker_toggle, "on", D_("Speaker"));
	elm_check_state_set(data->speaker_toggle, EINA_FALSE);
	elm_object_scale_set(data->speaker_toggle, 1.2);
	evas_object_smart_callback_add(data->speaker_toggle, "changed",
			_speaker_toggle_change, data);
	window_swallow(win, "speaker_toggle", data->speaker_toggle);
	evas_object_show(data->speaker_toggle);

	g_debug("adding the mute toggle...");
	data->mute_toggle = elm_check_add(window_evas_object_get(win));
	elm_object_style_set(data->mute_toggle, "toggle");
	elm_object_part_text_set(data->mute_toggle, "on", D_("Mute"));
	elm_check_state_set(data->mute_toggle, EINA_FALSE);
	elm_object_scale_set(data->mute_toggle, 1.2);
	evas_object_smart_callback_add(data->mute_toggle, "changed",
			_mute_toggle_change, data);
	window_swallow(win, "mute_toggle", data->mute_toggle);
	evas_object_show(data->mute_toggle);

	g_debug("adding the volume slider...");
	data->volume_slider = elm_slider_add(window_evas_object_get(win));
	elm_object_text_set(data->volume_slider, D_("Volume"));
	elm_slider_min_max_set(data->volume_slider, 0.0, 100.0);
	elm_slider_value_set(data->volume_slider,
			(double)phoneui_utils_sound_volume_get(
				CONTROL_SPEAKER));
	//elm_slider_horizontal_set(data->volume_slider, EINA_FALSE);
	evas_object_smart_callback_add(data->volume_slider, "delay,changed",
			_volume_slider_change, data);
	window_swallow(win, "volume_slider", data->volume_slider);
	evas_object_show(data->volume_slider);

	g_debug("adding the mic slider...");
	data->mic_slider = elm_slider_add(window_evas_object_get(win));
	elm_object_text_set(data->mic_slider, D_("Mic"));
	elm_slider_min_max_set(data->mic_slider, 0.0, 100.0);
	elm_slider_value_set(data->mic_slider,
			(double)phoneui_utils_sound_volume_get(
				CONTROL_MICROPHONE));
	evas_object_smart_callback_add(data->mic_slider, "delay,changed",
			_mic_slider_change, data);
	window_swallow(win, "mic_slider", data->mic_slider);
	evas_object_show(data->mic_slider);

	data->bt_call_state = elm_button_add(window_evas_object_get(win));
	elm_object_text_set(data->bt_call_state, D_("Release"));
	evas_object_smart_callback_add(data->bt_call_state, "clicked",
				       (Evas_Smart_Cb) call_button_state_clicked, data);
	window_swallow(win, "button_release", data->bt_call_state);
	evas_object_show(data->bt_call_state);

	data->bt_keypad = elm_button_add(window_evas_object_get(win));
	elm_object_text_set(data->bt_keypad, D_("Keypad"));
	evas_object_smart_callback_add(data->bt_keypad, "clicked",
				       (Evas_Smart_Cb) call_button_dtmf_clicked, data);
	window_swallow(win, "button_dtmf", data->bt_keypad);
	evas_object_show(data->bt_keypad);

	window_show(win);

	phoneui_utils_sound_volume_change_callback_set(
			_volume_changed, data);
	phoneui_utils_sound_volume_mute_change_callback_set(
			_mute_changed, data);

	return data;
}

void
call_active_view_hide(struct CallActiveViewData *data)
{
	g_debug("call_active_hide()");

	if (data->parent.dtmf_active) {
		call_dtmf_disable(&(data->parent));
	}

	phoneui_utils_sound_volume_change_callback_set(NULL, NULL);
	phoneui_utils_sound_volume_mute_change_callback_set(NULL, NULL);

	data->parent.number_state = CALL_NUMBER_NULL;
	evas_object_del(data->parent.elmphoto);
	evas_object_del(data->mute_toggle);
	evas_object_del(data->speaker_toggle);
	evas_object_del(data->volume_slider);
	evas_object_del(data->bt_call_state);
	evas_object_del(data->bt_keypad);

	common_utils_object_unref_free(data);
}

static void
call_button_dtmf_clicked(struct CallActiveViewData *data, Evas_Object * obj,
			 void *event_info)
{
	(void) obj;
	(void) event_info;
	g_debug("dtmf_clicked()");
	if (data->parent.dtmf_active) {
		data->parent.dtmf_active = FALSE;
		call_dtmf_disable((struct CallViewData *) data);
		elm_object_text_set(data->bt_keypad, D_("Keypad"));
	}
	else {
		data->parent.dtmf_active = TRUE;
		call_dtmf_enable((struct CallViewData *) data);
		elm_object_text_set(data->bt_keypad, D_("Hide Keypad"));
	}
}

void
call_button_state_clicked(struct CallActiveViewData *data, Evas_Object * obj,
			  void *event_info)
{
	(void) obj;
	(void) event_info;
	g_debug("state_clicked(id=%d,state=%d)", data->parent.id, data->state);
	if (data->state == CALL_STATE_ACTIVE) {
		phoneui_utils_call_release(data->parent.id, NULL, NULL);
	}
	else if (data->state == CALL_STATE_PENDING) {
		phoneui_utils_call_activate(data->parent.id, NULL, NULL);
		call_common_window_new_active(data->parent.id);
	}
	else {
		g_debug("bad state, BUG! shouldn't have gotten here");
	}
}
