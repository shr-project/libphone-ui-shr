#include <Edje_Edit.h>
#include <Evas.h>
#include <phoneui/phoneui-utils.h>
#include <phoneui/phoneui.h>
#include <glib.h>
#include <time.h>

#include "views.h"
#include "idle-view.h"
#include "util/ui-utils.h"

/*FIXME: no need to pass the screen handle all the time since we only allow
 * one idle screen at a time */

enum IdleScreenCallState {
	INCOMING,
	ACTIVE,
	RELEASED
};

#define RESOURCE_CPU 		"cpuResource"
#define RESOURCE_DISPLAY	"displayResource"
#define RESOURCE_BT		"bluetoothResource"
#define RESOURCE_WIFI		"wifiResource"
#define RESOURCE_GPS		"gpsResource"

struct IdleScreenViewData {
	struct View parent;
	Evas_Object *wallpaper;
};
/* No need to pass the active window everywhere, as we don't allow multiple windows. */
static struct IdleScreenViewData view;

static void _idle_screen_show();
static void _idle_screen_hide();
static void _idle_screen_update_calls(int i);
static void _idle_screen_update_messages(int i);
static void _idle_screen_update_tasks(int i);
static void _idle_screen_update_power(int i);
static void _idle_screen_update_time();
static void _idle_screen_update_gsm(int i, const char *provider);
static void _idle_screen_update_resources(int i, const char *resource_name);
static void _idle_screen_update_call(enum IdleScreenCallState state,
					  const char *name, const char *number);
static void _idle_screen_update_alarm(int i);
static void _idle_screen_update_profile(const char *profile);
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

void
idle_screen_view_update(enum PhoneuiIdleScreenRefresh type)
{
	switch (type) {
	case PHONEUI_IDLE_SCREEN_REFRESH_ALL:
		idle_screen_view_update
			(PHONEUI_IDLE_SCREEN_REFRESH_MISSED_CALLS);
		idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_MESSAGES);
		idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_TASKS);
		idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_POWER);
		idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_CALL);
		idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_GSM);
		idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_RESOURCES);
		idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_ALARM);
		idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_PROFILE);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_MISSED_CALLS:
		_idle_screen_update_calls(3);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_MESSAGES:
		_idle_screen_update_messages(425);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_TASKS:
		_idle_screen_update_tasks(1);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_POWER:
		_idle_screen_update_power(10);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_CALL:
		/*_idle_screen_update_call(INCOMING,
		 * "Mr. Anonymous", "+49123456789");
		 * _idle_screen_update_call(ACTIVE,
		 * "Mr. Anonymous", "+49123456789");
		 * _idle_screen_update_call(RELEASED,
		 * "Mr. Anonymous", "+49123456789");
		 */
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_GSM:
		_idle_screen_update_gsm(23, "Provider");
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_RESOURCES:
		_idle_screen_update_resources(1,RESOURCE_CPU);
		_idle_screen_update_resources(1,RESOURCE_DISPLAY);
		_idle_screen_update_resources(1,RESOURCE_BT);
		_idle_screen_update_resources(1,RESOURCE_WIFI);
		_idle_screen_update_resources(0,RESOURCE_GPS);
		break;

/*	case PHONEUI_IDLE_SCREEN_REFRESH_TIME:
		_idle_screen_update_time();
		break;
*/
	case PHONEUI_IDLE_SCREEN_REFRESH_ALARM:
		_idle_screen_update_alarm(1);
		_idle_screen_update_alarm(0);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_PROFILE:
		_idle_screen_update_profile("Vibrate");
		break;

	}
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

	edje_object_signal_emit(ui_utils_view_window_get(&view.parent),
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
_idle_screen_update_calls(int i)
{
	_idle_screen_update_counter("missedCalls", "missedCallsLabel", i);
}

static void
_idle_screen_update_messages(int i)
{
	_idle_screen_update_counter("unreadMessages",
					 "unreadMessagesLabel", i);
}

static void
_idle_screen_update_tasks(int i)
{

	_idle_screen_update_counter("unfinishedTasks",
					 "unfinishedTasksLabel", i);
}

static void
_idle_screen_update_power(int i)
{
	char buf[16];
	snprintf(buf, 16, "%d", i);

	edje_object_signal_emit(ui_utils_view_layout_get(&view.parent),
				buf, "batteryPowerChange");
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

static void
_idle_screen_update_gsm(int i, const char *provider)
{
	char buf[16];
	snprintf(buf, 16, "%d", i);

	ui_utils_view_text_set(&view.parent, "gsmProvider", provider);
	edje_object_signal_emit(ui_utils_view_layout_get(&view.parent),
				buf, "gsmSignalChange");
}

static void
_idle_screen_update_resources(int i, const char *resource_name)
{
	if (i > 0)
		edje_edit_part_selected_state_set(ui_utils_view_layout_get(&view.parent),
						  resource_name, "active 0.0");
	else
		edje_edit_part_selected_state_set(ui_utils_view_layout_get(&view.parent),
						  resource_name, "default 0.0");
}

static void
_idle_screen_update_call(enum IdleScreenCallState state, const char *name,
			      const char *number)
{
	switch (state) {
	case INCOMING:
		edje_object_signal_emit(ui_utils_view_layout_get(&view.parent), "",
					"activate_incomingCall");
		ui_utils_view_text_set(&view.parent, "incomingCallHeading", "Incoming Call:");
		ui_utils_view_text_set(&view.parent, "incomingCallLine1", name);
		ui_utils_view_text_set(&view.parent, "incomingCallLine2", number);
		break;

	case ACTIVE:
		edje_object_signal_emit(ui_utils_view_layout_get(&view.parent), "",
					"activate_incomingCall");
		ui_utils_view_text_set(&view.parent, "incomingCallHeading", "Active Call:");
		ui_utils_view_text_set(&view.parent, "incomingCallLine1", name);
		ui_utils_view_text_set(&view.parent, "incomingCallLine2", number);
		break;

	case RELEASED:
		edje_object_signal_emit(ui_utils_view_layout_get(&view.parent), "",
					"deactivate_incomingCall");
		ui_utils_view_text_set(&view.parent, "incomingCallHeading", "");
		ui_utils_view_text_set(&view.parent, "incomingCallLine1", "");
		ui_utils_view_text_set(&view.parent, "incomingCallLine2", "");
		break;
	}
}

static void
_idle_screen_update_alarm(int i)
{
	if (i > 0)
		edje_edit_part_selected_state_set(ui_utils_view_layout_get(&view.parent),
						  "alarm", "active 0.0");
	else
		edje_edit_part_selected_state_set(ui_utils_view_layout_get(&view.parent),
						  "alarm", "default 0.0");
}

static void
_idle_screen_update_profile(const char *profile)
{
	ui_utils_view_text_set(&view.parent, "profile", profile);
}
