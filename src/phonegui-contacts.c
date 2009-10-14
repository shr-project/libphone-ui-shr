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
	//g_debug("phonegui_backend_contacts_hide()");
	//if (win) {
	//	window_destroy(win, NULL);
	//	win = NULL;
	//}
}



