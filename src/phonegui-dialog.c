
#include "phonegui-dialog.h"
#include <glib.h>
#include <assert.h>
#include "instance.h"
#include "window.h"
#include "views.h"


void
phonegui_backend_dialog_show(int type)
{
	struct Window *win = window_new(D_("Information"));
	instance_manager_add(INSTANCE_DIALOG, type, win);

	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(options, "type", (gpointer) type);

	window_init(win);
	window_view_show(win, options, dialog_view_show, dialog_view_hide);
	elm_run();
	elm_shutdown();
}

void
phonegui_backend_dialog_hide(int type)
{
	struct Window *win = instance_manager_remove(INSTANCE_DIALOG, type);
	assert(win != NULL);

	window_destroy(win, NULL);
}

