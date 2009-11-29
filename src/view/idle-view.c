#include "views.h"
#include <Edje_Edit.h>
#include <phoneui/phoneui-utils.h>
#include <phoneui/phoneui.h>

struct IdleScreenViewData {
	struct Window *win;
	Evas_Object *wallpaper;
};

enum Resources {
	CPU,
	Display,
	Bluetooth,
	WiFi,
	GPS
};

enum CallStates {
	INCOMING,
	ACTIVE,
	RELEASED
};

static void frame_idle_screen_show(void *_data);
static void frame_idle_screen_hide(void *_data);
static void frame_idle_screen_update_calls(int i, void *_data);
static void frame_idle_screen_update_messages(int i, void *_data);
static void frame_idle_screen_update_tasks(int i, void *_data);
static void frame_idle_screen_update_power(int i, void *_data);
static void frame_idle_screen_update_time(void *_data);
static void frame_idle_screen_update_gsm(int i, char provider[25], void *_data);
static void frame_idle_screen_update_resources(int i, enum Resources resource, void *_data);
static void frame_idle_screen_update_call(enum CallStates state, char *name, char *number,
			      void *_data);
static void frame_idle_screen_update_alarm(int i, void *_data);
static void frame_idle_screen_update_profile(char *profile, void *_data);


void *
idle_screen_view_show(struct Window *win, void *_options)
{
	g_debug("idle_screen_view_show()");
	GHashTable *options = (GHashTable *) _options;
	struct IdleScreenViewData *data =
		calloc(1, sizeof(struct IdleScreenViewData));
	data->win = win;

	window_frame_show(win, data, frame_idle_screen_show,
			  frame_idle_screen_hide);
	window_show(win);
	return data;
}

void
idle_screen_view_hide(void *_data)
{
	g_debug("idle_screen_view_hide()");
	free(_data);
}

void
idle_screen_view_update(enum PhoneuiIdleScreenRefresh type, struct Window *win)
{
	g_debug("idle_screen_view_update()");
	switch (type) {
	case PHONEUI_IDLE_SCREEN_REFRESH_ALL:
		idle_screen_view_update
			(PHONEUI_IDLE_SCREEN_REFRESH_MISSED_CALLS, win);
		idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_MESSAGES,
					win);
		idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_TASKS, win);
		idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_POWER, win);
		idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_CALL, win);
		idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_GSM, win);
		idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_RESOURCES,
					win);
		idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_TIME, win);
		idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_ALARM, win);
		//idle_screen_view_update(PHONEUI_IDLE_SCREEN_REFRESH_PROFILE, win);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_MISSED_CALLS:
		//frame_idle_screen_update_calls(3, win->view_data);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_MESSAGES:
		//frame_idle_screen_update_messages(425, win->view_data);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_TASKS:
		//frame_idle_screen_update_tasks(1, win->view_data);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_POWER:
		//frame_idle_screen_update_power(10, win->view_data);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_CALL:
		//frame_idle_screen_update_call(INCOMING,\
		"Mr. Anonymous", "+49123456789", win->view_data);
		//frame_idle_screen_update_call(ACTIVE,\
		"Mr. Anonymous", "+49123456789", win->view_data);
		//frame_idle_screen_update_call(RELEASED,\
		"Mr. Anonymous", "+49123456789", win->view_data);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_GSM:
		//frame_idle_screen_update_gsm(23, "Provider", win->view_data);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_RESOURCES:
		//frame_idle_screen_update_resources(1,CPU,win->view_data);
		//frame_idle_screen_update_resources(1,Display,win->view_data);
		//frame_idle_screen_update_resources(1,Bluetooth,win->view_data);
		//frame_idle_screen_update_resources(1,WiFi,win->view_data);
		//frame_idle_screen_update_resources(1,GPS,win->view_data);

		//frame_idle_screen_update_resources(0,WiFi,win->view_data);
		//frame_idle_screen_update_resources(0,GPS,win->view_data);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_TIME:
		//frame_idle_screen_update_time(win->view_data);
		break;

	case PHONEUI_IDLE_SCREEN_REFRESH_ALARM:
		//frame_idle_screen_update_alarm(1,win->view_data);
		//frame_idle_screen_update_alarm(0,win->view_data);
		break;

/*
	TODO: Implement this in libphone-ui
	case PHONEUI_IDLE_SCREEN_REFRESH_PROFILE:
		//frame_idle_screen_update_profile("Vibrate",win->view_data);
		break;
*/
	}
}

//
// frame_idle_screen
//

static void
  frame_idle_screen_show(void *_data) {
	struct IdleScreenViewData *data = (struct IdleScreenViewData *) _data;
	struct Window *win = data->win;
	window_layout_set(win, IDLE_SCREEN_THEME,
			  "phoneui/idle_screen/idle_screen");

	data->wallpaper = elm_icon_add(window_evas_object_get(win));
	elm_icon_file_set(data->wallpaper, IDLE_SCREEN_WALLPAPER, NULL);
	window_swallow(win, "background", data->wallpaper);
	evas_object_show(data->wallpaper);

	edje_object_signal_callback_add(window_layout_get(win), "unlockScreen",
					"slider", phoneui_idle_screen_hide, data);
}

static void
  frame_idle_screen_hide(void *_data) {
	struct IdleScreenViewData *data = (struct IdleScreenViewData *) _data;
	struct Window *win = data->win;

	evas_object_del(data->wallpaper);
}

