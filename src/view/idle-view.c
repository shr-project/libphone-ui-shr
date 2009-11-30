#include <Edje_Edit.h>
#include <phoneui/phoneui-utils.h>
#include <phoneui/phoneui.h>
#include <glib.h>
#include <time.h>

#include "views.h"
#include "idle-view.h"

/*FIXME: no need to pass the screen handle all the time since we only allow
 * one idle screen at a time */

enum IdleScreenCallState {
	INCOMING,
	ACTIVE,
	RELEASED
};

enum Resources {
	CPU,
	Display,
	Bluetooth,
	WiFi,
	GPS
};

struct IdleScreenViewData {
	struct Window *win;
	Evas_Object *wallpaper;
};
/* No need to pass the active window everywhere, as we don't allow multiple windows. */
static struct IdleScreenViewData data;

static void frame_idle_screen_show();
static void frame_idle_screen_hide();
static void frame_idle_screen_update_calls(int i);
static void frame_idle_screen_update_messages(int i);
static void frame_idle_screen_update_tasks(int i);
static void frame_idle_screen_update_power(int i);
static void frame_idle_screen_update_time();
static void frame_idle_screen_update_gsm(int i, const char *provider);
static void frame_idle_screen_update_resources(int i, enum Resources resource);
static void frame_idle_screen_update_call(enum IdleScreenCallState state,
					  const char *name, const char *number);
static void frame_idle_screen_update_alarm(int i);
static void frame_idle_screen_update_profile(const char *profile);

/*Hackish prototypes, just to make broken window.h happy */
void *
idle_screen_view_show(struct Window *win, GHashTable * options)
{
	g_debug("idle_screen_view_show()");
	data.win = win;

	window_frame_show(win, &data, frame_idle_screen_show,
			  frame_idle_screen_hide);
	window_show(win);
	return;
}

void
idle_screen_view_hide(struct Window *win)
{
	/* no need to do anything here */
}

void
idle_screen_view_update(enum PhoneuiIdleScreenRefresh type)
{
	g_debug("idle_screen_view_update()");
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
		idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_TIME);
		idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_ALARM);
		idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_PROFILE);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_MISSED_CALLS:
		//frame_idle_screen_update_calls(3);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_MESSAGES:
		//frame_idle_screen_update_messages(425);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_TASKS:
		//frame_idle_screen_update_tasks(1);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_POWER:
		//frame_idle_screen_update_power(10);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_CALL:
		/*frame_idle_screen_update_call(INCOMING,
		 * "Mr. Anonymous", "+49123456789");
		 * frame_idle_screen_update_call(ACTIVE,
		 * "Mr. Anonymous", "+49123456789");
		 * frame_idle_screen_update_call(RELEASED,
		 * "Mr. Anonymous", "+49123456789");
		 */
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_GSM:
		//frame_idle_screen_update_gsm(23, "Provider");
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_RESOURCES:
		/*frame_idle_screen_update_resources(1,CPU);
		 * frame_idle_screen_update_resources(1,Display);
		 * frame_idle_screen_update_resources(1,Bluetooth);
		 * frame_idle_screen_update_resources(1,WiFi);
		 * frame_idle_screen_update_resources(1,GPS);
		 *
		 * frame_idle_screen_update_resources(0,WiFi);
		 * frame_idle_screen_update_resources(0,GPS);
		 */
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_TIME:
		//frame_idle_screen_update_time();
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_ALARM:
		//frame_idle_screen_update_alarm(1);
		//frame_idle_screen_update_alarm(0);
		break;

/* TODO: Implement this in libphone-ui */
	case PHONEUI_IDLE_SCREEN_REFRESH_PROFILE:
		//frame_idle_screen_update_profile("Vibrate");
		break;

	}
}

/*
 * frame_idle_screen
 */

static void
frame_idle_screen_show()
{
	struct Window *win = data.win;
	window_layout_set(win, IDLE_SCREEN_THEME,
			  "phoneui/idle_screen/idle_screen");

	data.wallpaper = elm_icon_add(window_evas_object_get(win));
	elm_icon_file_set(data.wallpaper, IDLE_SCREEN_WALLPAPER, NULL);
	window_swallow(win, "background", data.wallpaper);
	evas_object_show(data.wallpaper);

	edje_object_signal_callback_add(window_layout_get(win), "unlockScreen",
					"slider", phoneui_idle_screen_hide,
					&data);
}

static void
frame_idle_screen_hide()
{
	/* No need for this: evas_object_del(data.wallpaper); */
}


