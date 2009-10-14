#include "phonegui-init.h"
#include <Ecore_Evas.h>
#include <Ecore.h>
#include <Edje.h>
#include <Etk.h>
#include <Elementary.h>
#include <glib.h>


static gboolean
_idle_foo(void *foo)
{
	return (TRUE);
}


void
phonegui_backend_init(int argc, char **argv, int (*idle_cb) (void *))
{
	// Initialize gettext
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");

	g_type_init();

	/* Initialize glib main loop */
	GMainLoop *mainloop = g_main_loop_new(NULL, FALSE);

	// Initializations
	elm_init(argc, argv);
	g_debug("Initiated elementary");

	if (!ecore_main_loop_glib_integrate()) {
		g_fatal("failed integrating the glib mainloop");
	}

}

void phonegui_backend_loop()
{
	elm_run();
	elm_shutdown();
}
