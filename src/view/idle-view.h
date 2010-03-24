#ifndef _IDLE_VIEW_H
#define _IDLE_VIEW_H
#include <phoneui/phoneui.h>

void idle_screen_view_show();
void idle_screen_view_hide();
void idle_screen_view_toggle();

int idle_screen_view_is_init();
int idle_screen_view_init();
void idle_screen_view_deinit();

void idle_screen_view_update_unfinished_tasks(const int amount);
void idle_screen_view_update_call(enum PhoneuiCallState state,
		const char *name, const char *number);
void idle_screen_view_update_alarm(const int alarm);

#endif

