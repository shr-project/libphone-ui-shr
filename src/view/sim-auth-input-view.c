
#include "views.h"
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-dbus.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-sim.h>
#include <unistd.h>		/* for sleep */

#define _MAX_PIN_LENGTH 9

struct SimAuthInputViewData {
	struct Window *win;

	int mode;

	char stars[_MAX_PIN_LENGTH];
	char pin[_MAX_PIN_LENGTH];
	int pin_length;

	char puk[_MAX_PIN_LENGTH];
	int puk_length;
	char pin_confirm[_MAX_PIN_LENGTH];
	int pin_confirm_length;

	Evas_Object *bt1, *bt2, *keypad;
};

enum SimAuthModes {
	MODE_PIN,
	MODE_PUK,
	MODE_PUK_NEW_PIN,
	MODE_PUK_NEW_PIN_CONFIRM
};


void
  sim_auth_update(struct SimAuthInputViewData *data);
void
  sim_auth_clear(struct SimAuthInputViewData *data);

int
  pins_different_callback(struct SimAuthInputViewData *data);
void
  sim_auth_callback(GError * error, int status, gpointer data);
int
  reset_callback(void *data);

void
  sim_auth_keypad_clicked(void *_data, Evas_Object * obj, void *event_info);
void
  sim_auth_delete_clicked(void *_data, Evas_Object * obj, void *event_info);
void
  sim_auth_ok_clicked(void *_data, Evas_Object * obj, void *event_info);

static void
  frame_input_show(void *data);
static void
  frame_input_hide(void *data);
static void
  frame_checking_show(void *data);
static void
  frame_pins_different_show(void *data);
static void
  frame_pin_correct_show(void *data);
static void
  frame_pin_wrong_show(void *data);
static void
  frame_puk_correct_show(void *data);
static void
  frame_puk_wrong_show(void *data);
static void
  frame_pin_invalid_length_show(void *data);



void *
sim_auth_input_view_show(struct Window *win, void *_options)
{
	struct SimAuthInputViewData *data;
	GHashTable *options = (GHashTable *) _options;

	g_debug("sim_auth_input_view_show(win=%d)", win);

	/* see if we are called for the first time and create
	 * the data if so... otherwise update */
	if (!win->view_data) {
		data = g_slice_alloc0(sizeof(struct SimAuthInputViewData));
		data->win = win;
	}
	else {
		data = (struct SimAuthInputViewData *)win->view_data;
	}

	int status = GPOINTER_TO_INT(g_hash_table_lookup(options, "status"));
	switch (status) {
	case SIM_PIN_REQUIRED:
		g_debug("SIM_PIN_REQUIRED");
		data->mode = MODE_PIN;
		break;
	case SIM_PUK_REQUIRED:
		g_debug("SIM_PUK_REQUIRED");
		data->mode = MODE_PUK;
		break;
	default:
		g_error("Unknown status %d", status);
		break;
	}

	window_frame_show(win, data, frame_input_show, frame_input_hide);
	return data;
}

void
sim_auth_input_view_hide(void *_data)
{
	struct SimAuthInputViewData *data =
		(struct SimAuthInputViewData *) _data;

	g_debug("sim_auth_input_view_hide()");
}

void
sim_auth_update(struct SimAuthInputViewData *data)
{
	int length;

	g_debug("sim_auth_update() MODE: %d", data->mode);

	if (data->mode == MODE_PUK)
		length = data->puk_length;
	else if (data->mode == MODE_PIN || data->mode == MODE_PUK_NEW_PIN)
		length = data->pin_length;
	else			// matches MODE_PUK_NEW_PIN_CONFIRM
		length = data->pin_confirm_length;

	int i;
	for (i = 0; i < length && i < _MAX_PIN_LENGTH; i++)
		data->stars[i] = '*';
	data->stars[i] = 0;
	window_text_set(data->win, "input_text", data->stars);
}

void
sim_auth_clear(struct SimAuthInputViewData *data)
{
	data->pin[0] = 0;
	data->pin_length = 0;
	data->puk[0] = 0;
	data->puk_length = 0;
	data->pin_confirm[0] = 0;
	data->pin_confirm_length = 0;
}


int
reset_callback(void *_data)
{
	struct SimAuthInputViewData *data =
		(struct SimAuthInputViewData *) _data;

	sim_auth_clear(data);
	window_frame_show(data->win, data, frame_input_show, frame_input_hide);

	return 0;
}

