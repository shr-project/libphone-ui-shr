#include <Edje_Edit.h>
#include <Elementary.h>
#include <Evas.h>
#include <phoneui/phoneui-utils.h>
#include <phoneui/phoneui-info.h>
#include <phoneui/phoneui.h>
#include <glib.h>
#include <time.h>

#include "phoneui-shr.h"
#include "views.h"
#include "idle-view.h"
#include "util/ui-utils.h"

/*FIXME: no need to pass the screen handle all the time since we only allow
 * one idle screen at a time */


struct IdleScreenViewData {
	struct View parent;
	Evas_Object *wallpaper;
};
/* No need to pass the active window everywhere, as we don't allow multiple windows. */
static struct IdleScreenViewData view;

static void _resource_status(void *data, const char *resource, gboolean state, GHashTable *properties);
static void _capacity_change(void *data, int capacity);
static void _network_status(void *data, GHashTable *properties);
static void _pdp_context_status(void *data, FreeSmartphoneGSMContextStatus status, GHashTable *properties);
static void _signal_strength(void *data, int strength);
static void _profile_change(void *data, const char *profile);
static void _missed_calls(void *data, int amount);
static void _unread_messages(void *data, int amount);
static void _unfinished_tasks(void *data, int amount);

static void _update_counter(const char *name, const char *label_name, int count);
static void _update_signal_strength(int strength);

static void
_delete_cb(struct View *view, Evas_Object * win, void *event_info)
{
        (void) view;
        (void) win;
        (void) event_info;
        idle_screen_view_hide();
}

void
idle_screen_view_show()
{
	ui_utils_view_show(VIEW_PTR(view));
}

void
idle_screen_view_hide()
{
	ui_utils_view_hide(VIEW_PTR(view));
}

void
idle_screen_view_toggle()
{
	ui_utils_view_toggle(VIEW_PTR(view));
}

int
idle_screen_view_init()
{
	Evas_Object *win;
	int ret;

	ret = ui_utils_view_init(VIEW_PTR(view), ELM_WIN_BASIC, D_("Idle_Screen"),
				NULL, NULL, NULL);
	if (ret) {
		g_critical("Failed to init idle screen");
		return ret;
	}
	ui_utils_view_delete_callback_set(VIEW_PTR(view), _delete_cb);
	ui_utils_view_layout_set(VIEW_PTR(view), IDLE_SCREEN_THEME,
			  "phoneui/idle_screen/idle_screen");

	edje_object_signal_emit(ui_utils_view_layout_get(VIEW_PTR(view)),
				"clock_init", "");

	win = ui_utils_view_window_get(VIEW_PTR(view));

	elm_win_fullscreen_set(win, 1);
	elm_win_layer_set(win, 200);
	evas_object_show(view.wallpaper);

	edje_object_signal_callback_add(ui_utils_view_layout_get(VIEW_PTR(view)), "unlockScreen",
					"slider", idle_screen_view_hide,
					NULL);

	phoneui_info_register_and_request_resource_status(_resource_status, NULL);
	phoneui_info_register_and_request_network_status(_network_status, NULL);
	phoneui_info_register_and_request_signal_strength(_signal_strength, NULL);
	phoneui_info_register_and_request_pdp_context_status(_pdp_context_status, NULL);
	phoneui_info_register_and_request_profile_changes(_profile_change, NULL);
	phoneui_info_register_and_request_capacity_changes(_capacity_change, NULL);
	phoneui_info_register_and_request_missed_calls(_missed_calls, NULL);
	phoneui_info_register_and_request_unread_messages(_unread_messages, NULL);
	phoneui_info_register_and_request_unfinished_tasks(_unfinished_tasks, NULL);

	return 0;
}

void
idle_screen_view_deinit()
{
	evas_object_del(view.wallpaper);
	ui_utils_view_deinit(VIEW_PTR(view));
}

int
idle_screen_view_is_init()
{
	return ui_utils_view_is_init(VIEW_PTR(view));
}

void
idle_screen_view_update_call(enum PhoneuiCallState state, const char *name, const char *number)
{
	if (!idle_screen_view_is_init())
		return;

	switch (state) {
	case PHONEUI_CALL_STATE_INCOMING:
		edje_object_signal_emit(ui_utils_view_layout_get(VIEW_PTR(view)), "",
					"activate_incomingCall");
		ui_utils_view_text_set(VIEW_PTR(view), "incomingCallHeading", "Incoming Call:");
		ui_utils_view_text_set(VIEW_PTR(view), "incomingCallLine1", name);
		ui_utils_view_text_set(VIEW_PTR(view), "incomingCallLine2", number);
		break;

	case PHONEUI_CALL_STATE_ACTIVE:
		edje_object_signal_emit(ui_utils_view_layout_get(VIEW_PTR(view)), "",
					"activate_incomingCall");
		ui_utils_view_text_set(VIEW_PTR(view), "incomingCallHeading", "Active Call:");
		ui_utils_view_text_set(VIEW_PTR(view), "incomingCallLine1", name);
		ui_utils_view_text_set(VIEW_PTR(view), "incomingCallLine2", number);
		break;

	case PHONEUI_CALL_STATE_RELEASE:
		edje_object_signal_emit(ui_utils_view_layout_get(VIEW_PTR(view)), "",
					"deactivate_incomingCall");
		ui_utils_view_text_set(VIEW_PTR(view), "incomingCallHeading", "");
		ui_utils_view_text_set(VIEW_PTR(view), "incomingCallLine1", "");
		ui_utils_view_text_set(VIEW_PTR(view), "incomingCallLine2", "");
		break;
	case PHONEUI_CALL_STATE_HELD:
	case PHONEUI_CALL_STATE_OUTGOING:
		/*FIXME: implement */
		break;
	}
}

