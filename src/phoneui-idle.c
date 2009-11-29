#include <glib.h>
#include <phoneui/phoneui.h>
#include <Elementary.h>

#include "phoneui-idle.h"
#include "window.h"
#include "views.h"
#include "view/idle-view.h"

static struct Window *win = NULL;

static void
_exit_cb()
{
	/*FIXME: free data */
	win = NULL;
}

void
phoneui_backend_idle_screen_show()
{
	if (win) {
		g_debug("Idle_screen: Showing an existing screen");
		window_show(win);
	}
	else {
		g_debug("Idle_screen: Showing and creating idle screen");
		win = window_new(D_("Idle_Screen"));
		window_init(win);
		window_view_show(win, NULL, idle_screen_view_show,
				 idle_screen_view_hide, _exit_cb);
	}
}

void
phoneui_backend_idle_screen_hide()
{
	if (win) {
		g_debug("Idle_screen: Hiding.");
		window_view_hide(win, NULL);
	}
	else {
		g_critical("Idle_screen: Tried to hide a non existing screen");
	}
}

void
phoneui_backend_idle_screen_update(enum PhoneuiIdleScreenRefresh type)
{
	g_debug("phoneui_backend_idle_screen_update()");
	if (win) {
		idle_screen_view_update(type);
	}
}
