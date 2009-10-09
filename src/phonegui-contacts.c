#include "phonegui-contacts.h"
#include <glib.h>
#include "window.h"
#include "views.h"

static struct Window *win = NULL;


void
phonegui_backend_contacts_show()
{
	g_debug("phonegui_backend_contacts_show()");
	if (win) {
		window_show(win);
		return;
	}
	win = window_new(D_("Contacts"));
	window_init(win);
	window_view_show(win, NULL, contact_list_view_show,
			 contact_list_view_hide);
	elm_run();
	elm_shutdown();
}

void
phonegui_backend_contacts_hide()
{
	if (win) {
		window_destroy(win, NULL);
		win = NULL;
	}
}



