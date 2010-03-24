#include "phoneui-ussd.h"
#include <glib.h>
#include "phoneui-shr.h"
#include "window.h"
#include "views.h"


void
phoneui_backend_ussd_show(int mode, const char *message)
{
	g_debug("phoneui_backend_ussd_show(mode=%d, message=%s)", mode,
		message);
	struct Window *win = window_new(D_("Service Data"));

	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(options, "mode", GINT_TO_POINTER(mode));
	g_hash_table_insert(options, "message", g_strdup((char *) message));	/* we lose the const here */
	window_init(win);
	window_view_show(win, options, ussd_view_show, ussd_view_hide, NULL);
}

