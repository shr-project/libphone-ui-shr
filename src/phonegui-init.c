#include "phonegui-init.h"
#include <Ecore_Evas.h>
#include <Ecore.h>
#include <Edje.h>
#include <Etk.h>
#include <Elementary.h>
#include <glib.h>


void
phonegui_backend_init(int argc, char **argv, void (*exit_cb) ())
{
	// Initialize gettext
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");

	// Assign arguments
	phonegui_argc = argc;
	phonegui_argv = argv;
	phonegui_exit_cb = exit_cb;

	/* Initialize glib main loop */
	GMainLoop *mainloop = NULL;
	g_type_init();
	mainloop = g_main_loop_new(NULL, FALSE);

	// Initializations
	elm_init(phonegui_argc, phonegui_argv);
	g_debug("Initiated elementary");

	if (!ecore_main_loop_glib_integrate()) {
		g_fatal("failed integrating the glib mainloop");
	}
}



