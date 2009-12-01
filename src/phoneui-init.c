#include "phoneui-shr.h"
#include "phoneui-init.h"
#include <Ecore_Evas.h>
#include <Ecore.h>
#include <Edje.h>
#include <Elementary.h>
#include <glib.h>
#include <glib-object.h>


int phoneui_argc;
char **phoneui_argv;


void
phoneui_backend_init(int argc, char **argv, int (*idle_cb) (void *))
{
	GKeyFile *keyfile;
	GKeyFileFlags flags;
	GError *error = NULL;
	char *theme = NULL;
	int theme_len;

	// Initialize gettext
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	phoneui_argc = argc;
	phoneui_argv = argv;

	g_type_init();

	keyfile = g_key_file_new();
	flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;
	if (!g_key_file_load_from_file
	    (keyfile, PHONEUI_SHR_CONFIG, flags, &error)) {
		g_warning("%s", error->message);
		return;
	}

	theme = g_key_file_get_string(keyfile, "global", "theme", NULL);
	if (theme) {
		/* FIXME: possible overflow in line 51 */
		/* +6 for /, .edj and ending 0 */
		theme_len = strlen(PKGDATADIR) + strlen(theme) + 6;
		phoneui_theme = malloc(theme_len);
		if (!phoneui_theme) {
			g_critical("Out of memory allocating theme path");
		}
		else {
			snprintf(phoneui_theme, theme_len, "%s/%s.edj",
					PKGDATADIR, theme);
			g_debug("setting theme to (%d) %s", theme_len, phoneui_theme);
		}
	}

	g_key_file_free(keyfile);
}

void phoneui_backend_loop()
{
	/* Initialize glib main loop */
	GMainLoop *mainloop = g_main_loop_new(NULL, FALSE);

	// Initializations
	elm_init(phoneui_argc, phoneui_argv);
	g_debug("Initiated elementary");

	if (!ecore_main_loop_glib_integrate()) {
		g_critical("failed integrating the glib mainloop");
		return;
	}

	elm_run();
	elm_shutdown();
}
