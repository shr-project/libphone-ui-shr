#ifndef _IDLE_VIEW_H
#define _IDLE_VIEW_H
#include <phoneui/phoneui.h>

void idle_screen_view_show();
void idle_screen_view_hide();
void idle_screen_view_toggle();

int idle_screen_view_is_init();
int idle_screen_view_init();
void idle_screen_view_deinit();

void idle_screen_view_update_missed_calls(const int amount);
void idle_screen_view_update_unfinished_tasks(const int amount);
void idle_screen_view_update_unread_messages(const int amount);
void idle_screen_view_update_power(const int capacity);
void idle_screen_view_update_call(enum PhoneuiCallState state,
		const char *name, const char *number);
void idle_screen_view_update_signal_strength(const int signal);
void idle_screen_view_update_provider(const char *provider);
void idle_screen_view_update_resource(const char *resource, const int state);
void idle_screen_view_update_alarm(const int alarm);
void idle_screen_view_update_profile(const char *profile);

#endif

