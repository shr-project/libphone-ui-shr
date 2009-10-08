#include "phonegui-ussd.h"
#include <glib.h>
#include "window.h"
#include "views.h"


static struct Window *win_ussd = NULL;



static void
  _show(GHashTable * options);

static void
  _hide();

static void
  _reset();



void
phonegui_backend_ussd_show(int mode, const char *message)
{
	g_debug("phonegui_backend_ussd_show(mode=%d, message=%s)", mode,
		message);
	if (win_ussd) {
		window_show(win_ussd);
		return;
	}
	win_ussd = window_new(D_("Service Data"));

	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(options, "mode", GINT_TO_POINTER(mode));
	g_hash_table_insert(options, "message", g_strdup((char *) message));	/* we lose the const here */
	g_hash_table_insert(options, "callback_close", _reset);
	window_init(win_ussd);
	window_view_show(win_ussd, options, ussd_view_show, ussd_view_hide);
	elm_run();
}

void
phonegui_backend_ussd_hide()
{
	g_debug("phonegui_backend_ussd_hide()");
	if (win_ussd) {
		window_destroy(win_ussd, NULL);
		win_ussd = NULL;
	}
}




/* --- EFL thread functions * ----------------------------------------------- */



static void
_reset(void)
{
	g_debug("_reset()");
	win_ussd = NULL;
}
