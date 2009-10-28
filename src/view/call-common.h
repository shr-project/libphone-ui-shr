#ifndef _CALL_COMMON_H
#define _CALL_COMMON_H

#include "views.h"

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
	Evas_Object *bt_call_state, *bt_sound_state, *bt_keypad;
	Evas_Object *mute_toggle, *volume_slider, *speaker_toggle;
	Evas_Object *hangup_toggle;
	enum ActiveCallState state;
};

struct CallIncomingViewData {
	struct CallViewData parent;
	Evas_Object *bt_accept, *bt_reject;
	Evas_Object *number, *name, *photo;
};

enum _CallSoundState {
	CALL_SOUND_STATE_SPEAKER,
	CALL_SOUND_STATE_HANDSET,
	CALL_SOUND_STATE_HEADSET,
	CALL_SOUND_STATE_BT,
	CALL_SOUND_STATE_CLEAR,
	CALL_SOUND_STATE_INIT
};
typedef enum _CallSoundState CallSoundState;

void call_common_contact_callback(GHashTable *contact, void *_data);
void call_common_contact_callback2(void *_data);

int call_common_active_call_add(struct CallActiveViewData *win);
int call_common_active_call_remove(int id);
int call_common_active_call_get_last_id();

void call_common_window_to_pending(struct CallActiveViewData *win);
void call_common_window_to_active(struct CallActiveViewData *win);
void call_common_window_new_active(int id);
void call_common_window_update_state(struct CallActiveViewData *win,
				     CallSoundState state);

void call_common_window_update_state(struct CallActiveViewData *win,
				     CallSoundState state);
int call_common_set_sound_state(CallSoundState state);
CallSoundState call_common_get_sound_state();


void call_button_keypad_clicked(void *data, Evas_Object * obj,
				void *event_info);
void call_dtmf_enable(struct CallViewData *data);
void call_dtmf_disable(struct CallViewData *data);

#endif
