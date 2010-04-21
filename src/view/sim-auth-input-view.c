
#include <unistd.h>		/* for sleep */
#include <glib.h>
#include <Evas.h>
#include <Elementary.h>
#include <freesmartphone.h>
#include <phoneui-utils-sim.h>

#include "common-utils.h"
#include "ui-utils.h"
#include "views.h"
#include "widget/elm_keypad.h"
#include "sim-auth-input-view.h"


#define _MAX_PIN_LENGTH 9
#include "phoneui-shr.h"

struct SimAuthInputViewData {
	struct View parent;

	int mode;
	const char *msg;

	char stars[_MAX_PIN_LENGTH];
	char pin[_MAX_PIN_LENGTH];
	int pin_length;

	char puk[_MAX_PIN_LENGTH];
	int puk_length;
	char pin_confirm[_MAX_PIN_LENGTH];
	int pin_confirm_length;

	Evas_Object *bt1, *bt2, *keypad;
	Evas_Object *notify;
};
static struct SimAuthInputViewData view;

enum SimAuthModes {
	MODE_ERROR,
	MODE_PIN,
	MODE_PUK,
	MODE_PUK_NEW_PIN,
	MODE_PUK_NEW_PIN_CONFIRM
};


static void _sim_auth_keypad_clicked(void *data, Evas_Object *obj, void *event_info);
static void _sim_auth_ok_clicked(void *data, Evas_Object *obj, void *event_info);
static void _sim_auth_delete_clicked(void *data, Evas_Object *obj, void *event_info);

static void _evaluate_status(const int status);
static void _sim_auth_update();
static gboolean _sim_auth_close(gpointer data);


int
sim_auth_input_view_init()
{
	g_debug("Initializing the sim-auth-input view");
	Evas_Object *win;
	int ret;

	ret = ui_utils_view_init(VIEW_PTR(view), ELM_WIN_BASIC, D_("SIM Auth"),
				 NULL, NULL, NULL);
	if (ret) {
		g_critical("Failed to init sim-auth-input view");
		return ret;
	}

	view.notify = NULL;
	win = ui_utils_view_window_get(VIEW_PTR(view));

	ui_utils_view_layout_set(VIEW_PTR(view), phoneui_theme,
				 "phoneui/notification/sim_auth_input");

	view.bt1 = elm_button_add(win);
	elm_button_label_set(view.bt1, D_("Delete"));
	evas_object_smart_callback_add(view.bt1, "clicked",
				       _sim_auth_delete_clicked, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_delete", view.bt1);
	evas_object_show(view.bt1);

	view.bt2 = elm_button_add(win);
	elm_button_label_set(view.bt2, D_("OK"));
	evas_object_smart_callback_add(view.bt2, "clicked",
				       _sim_auth_ok_clicked, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_ok", view.bt2);
	elm_object_disabled_set(view.bt2, EINA_TRUE);
	evas_object_show(view.bt2);

	view.keypad = (Evas_Object *)elm_keypad_add(win);
	evas_object_smart_callback_add(view.keypad, "clicked",
				       _sim_auth_keypad_clicked, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "keypad", view.keypad);
	evas_object_show(view.keypad);

	return 0;
}

int
sim_auth_input_view_is_init()
{
	return ui_utils_view_is_init(VIEW_PTR(view));
}

void
sim_auth_input_view_deinit()
{
	ui_utils_view_deinit(VIEW_PTR(view));

	evas_object_smart_callback_del(view.keypad, "clicked",
				       _sim_auth_keypad_clicked);
}

void
sim_auth_input_view_show(const int status)
{
	_evaluate_status(status);
	ui_utils_view_show(VIEW_PTR(view));
}

void
sim_auth_input_view_hide()
{
	ui_utils_view_hide(VIEW_PTR(view));
}



static void
_evaluate_status(const int status)
{
	const char *msg;
	switch (status) {
	case FREE_SMARTPHONE_GSM_SIM_AUTH_STATUS_PIN_REQUIRED:
		g_debug("PHONEUI_SIM_PIN_REQUIRED");
		view.mode = MODE_PIN;
		msg = D_("Please enter your PIN");
		break;
	case FREE_SMARTPHONE_GSM_SIM_AUTH_STATUS_PIN2_REQUIRED:
		g_debug("PHONEUI_SIM_PIN2_REQUIRED");
		view.mode = MODE_PIN;
		msg = D_("Please enter your PIN2");
		break;
	case FREE_SMARTPHONE_GSM_SIM_AUTH_STATUS_PUK_REQUIRED:
		g_debug("PHONEUI_SIM_PUK_REQUIRED");
		view.mode = MODE_PUK;
		msg = D_("Please enter your PUK");
		break;
	case FREE_SMARTPHONE_GSM_SIM_AUTH_STATUS_PUK2_REQUIRED:
		g_debug("PHONEUI_SIM_PUK2_REQUIRED");
		view.mode = MODE_PUK;
		msg = D_("Please enter your PUK2");
		break;
	case FREE_SMARTPHONE_GSM_SIM_AUTH_STATUS_READY:
		g_debug("PHONEUI_SIM_READY");
		sim_auth_input_view_hide();
		view.notify = ui_utils_notify(ui_utils_view_window_get(VIEW_PTR(view)),
					      D_("SIM unlocked"), 5);
		evas_object_show(view.notify);
		return;
	default:
		g_debug("Unknown status %d", status);
		view.mode = MODE_ERROR;
		view.notify = ui_utils_notify(ui_utils_view_window_get(VIEW_PTR(view)),
					      D_("Unknown error"), 10);
		evas_object_show(view.notify);
		return;
	}
	ui_utils_view_text_set(VIEW_PTR(view), "instruction", msg);
}

static void
_auth_status_cb(GError *error, FreeSmartphoneGSMSIMAuthStatus status, gpointer data)
{
	(void) error;
	(void) data;
	if (view.notify) {
		evas_object_del(view.notify);
	}
	_evaluate_status(status);
	view.pin[0] = 0;
	view.pin_length = 0;
	view.pin_confirm[0] = 0;
	view.pin_confirm_length = 0;
	view.puk[0] = 0;
	view.puk_length = 0;
	_sim_auth_update();
}

static void
_pin_send_cb(GError *error, gpointer data)
{
	(void) data;

	if (view.notify) {
		evas_object_del(view.notify);
	}

	if (error) {
		g_warning("Sending PIN failed");
		view.notify = ui_utils_notify
				(ui_utils_view_window_get(VIEW_PTR(view)),
				 D_("PIN wrong"), 0);
		phoneui_utils_sim_auth_status_get(_auth_status_cb, NULL);
		return;
	}
	g_debug("PIN worked out");
	view.notify = ui_utils_notify (ui_utils_view_window_get(VIEW_PTR(view)),
				       D_("PIN Ok"), 0);
	g_timeout_add(5, _sim_auth_close, NULL);
}

static void
_puk_send_cb(GError *error, gpointer data)
{
	(void) data;

	if (view.notify) {
		evas_object_hide(view.notify);
	}

	if (error) {
		g_critical("Sending PUK failed");
		view.notify = ui_utils_notify
				(ui_utils_view_window_get(VIEW_PTR(view)),
				 D_("PUK wrong"), 0);
		phoneui_utils_sim_auth_status_get(_auth_status_cb, NULL);
	}
	else {
		view.notify = ui_utils_notify
				(ui_utils_view_window_get(VIEW_PTR(view)),
				 D_("PUK Ok"), 0);
		g_timeout_add(5, _sim_auth_close, NULL);
	}
}

static void
_sim_auth_ok_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	(void) _data;

	switch (view.mode) {
	case MODE_PIN:
		view.notify =
			ui_utils_notify(ui_utils_view_window_get(VIEW_PTR(view)),
					D_("Checking..."), 0);
                evas_object_show(view.notify);
		phoneui_utils_sim_pin_send(view.pin, _pin_send_cb, NULL);
		break;
	case MODE_PUK:
		view.mode = MODE_PUK_NEW_PIN;
		_sim_auth_update();
		ui_utils_view_text_set(VIEW_PTR(view), "instruction",
				       D_("Please enter a new PIN"));
		break;
	case MODE_PUK_NEW_PIN:
		view.mode = MODE_PUK_NEW_PIN_CONFIRM;
		_sim_auth_update();
		ui_utils_view_text_set(VIEW_PTR(view), "instruction",
				       D_("Please confirm the new PIN"));
				       break;
	case MODE_PUK_NEW_PIN_CONFIRM:
		view.notify =
			ui_utils_notify(ui_utils_view_window_get(VIEW_PTR(view)),
					D_("Checking..."), 0);
                evas_object_show(view.notify);
		phoneui_utils_sim_puk_send(view.puk, view.pin, _puk_send_cb, NULL);
		break;
	}
}

static void
_sim_auth_keypad_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) data;

	char *string;
	int *length;
	char input = ((char *) event_info)[0];

	if (view.mode == MODE_PIN || view.mode == MODE_PUK_NEW_PIN) {
		string = view.pin;
		length = &(view.pin_length);
	}
	else if (view.mode == MODE_PUK) {
		string = view.puk;
		length = &(view.puk_length);
	}
	else {
		string = view.pin_confirm;
		length = &(view.pin_confirm_length);
	}

	if (*length < _MAX_PIN_LENGTH) {
		string[*length] = input;
		(*length)++;
		string[*length] = 0;
		_sim_auth_update();
	}
}

