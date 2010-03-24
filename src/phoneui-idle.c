#include <phoneui/phoneui.h>

#include "phoneui-idle.h"
#include "view/idle-view.h"

void
phoneui_backend_idle_screen_show()
{
	if (!idle_screen_view_is_init()) {
		if (idle_screen_view_init()) {
			return;
		}
	}
	idle_screen_view_show();
}

void
phoenui_backend_idle_screen_deinit()
{
	idle_screen_view_deinit();
}

void
phoneui_backend_idle_screen_hide()
{
	idle_screen_view_hide();
}

void
phoneui_backend_idle_screen_toggle()
{
	if (!idle_screen_view_is_init()) {
		if (idle_screen_view_init()) {
			return;
		}
	}
	idle_screen_view_toggle();
}