void
idle_screen_view_update_alarm(const int alarm)
{
	if (idle_screen_view_is_init()) {
		if (alarm > 0) {
			edje_object_signal_emit(ui_utils_view_layout_get(VIEW_PTR(view)),
						"alarm,active", "resourceStateChange");
		}
		else {
			edje_object_signal_emit(ui_utils_view_layout_get(VIEW_PTR(view)),
						"alarm,default", "resourceStateChange");
		}
	}
}

static void
_resource_status(void *data, const char *resource,
		 gboolean state, GHashTable *properties)
{
	(void) properties;
	(void) data;
	const char *edje_state;
	g_debug("Resource Status: %s --> %s", resource, state ? "ON" : "OFF");
	if (state) {
		edje_state = "active";
	}
	else {
		edje_state = "default";
		if (strcmp(resource, "GSM") == 0) {
			ui_utils_view_text_set(VIEW_PTR(view), "gsmProvider", "");
			_update_signal_strength(0);
		}
	}
	char resource_state[32];
	snprintf(resource_state, 32, "%s,%s", resource, edje_state);
	edje_object_signal_emit(ui_utils_view_layout_get(VIEW_PTR(view)),
				resource_state, "resourceStateChange");
}

static void
_capacity_change(void *data, int capacity)
{
	(void) data;
	char buf[16];
	g_debug("Capacity is now %d", capacity);
	snprintf(buf, 16, "%d", capacity);

	edje_object_signal_emit(ui_utils_view_layout_get(VIEW_PTR(view)),
				buf, "batteryPowerChange");

}
static void
_profile_change(void *data, const char *profile)
{
	(void) data;
	g_debug("Profile changed to '%s'", profile);
	ui_utils_view_text_set(VIEW_PTR(view), "profile", profile);
}

static void
_signal_strength(void *data, int strength)
{
	(void) data;
	_update_signal_strength(strength);
}

static void
_pdp_context_status(void *data, FreeSmartphoneGSMContextStatus status,
		    GHashTable *properties)
{
	(void) data;
	(void) properties;
	g_debug("PDP ContextStatus: %s", free_smartphone_gsm_context_status_to_string(status));
}

static void
_network_status(void *data, GHashTable *properties)
{
	(void) data;
	GValue *v;
	const char *s = "";
	const char *sig = "";

	/* special case when ogsmd disappears from the bus (NameOwnerChange) */
	if (properties == NULL) {
		_update_signal_strength(0);
		ui_utils_view_text_set(VIEW_PTR(view), "gsmProvider", s);
		ui_utils_view_text_set(VIEW_PTR(view), "pdpStatus", sig);
		return;
	}

	v = g_hash_table_lookup(properties, "display");
	if (v) {
		s = g_value_get_string(v);
	}
	if (!s || !*s) {
		v = g_hash_table_lookup(properties, "provider");
		if (v) {
			s = g_value_get_string(v);
		}
	}
	g_debug("provider is '%s'", g_value_get_string(v));
	ui_utils_view_text_set(VIEW_PTR(view), "gsmProvider", s);
	v = g_hash_table_lookup(properties, "strength");
	if (v) {
		_update_signal_strength(g_value_get_int(v));
	}
	v = g_hash_table_lookup(properties, "pdp.registration");
	if (v) {
		s = g_value_get_string(v);
		if (!strcmp(s, "home") || !strcmp(s, "roaming")) {
			v = g_hash_table_lookup(properties, "act");
			if (v) {
				s = g_value_get_string(v);
				g_debug("PDP Status: %s", s);
				if (strcmp(s, "EDGE") == 0) {
					sig = "E";
				}
				else if (strcmp(s, "UMTS") == 0) {
					sig = "3G";
				}
				else if (strcmp(s, "HSDPA") == 0 ||
					strcmp(s, "HSUPA") == 0 ||
					strcmp(s, "HSDPA/HSUPA") == 0) {
					sig = "H";
				}
				else {
					sig = "G";
				}
			}
		}
	}
	ui_utils_view_text_set(VIEW_PTR(view), "pdpStatus", sig);
}

static void
_missed_calls(void *data, int amount)
{
	(void) data;
	_update_counter("missedCalls", "missedCallsLabel", amount);
}

static void
_unread_messages(void *data, int amount)
{
	(void) data;
	_update_counter("unreadMessages", "unreadMessagesLabel", amount);
}

static void
_unfinished_tasks(void *data, int amount)
{
	(void) data;
	_update_counter("unfinishedTasks", "unfinishedTasksLabel", amount);
}

static void
_update_signal_strength(int strength)
{
	char buf[16];
	g_debug("signal strength is %d", strength);
	snprintf(buf, 16, "%d", strength);
	edje_object_signal_emit(ui_utils_view_layout_get(VIEW_PTR(view)),
				buf, "gsmSignalChange");
}

static void
_update_counter(const char *name, const char *label_name,
				 int count)
{
	char buf[16];
	snprintf(buf, 16, "%d", count);

	if (count > 0) {
		edje_object_signal_emit
			(ui_utils_view_layout_get(VIEW_PTR(view)), "show", name);
	}
	else {
		edje_object_signal_emit
			(ui_utils_view_layout_get(VIEW_PTR(view)), "hide", name);
	}
	ui_utils_view_text_set(VIEW_PTR(view), label_name, buf);
}

