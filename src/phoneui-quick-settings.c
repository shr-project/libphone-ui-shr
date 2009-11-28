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
		return;
	}
	win = window_new(D_("Dialer"));
	window_init(win);
	window_view_show(win, NULL, quick_settings_view_show, quick_settings_view_hide, _exit_cb);
}

