#include "phonegui-contacts.h"
#include <glib.h>
#include "window.h"
#include "async.h"
#include "views.h"

static struct Window *win = NULL;

static void _show(struct Window *win);
static void _hide(struct Window *win);


void
phonegui_backend_contacts_show()
{
	g_debug("phonegui_backend_contacts_show()");
	if (win == NULL) {
		win = window_new(D_("Contacts"));
		async_trigger(_show, win);
	}
}

void
phonegui_backend_contacts_hide()
{
	async_trigger(_hide, win);
}



typedef
	struct _tmp_pack {
	struct Window *win;
	GHashTable *options;
} tmp_pack;
static void
_show_new(tmp_pack * pack)
{
	window_init(pack->win);
	window_view_show(pack->win, pack->options, contact_show_view_show,
			 contact_show_view_hide);
	free(pack);
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
		g_hash_table_insert(options, "Number", common_utils_new_gvalue_string(number));
		free(number);
	}
#if 0
	g_hash_table_insert(options, "change_callback", frame_list_refresh);
	g_hash_table_insert(options, "change_callback_data", data);
#endif

	struct Window *win = window_new(D_("New Contact"));
	tmp_pack *tmp = malloc(sizeof(*tmp));
	tmp->win = win;
	tmp->options = options;
	async_trigger(_show_new, tmp);
}


static void
_show(struct Window *win)
{
	window_init(win);
	window_view_show(win, NULL, contact_list_view_show,
			 contact_list_view_hide);
}

static void
_hide(struct Window *win)
{
	window_destroy(win, NULL);
}
