#include "phonegui-dialer.h"
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
phonegui_backend_dialer_show()
{
	g_debug("phonegui_backend_dialer_show()");
	if (win) {
		window_show(win);
		return;
	}
	win = window_new(D_("Dialer"));
	window_init(win);
	window_view_show(win, NULL, dialer_view_show, dialer_view_hide, _exit_cb);
}

void
phonegui_backend_dialer_hide()
{
	g_debug("phonegui_backend_dialer_hide()");
	if (win) {
		window_destroy(win, NULL);
		win = NULL;
	}
}


