#include "views.h"
#include "call-common.h"
#include "common-utils.h"
#include "widget/elm_keypad.h"

#include <phoneui/phoneui-utils.h>




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
		g_prefix_error(error, " activating call failed (id=%d)",
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
	phongeui_call_activate(win->parent.id, _call_activate_callback, win);
#else
	phoneui_utils_call_activate(win->parent.id, NULL, NULL);
	call_common_window_to_active(win);
#endif
}

void
call_common_contact_callback(GHashTable *contact, void *_data)
{
	struct CallViewData *data =
		(struct CallViewData *) _data;
	if (data->number_state == CALL_NUMBER_NULL) {
		common_utils_object_unref_free(data);
		return;
	}
	if (contact) {
		g_debug("call_common_contact_callback... got a contact");
		GValue *tmp;
		char *s;

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

		tmp = g_hash_table_lookup(contact, "_Name");
		if (tmp) {
			s = g_value_get_string(tmp);
		}
		else {
			s = CONTACT_NAME_UNDEFINED_STRING;
		}
		data->name = g_strdup(s);
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
				enum SoundState state)
{
	const char *state_string = "";

	switch (state) {
	case SOUND_STATE_SPEAKER:
		state_string = D_("Handset");
		break;
	case SOUND_STATE_HEADSET:
		break;
		/* default to handset */
	case SOUND_STATE_IDLE:
	case SOUND_STATE_HANDSET:
		state_string = D_("Speaker");
		break;
	case SOUND_STATE_BT:
		break;
	default:
		break;
	}

	elm_button_label_set(win->bt_sound_state, state_string);
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
		g_queue_foreach(active_calls_list, _foreach_new_active,
				(void *) id);
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
call_common_set_sound_state(enum SoundState state)
{
	phoneui_utils_sound_state_set(state);
	if (active_calls_list) {
		g_queue_foreach(active_calls_list,
				call_common_window_update_state,
				(void *) state);
	}
}

int
call_common_active_call_add(struct CallActiveViewData *win)
{
	/* if it's not the first call, update all the windows */
	if (active_calls_list) {
		g_queue_foreach(active_calls_list, _foreach_new_active,
				(void *) -1);
	}
	/*init */
	/* if first, init state */
	else {
		call_common_set_sound_state(SOUND_STATE_INIT);
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
		GList *link = g_queue_find_custom(active_calls_list, id,
						  _queue_find_by_id);
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
		call_common_set_sound_state(SOUND_STATE_IDLE);
	}
	return 0;
}

void
call_button_keypad_clicked(void *data, Evas_Object * obj,
			   void *event_info)
{
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