static void
frame_idle_screen_update_counter(const char *name, const char *label_name,
				 int count)
{
	struct Window *win = data.win;

	char buf[16];
	snprintf(buf, 16, "%d", count);

	if (count > 0) {
		edje_edit_part_selected_state_set(window_layout_get(win),
						  name, "active 0.0");
		window_text_set(win, label_name, buf);
	}
	else {
		edje_edit_part_selected_state_set(window_layout_get(data.win),
						  name, "default 0.0");
		window_text_set(data.win, label_name, buf);
	}
}

static void
frame_idle_screen_update_calls(int i)
{
	frame_idle_screen_update_counter("missedCalls", "missedCallsLabel", i);
}

static void
frame_idle_screen_update_messages(int i)
{
	frame_idle_screen_update_counter("unreadMessages",
					 "unreadMessagesLabel", i);
}

static void
frame_idle_screen_update_tasks(int i)
{

	frame_idle_screen_update_counter("unfinishedTasks",
					 "unfinishedTasksLabel", i);
}

static void
frame_idle_screen_update_power(int i)
{
	g_debug("frame_idle_screen_update_power()");
	struct Window *win = data.win;

	char buf[16];
	snprintf(buf, 16, "%d", i);

	edje_object_signal_emit(window_layout_get(win),
				buf, "batteryPowerChange");
}

static void
frame_idle_screen_update_time()
{
	g_debug("frame_idle_screen_update_time()");
	struct Window *win = data.win;
	char date_str[16];
	struct timeval tv;
	struct tm ptime;
	char *levelstr;

	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &ptime);
/*FIXME take time format from locale */
	strftime(date_str, 14, "%d.%m.%Y", &ptime);

	window_text_set(win, "date", date_str);

	strftime(date_str, 14, "%H:%M", &ptime);
	window_text_set(win, "time", date_str);
}

static void
frame_idle_screen_update_gsm(int i, const char *provider)
{
	g_debug("frame_idle_screen_update_gsm()");
	struct Window *win = data.win;

	char buf[16];
	snprintf(buf, 16, "%d", i);

	window_text_set(win, "gsmProvider", provider);
	edje_object_signal_emit(window_layout_get(win), buf, "gsmSignalChange");
}

static void
frame_idle_screen_update_resources(int i, enum Resources resource)
{
	struct Window *win = data.win;

	const char *resource_name;

	switch (resource) {
	case CPU:
		resource_name = "cpuResource";
		break;

	case Display:
		resource_name = "displayResource";
		break;

	case Bluetooth:
		resource_name = "bluetoothResource";
		break;

	case WiFi:
		resource_name = "wifiResource";
		break;

	case GPS:
		resource_name = "gpsResource";
		break;

	default:
		g_critical("no such resource!");
		return;
		break;
	}

	if (i > 0)
		edje_edit_part_selected_state_set(window_layout_get(data.win),
						  resource_name, "active 0.0");
	else
		edje_edit_part_selected_state_set(window_layout_get(data.win),
						  resource_name, "default 0.0");
}

static void
frame_idle_screen_update_call(enum IdleScreenCallState state, const char *name,
			      const char *number)
{
	struct Window *win = data.win;

	switch (state) {
	case INCOMING:
		edje_object_signal_emit(window_layout_get(win), "",
					"activate_incomingCall");
		window_text_set(win, "incomingCallHeading", "Incoming Call:");
		window_text_set(win, "incomingCallLine1", name);
		window_text_set(win, "incomingCallLine2", number);
		break;

	case ACTIVE:
		edje_object_signal_emit(window_layout_get(win), "",
					"activate_incomingCall");
		window_text_set(win, "incomingCallHeading", "Active Call:");
		window_text_set(win, "incomingCallLine1", name);
		window_text_set(win, "incomingCallLine2", number);
		break;

	case RELEASED:
		edje_object_signal_emit(window_layout_get(win), "",
					"deactivate_incomingCall");
		window_text_set(win, "incomingCallHeading", "");
		window_text_set(win, "incomingCallLine1", "");
		window_text_set(win, "incomingCallLine2", "");
		break;
	}
}

static void
frame_idle_screen_update_alarm(int i)
{
	struct Window *win = data.win;

	if (i > 0)
		edje_edit_part_selected_state_set(window_layout_get(data.win),
						  "alarm", "active 0.0");
	else
		edje_edit_part_selected_state_set(window_layout_get(data.win),
						  "alarm", "default 0.0");
}

static void
frame_idle_screen_update_profile(const char *profile)
{
	struct Window *win = data.win;

	window_text_set(win, "profile", profile);
}
