#ifndef _PHONEGUI_IDLE_SCREEN_H
#define _PHONEGUI_IDLE_SCREEN_H

#include <phoneui/phoneui.h>
#include "util/window.h"

void phoneui_backend_idle_screen_show();
void phoneui_backend_idle_screen_hide();
void phoneui_backend_idle_screen_update(enum PhoneuiIdleScreenRefresh type);

#endif