void
_sim_auth_delete_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) obj;
	(void) event_info;
	(void) data;
	char *string;
	int *length;

	if (view.mode == MODE_PIN || view.mode == MODE_PUK_NEW_PIN) {
		string = view.pin;
		length = &(view.pin_length);
	}
	else if (view.mode == MODE_PUK) {
		string = view.puk;
		length = &(view.puk_length);
	}
	else {
		string = view.pin_confirm;
		length = &(view.pin_confirm_length);
	}

	if (*length > 0) {
		(*length)--;
		string[*length] = 0;
		_sim_auth_update();
	}
}

static void
_sim_auth_update()
{
	Eina_Bool disabled = EINA_TRUE;
	int i, length = 0;

	if (view.mode == MODE_PIN || view.mode == MODE_PUK_NEW_PIN) {
		if (common_utils_is_pin(view.pin)) {
			disabled = EINA_FALSE;
		}
		length = view.pin_length;
	}
	else if (view.mode == MODE_PUK_NEW_PIN_CONFIRM) {
		if (common_utils_is_pin(view.pin_confirm) &&
		    !strcmp(view.pin, view.pin_confirm)) {
			disabled = EINA_FALSE;
		}
		length = view.pin_confirm_length;
	}
	else if (view.mode == MODE_PUK) {
		if (common_utils_is_puk(view.puk)) {
			disabled = EINA_FALSE;
		}
		length = view.puk_length;
	}
	else {
		g_warning("Invalid sim auth mode %d", view.mode);
	}

	elm_object_disabled_set(view.bt2, disabled);
	for (i = 0; i < length && i < _MAX_PIN_LENGTH; i++) {
		view.stars[i] = '*';
	}
	view.stars[i] = 0;
	ui_utils_view_text_set(VIEW_PTR(view), "input_text", view.stars);
}

static gboolean
_sim_auth_close(gpointer data)
{
	(void) data;
	sim_auth_input_view_deinit();
	return FALSE;
}
