
#include "phoneui-dialog.h"
#include <glib.h>
#include "phoneui-shr.h"
#include "window.h"
#include "views.h"


void
phoneui_backend_dialog_show(const int type)
{
	struct Window *win = window_new(D_("Information"));

	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(options, "type", GINT_TO_POINTER(type));

	window_init(win);
	window_view_show(win, options, dialog_view_show, dialog_view_hide, NULL);
}

