#include <Edje_Edit.h>
#include <Evas.h>
#include <phoneui/phoneui-utils.h>
#include <phoneui/phoneui-info.h>
#include <phoneui/phoneui.h>
#include <glib.h>
#include <time.h>

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

static void _idle_screen_show();
static void _idle_screen_hide();
static void _idle_screen_update_counter(const char *name,
		const char *label_name, int count);
static void _idle_screen_update_time();
static void _idle_destroy_cb(struct View *_view);

void
idle_screen_view_show()
{
	ui_utils_view_show(&view.parent);
}

void
idle_screen_view_hide()
{
	ui_utils_view_hide(&view.parent);
}

int
idle_screen_view_init()
{
	struct Evas_Object *win;
	int ret;

	ret = ui_utils_view_init(&view.parent, ELM_WIN_BASIC, D_("Idle_Screen"),
				NULL, NULL, _idle_destroy_cb);
	if (ret) {
		g_critical("Failed to init idle screen");
		return ret;
	}
		
	ui_utils_view_layout_set(&view.parent, IDLE_SCREEN_THEME,
			  "phoneui/idle_screen/idle_screen");

	edje_object_signal_emit(ui_utils_view_layout_get(&view.parent),
				"clock_init", "");

	win = ui_utils_view_window_get(&view.parent);

	elm_win_fullscreen_set(win, 1);
	view.wallpaper = elm_icon_add(win);
	elm_icon_file_set(view.wallpaper, IDLE_SCREEN_WALLPAPER, NULL);
	ui_utils_view_swallow(&view.parent, "background", view.wallpaper);
	evas_object_show(view.wallpaper);

	edje_object_signal_callback_add(ui_utils_view_layout_get(&view.parent), "unlockScreen",
					"slider", idle_screen_view_hide,
					NULL);
	phoneui_info_trigger();
	return 0;
}

void
idle_screen_view_deinit()
{
	evas_object_del(view.wallpaper);
	ui_utils_view_deinit(&view.parent);
}

int
idle_screen_view_is_init()
{
	return ui_utils_view_is_init(&view.parent);
}

void
idle_screen_view_update_missed_calls(const int amount)
{
	if (idle_screen_view_is_init()) {
		g_debug("--- missed calls: %d", amount);
		_idle_screen_update_counter("missedCalls", "missedCallsLabel", amount);
	}
}

void
idle_screen_view_update_unfinished_tasks(const int amount)
{
	if (idle_screen_view_is_init()) {
		_idle_screen_update_counter("unfinishedTasks",
						 "unfinishedTasksLabel", amount);
	}
}

void
idle_screen_view_update_unread_messages(const int amount)
{
	if (idle_screen_view_is_init()) {
		g_debug("--- unread messages: %d", amount);
		_idle_screen_update_counter("unreadMessages",
						 "unreadMessagesLabel", amount);
	}
}

void
idle_screen_view_update_power(const int capacity)
{
	if (idle_screen_view_is_init()) {
		g_debug("--- capacity: %d", capacity);
		char buf[16];
		snprintf(buf, 16, "%d", capacity);

		edje_object_signal_emit(ui_utils_view_layout_get(&view.parent),
					buf, "batteryPowerChange");
	}
}

void
idle_screen_view_update_call(enum PhoneuiCallState state, const char *name, const char *number)
{
	if (!idle_screen_view_is_init())
		return;

	switch (state) {
	case PHONEUI_CALL_STATE_INCOMING:
		edje_object_signal_emit(ui_utils_view_layout_get(&view.parent), "",
					"activate_incomingCall");
		ui_utils_view_text_set(&view.parent, "incomingCallHeading", "Incoming Call:");
		ui_utils_view_text_set(&view.parent, "incomingCallLine1", name);
		ui_utils_view_text_set(&view.parent, "incomingCallLine2", number);
		break;

	case PHONEUI_CALL_STATE_ACTIVE:
		edje_object_signal_emit(ui_utils_view_layout_get(&view.parent), "",
					"activate_incomingCall");
		ui_utils_view_text_set(&view.parent, "incomingCallHeading", "Active Call:");
		ui_utils_view_text_set(&view.parent, "incomingCallLine1", name);
		ui_utils_view_text_set(&view.parent, "incomingCallLine2", number);
		break;

	case PHONEUI_CALL_STATE_RELEASE:
		edje_object_signal_emit(ui_utils_view_layout_get(&view.parent), "",
					"deactivate_incomingCall");
		ui_utils_view_text_set(&view.parent, "incomingCallHeading", "");
		ui_utils_view_text_set(&view.parent, "incomingCallLine1", "");
		ui_utils_view_text_set(&view.parent, "incomingCallLine2", "");
		break;
	}
}

void
idle_screen_view_update_signal_strength(const int signal)
{
	if (idle_screen_view_is_init()) {
		g_debug("--- signal strength: %d", signal);
		char buf[16];
		snprintf(buf, 16, "%d", signal);

		edje_object_signal_emit(ui_utils_view_layout_get(&view.parent),
					buf, "gsmSignalChange");
	}
}

void
idle_screen_view_update_provider(const char *provider)
{
	if (idle_screen_view_is_init()) {
		g_debug("--- provider: %s", provider);
		ui_utils_view_text_set(&view.parent, "gsmProvider", provider);
	}
}

void
idle_screen_view_update_resource(const char *resource, const int state)
{
	if (idle_screen_view_is_init()) {
		g_debug("--- resource: %s --> %s", resource, state ? "ON" : "OFF");
		if (state) {
			edje_edit_part_selected_state_set
				(ui_utils_view_layout_get(&view.parent),
				 resource, "active 0.0");
		}
		else {
			edje_edit_part_selected_state_set
				(ui_utils_view_layout_get(&view.parent),
				 resource, "default 0.0");
		}
	}
}

void
idle_screen_view_update_alarm(const int alarm)
{
	if (idle_screen_view_is_init()) {
		if (alarm > 0) {
			edje_edit_part_selected_state_set
				(ui_utils_view_layout_get(&view.parent),
				 "alarm", "active 0.0");
		}
		else {
			edje_edit_part_selected_state_set
				(ui_utils_view_layout_get(&view.parent),
				 "alarm", "default 0.0");
		}
	}
}

void
idle_screen_view_update_profile(const char *profile)
{
	if (idle_screen_view_is_init()) {
		g_debug("--- profile: %s", profile);
		ui_utils_view_text_set(&view.parent, "profile", profile);
	}
}


static void
_idle_destroy_cb(struct View *_view)
{
	struct DialerViewData *view = (struct DialerViewData *) _view;
	idle_screen_view_hide();
	
}

static void
_idle_screen_update_counter(const char *name, const char *label_name,
				 int count)
{
	char buf[16];
	snprintf(buf, 16, "%d", count);

	if (count > 0) {
		edje_edit_part_selected_state_set(ui_utils_view_layout_get(&view.parent),
						  name, "active 0.0");
	}
	else {
		edje_edit_part_selected_state_set(ui_utils_view_layout_get(&view.parent),
						  name, "default 0.0");
	}
	ui_utils_view_text_set(&view.parent, label_name, buf);
}

static void
_idle_screen_update_time()
{
	char date_str[16];
	struct timeval tv;
	struct tm ptime;
	char *levelstr;

	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &ptime);
/*FIXME take time format from locale */
	strftime(date_str, 14, "%d.%m.%Y", &ptime);

	ui_utils_view_text_set(&view.parent, "date", date_str);
}

