#ifndef _PHONEUI_IDLE_SCREEN_H
#define _PHONEUI_IDLE_SCREEN_H

#include <phoneui/phoneui.h>

void phoneui_backend_idle_screen_show();
void phoneui_backend_idle_screen_hide();
void phoneui_backend_idle_screen_toggle();
void phoneui_backend_idle_screen_update_missed_calls(const int amount);
void phoneui_backend_idle_screen_update_unfinished_tasks(const int amount);
void phoneui_backend_idle_screen_update_unread_messages(const int amount);
void phoneui_backend_idle_screen_update_power(const int capacity);
void phoneui_backend_idle_screen_update_call(enum PhoneuiCallState state,
		const char *name, const char *number);
void phoneui_backend_idle_screen_update_signal_strength(const int signal);
void phoneui_backend_idle_screen_update_provider(const char *provider);
void phoneui_backend_idle_screen_update_resource(const char *resource,
		const int state);
void phoneui_backend_idle_screen_update_alarm(const int alarm);
void phoneui_backend_idle_screen_update_profile(const char *profile);

#endif
