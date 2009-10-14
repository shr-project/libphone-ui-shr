#include "phonegui-contact.h"
#include <glib.h>
#include "window.h"
#include "views.h"

static struct Window *win = NULL;



void
phonegui_backend_contact_new_show(const char *name, const char *number)
{
	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(options, "name", name);
	g_hash_table_insert(options, "number", number);
#if 0
	g_hash_table_insert(options, "change_callback", frame_list_refresh);
	g_hash_table_insert(options, "change_callback_data", data);
#endif

	struct Window *win = window_new(D_("New Contact"));
	window_init(win);
	window_view_show(win, options, contact_show_view_show,
			 contact_show_view_hide, NULL);
}

void
phonegui_backend_contact_hide()
{
	window_destroy(win, NULL);
}

