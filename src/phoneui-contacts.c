#include "phoneui-contacts.h"
#include <glib.h>
#include "window.h"
#include "views.h"


static struct Window *list = NULL;

static void
_exit_cb()
{
	list = NULL;
}

void
phoneui_backend_contacts_show()
{
	if (list) {
		window_show(list);
		return;
	}
	struct Window *list = window_new(D_("Contacts"));
	window_init(list);
	window_view_show(list, NULL, contact_list_view_show,
			 contact_list_view_hide, _exit_cb);
}

void
phoneui_backend_contacts_refresh()
{
	g_debug("phoneui_backend_contacts_refresh()");
	if (list) {
		g_debug("refreshing list of contacts");
		contact_list_view_refresh(list);
	}
}

static void
_contact_get_cb(GError *error, GHashTable *content, gpointer data)
{
	if (error) {
		g_warning("could not get contact content");
		g_error_free(error);
		return;
	}
	struct Window *win = window_new(D_("Contact"));
	window_init(win);
	window_view_show(win, content, contact_show_view_show,
			contact_show_view_hide, NULL);
}


void
phoneuid_backend_contacts_contact_show(const char *contact_path)
{
	phoneui_contact_get(contact_path, _contact_get_cb, NULL);
}


void
phoneui_backend_contacts_new_show(const char *name, const char *number)
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