void
sim_auth_callback(GError * error, int status, gpointer _data)
{
	struct SimAuthInputViewData *data =
		(struct SimAuthInputViewData *) _data;

	g_debug("sim_auth_callback()");

	if (status == SIM_PIN_REQUIRED)
		data->mode = MODE_PIN;
	else if (status == SIM_PUK_REQUIRED)
		data->mode = MODE_PUK;
	else
		g_error("Unhandled sim auth status: %d", status);

	reset_callback(_data);
}

void
sim_auth_ok_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct SimAuthInputViewData *data =
		(struct SimAuthInputViewData *) _data;

	g_debug("sim_auth_ok_clicked(win=%d)", data->win);

	if (data->mode == MODE_PIN && strcmp(data->pin, "")) {
		if (string_is_pin(data->pin)) {
			// PIN length is correct, try it
			g_debug("PIN looks correct... sending");
			window_frame_show(data->win, data, frame_checking_show,
					  NULL);
			phoneui_sim_pin_send(data->pin);
		}
		else {
			g_debug("no valid PIN...");
			// PIN length is wrong, thus PIN is wrong
			window_frame_show(data->win, data, frame_pin_wrong_show,
					  NULL);
			ecore_timer_add(2, reset_callback, data);
		}
	}
	else if (data->mode == MODE_PUK && strcmp(data->puk, "")) {
		g_debug("Ask for a new PIN");
		window_text_set(data->win, "instruction",
				D_("Enter a NEW PIN"));
		data->mode = MODE_PUK_NEW_PIN;
		g_debug("MODE: %d", data->mode);
		sim_auth_update(data);
	}
	else if (data->mode == MODE_PUK_NEW_PIN && strcmp(data->pin, "")) {
		g_debug("Ask for new PIN confirmation");
		window_text_set(data->win, "instruction",
				D_("Confirm your NEW PIN"));
		data->mode = MODE_PUK_NEW_PIN_CONFIRM;
		sim_auth_update(data);
	}
	else if (data->mode == MODE_PUK_NEW_PIN_CONFIRM) {
		g_debug("See if NEW PINs are identical");
		if (!string_is_pin(data->pin)) {
			g_debug("NEW PIN must be 4-8 chars long and consist of digits");
			data->mode = MODE_PUK;
			window_frame_show(data->win, data,
					  frame_pin_invalid_length_show, NULL);
			ecore_timer_add(2, reset_callback, data);
		}
		else if (strcmp(data->pin, data->pin_confirm)) {
			g_debug("NEW PINs different");
			data->mode = MODE_PUK;
			window_frame_show(data->win, data,
					  frame_pins_different_show, NULL);
			ecore_timer_add(2, reset_callback, data);
		}
		else if (!string_is_puk(data->puk)) {
			g_debug("PUK is invalid");
			data->mode = MODE_PUK;
			window_frame_show(data->win, data, frame_puk_wrong_show,
					  NULL);
			ecore_timer_add(2, reset_callback, data);
		}
		else {
			g_debug("NEW PINs identical, send PUK and NEW PIN");
			window_frame_show(data->win, data, frame_checking_show,
					  NULL);
			phoneui_sim_puk_send(data->puk, data->pin);
		}
	}
}

void
sim_auth_keypad_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct SimAuthInputViewData *data =
		(struct SimAuthInputViewData *) _data;
	char *string;
	int *length;
	char input = ((char *) event_info)[0];

	g_debug("MODE: %d", data->mode);


	if (data->mode == MODE_PIN || data->mode == MODE_PUK_NEW_PIN) {
		string = data->pin;
		length = &(data->pin_length);
	}
	else if (data->mode == MODE_PUK) {
		string = data->puk;
		length = &(data->puk_length);
	}
	else {
		string = data->pin_confirm;
		length = &(data->pin_confirm_length);
	}

	if (*length < _MAX_PIN_LENGTH) {
		string[*length] = input;
		(*length)++;
		string[*length] = 0;
		sim_auth_update(data);
	}
}

