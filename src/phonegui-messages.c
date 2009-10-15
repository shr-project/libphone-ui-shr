#include "phonegui-messages.h"
#include <glib.h>
#include "window.h"
#include "views.h"

static struct Window *message_list = NULL;

static void
_exit_cb()
{
	message_list = NULL;
}

void
phonegui_backend_messages_show(int argc, char **argv)
{
	g_debug("phonegui_backend_messages_show()");
	if (message_list) {
		window_show(message_list);
		return;
	}

	message_list = window_new(D_("Messages"));
	window_init(message_list);
	window_view_show(message_list, NULL, message_list_view_show,
			 message_list_view_hide, _exit_cb);
}

void
phonegui_backend_message_show(const char *path)
{
	struct Window *win = window_new(D_("Message"));
	window_init(win);
	window_view_show(win, NULL, message_show_view_show,
			message_show_view_hide, NULL);
}

