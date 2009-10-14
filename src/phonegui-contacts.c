#include "phonegui-contacts.h"
#include <glib.h>
#include "window.h"
#include "views.h"



void
phonegui_backend_contacts_show()
{
	struct Window *win = window_new(D_("Contacts"));
	window_init(win);
	window_view_show(win, NULL, contact_list_view_show,
			 contact_list_view_hide, NULL);
}

void
phonegui_backend_contacts_hide()
{
}

void
phonegui_backend_contacts_new_show(const char *name, const char *number)
{
	GHashTable *options = g_hash_table_new_full(g_str_hash, g_str_equal,
							NULL, free);
	if (name) {
		g_hash_table_insert(options, "Name", common_utils_new_gvalue_string(name));
	}
	if (number) {
		/* FIXME: BAD STRDUP!!! */
		number = common_utils_new_with_prefix(number, "tel:");
		g_hash_table_insert(options, "Phone",
				common_utils_new_gvalue_string(number));
		free(number);
	}
#if 0
	g_hash_table_insert(options, "change_callback", frame_list_refresh);
	g_hash_table_insert(options, "change_callback_data", data);
#endif

	struct Window *win = window_new(D_("New Contact"));
	window_init(win);
	window_view_show(win, options, contact_show_view_show,
			 contact_show_view_hide, NULL);
}