void
sim_auth_delete_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct SimAuthInputViewData *data =
		(struct SimAuthInputViewData *) _data;
	char *string;
	int *length;

	if (data->mode == MODE_PIN || data->mode == MODE_PUK_NEW_PIN) {
		string = data->pin;
		length = &(data->pin_length);
	}
	else if (data->mode == MODE_PUK) {
		string = data->puk;
		length = &(data->puk_length);
	}
	else {
		string = data->pin_confirm;
		length = &(data->pin_confirm_length);
	}

	if (*length > 0) {
		(*length)--;
		string[*length] = 0;
		sim_auth_update(data);
	}
}




/* --- main input frame --- */

static void
frame_input_show(void *_data)
{
	struct SimAuthInputViewData *data =
		(struct SimAuthInputViewData *) _data;
	struct Window *win = data->win;

	g_debug("frame_input_show()");

	window_layout_set(win, SIM_AUTH_FILE, "sim_auth_input");

	if (data->mode == MODE_PIN)
		window_text_set(win, "instruction",
				D_("Please enter your PIN"));
	else
		window_text_set(win, "instruction",
				D_("Please enter your PUK"));

	data->bt1 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt1, D_("Delete"));
	evas_object_smart_callback_add(data->bt1, "clicked",
				       sim_auth_delete_clicked, data);
	window_swallow(win, "button_delete", data->bt1);
	evas_object_show(data->bt1);

	data->bt2 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt2, D_("OK"));
	evas_object_smart_callback_add(data->bt2, "clicked",
				       sim_auth_ok_clicked, data);
	window_swallow(win, "button_ok", data->bt2);
	evas_object_show(data->bt2);

	data->keypad = elm_keypad_add(window_evas_object_get(win));
	evas_object_smart_callback_add(data->keypad, "clicked",
				       sim_auth_keypad_clicked, data);
	window_swallow(win, "keypad", data->keypad);
	evas_object_show(data->keypad);
}

static void
frame_input_hide(void *_data)
{
	struct SimAuthInputViewData *data =
		(struct SimAuthInputViewData *) _data;
	struct Window *win = data->win;

	g_debug("frame_input_hide()");

	evas_object_del(data->bt1);
	evas_object_del(data->bt2);
	evas_object_smart_callback_del(data->keypad, "clicked",
				       sim_auth_keypad_clicked);
	evas_object_del(data->keypad);
}

static void
frame_checking_show(void *_data)
{
	struct SimAuthInputViewData *data =
		(struct SimAuthInputViewData *) _data;

	window_layout_set(data->win, SIM_AUTH_FILE, "sim_auth_checking");
	window_text_set(data->win, "text", D_("Checking"));
}

static void
frame_pin_correct_show(void *_data)
{
	struct SimAuthInputViewData *data =
		(struct SimAuthInputViewData *) _data;

	window_layout_set(data->win, SIM_AUTH_FILE, "sim_auth_pin_correct");
	window_text_set(data->win, "text", D_("PIN correct"));
}

static void
frame_pin_wrong_show(void *_data)
{
	struct SimAuthInputViewData *data =
		(struct SimAuthInputViewData *) _data;

	window_layout_set(data->win, SIM_AUTH_FILE, "sim_auth_pin_wrong");
	window_text_set(data->win, "text", D_("PIN wrong"));
}

static void
frame_pins_different_show(void *_data)
{
	struct SimAuthInputViewData *data =
		(struct SimAuthInputViewData *) _data;

	window_layout_set(data->win, SIM_AUTH_FILE, "sim_auth_pins_different");
	window_text_set(data->win, "text", D_("PIN confirmation wrong"));
}

static void
frame_puk_correct_show(void *_data)
{
	struct SimAuthInputViewData *data =
		(struct SimAuthInputViewData *) _data;

	window_layout_set(data->win, SIM_AUTH_FILE, "sim_auth_puk_correct");
	window_text_set(data->win, "text", D_("New PIN saved"));
}

static void
frame_puk_wrong_show(void *_data)
{
	struct SimAuthInputViewData *data =
		(struct SimAuthInputViewData *) _data;

	window_layout_set(data->win, SIM_AUTH_FILE, "sim_auth_puk_wrong");
	window_text_set(data->win, "text", D_("PUK wrong"));
}

static void
frame_pin_invalid_length_show(void *_data)
{
	struct SimAuthInputViewData *data =
		(struct SimAuthInputViewData *) _data;

	window_layout_set(data->win, SIM_AUTH_FILE,
			  "sim_auth_pin_invalid_length");
	window_text_set(data->win, "text", D_("PIN must be 4-8 chars long"));
}