static void
  frame_idle_screen_update_calls(int i, void *_data) {
	g_debug("frame_idle_screen_update_calls()");
	struct IdleScreenViewData *data = (struct IdleScreenViewData *) _data;
	struct Window *win = data->win;

	char buf[16];
	snprintf(buf, 16, "%d", i);

	if (i > 0) {
		edje_edit_part_selected_state_set(window_layout_get(win),
						  "missedCalls", "active 0.0");
		window_text_set(win, "missedCallsLabel", buf);
	}
	else {
		edje_edit_part_selected_state_set(window_layout_get(data->win),
						  "missedCalls", "default 0.0");
		window_text_set(data->win, "missedCallsLabel", buf);
	}
}

static void
  frame_idle_screen_update_messages(int i, void *_data) {
	g_debug("frame_idle_screen_update_messages()");
	struct IdleScreenViewData *data = (struct IdleScreenViewData *) _data;
	struct Window *win = data->win;

	char buf[16];
	snprintf(buf, 16, "%d", i);

	if (i > 0) {
		edje_edit_part_selected_state_set(window_layout_get(win),
						  "unreadMessages",
						  "active 0.0");
		window_text_set(win, "unreadMessagesLabel", buf);
	}
	else {
		edje_edit_part_selected_state_set(window_layout_get(data->win),
						  "unreadMessages",
						  "default 0.0");
		window_text_set(data->win, "unreadMessagesLabel", buf);
	}
}

static void
  frame_idle_screen_update_tasks(int i, void *_data) {
	g_debug("frame_idle_screen_update_tasks()");
	struct IdleScreenViewData *data = (struct IdleScreenViewData *) _data;
	struct Window *win = data->win;

	char buf[16];
	snprintf(buf, 16, "%d", i);

	if (i > 0) {
		edje_edit_part_selected_state_set(window_layout_get(win),
						  "unfinishedTasks",
						  "active 0.0");
		window_text_set(win, "unfinishedTasksLabel", buf);
	}
	else {
		edje_edit_part_selected_state_set(window_layout_get(data->win),
						  "unfinishedTasks",
						  "default 0.0");
		window_text_set(data->win, "unfinishedTasksLabel", buf);
	}
}

static void
  frame_idle_screen_update_power(int i, void *_data) {
	g_debug("frame_idle_screen_update_power()");
	struct IdleScreenViewData *data = (struct IdleScreenViewData *) _data;
	struct Window *win = data->win;

	char buf[16];
	snprintf(buf, 16, "%d", i);

	edje_object_signal_emit(window_layout_get(win),
				buf, "batteryPowerChange");
}

static void
  frame_idle_screen_update_time(void *_data) {
	g_debug("frame_idle_screen_update_time()");
	struct IdleScreenViewData *data = (struct IdleScreenViewData *) _data;
	struct Window *win = data->win;

	struct tm *tmp;
	time_t t;

	t = time(NULL);
	tmp = localtime(&t);

	char buf[16];
	snprintf(buf, 16, "%02d.%02d.%04d",
		 tmp->tm_mday, tmp->tm_mon + 1, tmp->tm_year + 1900);
	window_text_set(win, "date", buf);

	char buf2[16];
	snprintf(buf2, 16, "%02d:%02d", tmp->tm_hour, tmp->tm_min);
	window_text_set(win, "time", buf2);
}

static void
  frame_idle_screen_update_gsm(int i, char provider[25], void *_data) {
	g_debug("frame_idle_screen_update_gsm()");
	struct IdleScreenViewData *data = (struct IdleScreenViewData *) _data;
	struct Window *win = data->win;

	char buf[16];
	snprintf(buf, 16, "%d", i);

	window_text_set(win, "gsmProvider", provider);
	edje_object_signal_emit(window_layout_get(win), buf, "gsmSignalChange");
}

static void

 
	frame_idle_screen_update_resources(int i, enum Resources resource,
					   void *_data) {
	g_debug("frame_idle_screen_update_resources()");
	struct IdleScreenViewData *data = (struct IdleScreenViewData *) _data;
	struct Window *win = data->win;

	char *resource_name;

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
		g_error("no such resource!");
		break;
	}

	if (i > 0)
		edje_edit_part_selected_state_set(window_layout_get(data->win),
						  resource_name, "active 0.0");
	else
		edje_edit_part_selected_state_set(window_layout_get(data->win),
						  resource_name, "default 0.0");
}

static void

 
	frame_idle_screen_update_call(enum CallStates state, char *name,
				      char *number, void *_data) {
	g_debug("frame_idle_screen_update_call()");
	struct IdleScreenViewData *data = (struct IdleScreenViewData *) _data;
	struct Window *win = data->win;

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
  frame_idle_screen_update_alarm(int i, void *_data) {
	g_debug("frame_idle_screen_update_alarm()");
	struct IdleScreenViewData *data = (struct IdleScreenViewData *) _data;
	struct Window *win = data->win;

	if (i > 0)
		edje_edit_part_selected_state_set(window_layout_get(data->win),
						  "alarm", "active 0.0");
	else
		edje_edit_part_selected_state_set(window_layout_get(data->win),
						  "alarm", "default 0.0");
}

static void
  frame_idle_screen_update_profile(char *profile, void *_data) {
	g_debug("frame_idle_screen_update_profile()");
	struct IdleScreenViewData *data = (struct IdleScreenViewData *) _data;
	struct Window *win = data->win;

	window_text_set(win, "profile", profile);
}
