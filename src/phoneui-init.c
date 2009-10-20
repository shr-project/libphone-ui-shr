#include "phoneui-init.h"
#include <Ecore_Evas.h>
#include <Ecore.h>
#include <Edje.h>
#include <Etk.h>
#include <Elementary.h>
#include <glib.h>

int phoneui_argc;
char **phoneui_argv;

static gboolean
_idle_foo(void *foo)
{
	return (TRUE);
}


void
phoneui_backend_init(int argc, char **argv, int (*idle_cb) (void *))
{
	// Initialize gettext
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	phoneui_argc = argc;
	phoneui_argv = argv;

	g_type_init();

}

void phoneui_backend_loop()
{
	/* Initialize glib main loop */
	GMainLoop *mainloop = g_main_loop_new(NULL, FALSE);

	// Initializations
	elm_init(phoneui_argc, phoneui_argv);
	g_debug("Initiated elementary");

	if (!ecore_main_loop_glib_integrate()) {
		g_fatal("failed integrating the glib mainloop");
	}

	elm_run();
	elm_shutdown();
}