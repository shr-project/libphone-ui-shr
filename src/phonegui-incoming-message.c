#include "phonegui-incoming-message.h"
#include <glib.h>
#include <assert.h>
#include "instance.h"
#include "window.h"
#include "views.h"


void
phonegui_backend_message_show(const int id)
{
	g_debug("phonegui_backend_message_show(id=%d)", id);

	struct Window *win = window_new(D_("New Message"));
	instance_manager_add(INSTANCE_INCOMING_MESSAGE, id, win);

	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(options, "id", GINT_TO_POINTER(id));
	window_init(win);
	window_view_show(win, options, message_show_view_show,
			 message_show_view_hide, NULL);
}

void
phonegui_backend_message_hide(int id)
{
	g_debug("phonegui_backend_message_hide()");
	struct Window *win =
		instance_manager_remove(INSTANCE_INCOMING_MESSAGE, id);
	window_destroy(win, NULL);
}


