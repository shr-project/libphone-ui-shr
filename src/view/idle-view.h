#ifndef _IDLE_VIEW_H
#define _IDLE_VIEW_H
#include <phoneui/phoneui.h>

void idle_screen_view_show();

void idle_screen_view_hide();

int idle_screen_view_is_init();
int idle_screen_view_init();
void idle_screen_view_deinit();

void idle_screen_view_update(enum PhoneuiIdleScreenRefresh type);
#endif
