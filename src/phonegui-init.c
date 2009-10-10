#include "phonegui-init.h"
#include <Ecore_Evas.h>
#include <Ecore.h>
#include <Edje.h>
#include <Etk.h>
#include <Elementary.h>
#include <glib.h>


static gboolean
_idle_foo(void *)
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

	/* Initialize glib main loop */
	GMainContext *context = NULL;
	GMainLoop *mainloop = NULL;
	g_type_init();
	context = g_main_context_new();
	mainloop = g_main_loop_new(context, FALSE);

	if (idle_cb) {
		GSourceFuncs *idle_funcs = g_malloc(sizeof(GSourceFuncs));
		idle_funcs->prepare = _idle_foo;
		idle_funcs->check = _idle_foo;
		idle_funcs->dispatch = idle_cb;
		idle_funcs->finalize = NULL;
		GSource *src = g_source_new(idle_funcs, sizeof(GSource));
		g_source_attach(src, context);
	}

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
