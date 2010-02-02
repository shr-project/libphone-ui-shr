
#include <glib.h>
#include <phoneui/phoneui-utils.h>
#include "phoneui-contacts.h"
#include "util/ui-utils.h"
#include "view/contact-view.h"

// TODO: remove when the list is converted too :P
#include "util/window.h"
#include "view/views.h"


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
_contact_get_cb(GHashTable *content, gpointer data)
{
	char *path = (char *)data;

	if (!content) {
		g_warning("Failed aquiring data for contact %s", path);
		// TODO: show some message dialog showing it did not work
		free (path);
		return;
	}

	if (contact_view_init(path, content))
		return;
	contact_view_show(path);
}


void
phoneui_backend_contacts_contact_show(const char *contact_path)
{
	g_debug("showing contact %s", contact_path);
	if (!contact_view_is_init(contact_path)) {
		phoneui_utils_contact_get(contact_path, _contact_get_cb,
				  strdup(contact_path));
		return;
	}
	contact_view_show(contact_path);
}


void
phoneui_backend_contacts_contact_new(GHashTable *options)
{
	g_debug("phoneui_backend_contacts_contact_new()");
	if (!contact_view_is_init("")) {
		if (contact_view_init(strdup(""), options)) {
			return;
		}
	}
	contact_view_show("");
}


void
phoneui_backend_contacts_contact_edit(const char *path)
{
	phoneui_backend_contacts_contact_show(path);
}

