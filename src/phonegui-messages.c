#include "phonegui-messages.h"
#include <glib.h>
#include "window.h"
#include "views.h"

static struct Window *win = NULL;


void
phonegui_backend_messages_show(int argc, char **argv)
{
	g_debug("phonegui_backend_messages_show()");
	if (win) {
		window_show(win);
		return;
	}

	win = window_new(D_("Messages"));
	window_init(win);
	window_view_show(win, NULL, message_list_view_show,
			 message_list_view_hide);
	elm_run();
}

void
phonegui_backend_messages_hide()
{
	g_debug("phonegui_backend_messages_hide()");
	if (win) {
		window_destroy(win, NULL);
		win = NULL;
	}
}


