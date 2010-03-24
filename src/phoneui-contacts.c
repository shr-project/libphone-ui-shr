
#include <glib.h>
#include <phoneui/phoneui-utils.h>
#include "phoneui-contacts.h"
#include "view/contact-list-view.h"
#include "view/contact-view.h"
#include "view/views.h"

void
phoneui_backend_contacts_show()
{
	if (!contact_list_view_is_init()) {
		if (contact_list_view_init()) {
			return;
		}
	}
	contact_list_view_show();
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

void
phoneui_backend_contacts_deinit()
{
	/*FIXME: also clean all the contact screens*/
	contact_list_view_deinit();
}

