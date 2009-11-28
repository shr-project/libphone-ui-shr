#include "phoneui-quick-settings.h"
#include <glib.h>
#include "window.h"
#include "views.h"

static struct Window *win = NULL;

static void
_exit_cb()
{
	win = NULL;
}

void
phoneui_backend_quick_settings_show()
{
	g_debug("phoneui_backend_quick_settings_show()");
	if (win) {
		window_show(win);
	} else {
		win = window_new(D_("Quick Settings"));
		window_init(win);
		window_view_show(win, NULL, quick_settings_view_show, quick_settings_view_hide, _exit_cb);
	}
}


void
phoneui_backend_quick_settings_hide()
{
	g_debug("phoneui_backend_quick_settings_hide()");
	if (win != NULL) {
		window_destroy(win, NULL);
		win = NULL;
	} else {
		win = window_new(D_("Quick Settings"));
		window_init(win);
		window_view_show(win, NULL, quick_settings_view_show, quick_settings_view_hide, _exit_cb);
	}
}

