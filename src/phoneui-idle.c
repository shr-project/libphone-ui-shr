#include "phoneui-idle.h"
#include <glib.h>
#include <phoneui/phoneui.h>
#include "window.h"
#include "views.h"
#include <Elementary.h>

static struct Window *win = NULL;

static void
_exit_cb()
{
	win = NULL;
}

void
phoneui_backend_idle_screen_show()
{
	g_debug("phoneui_backend_idle_screen_show()");
	if (win) {
		window_show(win);
		return;
	}
	win = window_new(D_("Idle_Screen"));
	window_init(win);
	window_view_show(win, NULL, idle_screen_view_show,
			 idle_screen_view_hide, _exit_cb);
}

void
phoneui_backend_idle_screen_hide()
{
	g_debug("phoneui_backend_idle_screen_hide()");
	if (win != NULL) {
		window_destroy(win, NULL);
		win = NULL;
	}
}

void
phoneui_backend_idle_screen_update(enum PhoneuiIdleScreenRefresh type)
{
	g_debug("phoneui_backend_idle_screen_update()");
	idle_screen_view_update(type, win);
}
