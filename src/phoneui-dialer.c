#include "phoneui-dialer.h"
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
phoneui_backend_dialer_show()
{
	g_debug("phoneui_backend_dialer_show()");
	if (win) {
		window_show(win);
		return;
	}
	win = window_new(D_("Dialer"));
	window_init(win);
	window_view_show(win, NULL, dialer_view_show, dialer_view_hide, _exit_cb);
}

