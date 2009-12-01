#include "phoneui-call.h"
#include "call-common.h"
#include <glib.h>
#include "window.h"
#include "views.h"


/* HACK UNTIL WE REMOVE THIS INSTANCE THING COMPLETELY*/
#include <Ecore_Evas.h>


struct Instance {
	int id;
	struct Window *win;
};


static int instances_size = 0;
static struct Instance *instances;


void
instance_manager_add(int id, struct Window *win)
{
	g_debug("instance_manager_add: id=%d, window=%d", id,
		win);
	instances_size++;
	if (instances_size == 1) {
		instances = malloc(sizeof(struct Instance));
		g_debug("malloc'ed");
	}
	else {
		instances =
			realloc(instances,
				sizeof(struct Instance) * instances_size);
		g_debug("realloc'ed");
	}

	g_debug("add window: %d", win->win);

	instances[instances_size - 1].id = id;
	instances[instances_size - 1].win = win;

	g_debug("done");
}

struct Window *
instance_manager_remove(int id)
{
	int i;
	for (i = 0; i < instances_size; i++) {
		if (instances[i].id == id) {
			g_debug("instance_manager_remove: %d",
				instances[i].win);
			return (instances[i].win);
		}
	}

	// TODO: Free things

	return NULL;
}


struct Window *
instance_manager_remove_by_ecore_evas(Ecore_Evas * ee)
{
	int i;
	for (i = 0; i < instances_size; i++) {
		g_debug("foreach win: %d", instances[i].win->win);
		if (ee ==
		    ecore_evas_ecore_evas_get(evas_object_evas_get
					      (instances[i].win->win))) {
			g_debug("instance_manager_remove_by_ecore_evas: %d",
				instances[i].win);
			return (instances[i].win);
		}
	}

	// TODO: Free things

	return NULL;
}

/* ADDED THE INSTANCE THING BACK JUST HERE AS i HAVE NO TIME TO REMOVE IT
 * COMPLETELY ATM */
enum CallTypes {
	CALL_INCOMING,
	CALL_ACTIVE
};

struct Call {
	int id;
	struct Window *win;
};

static void _show(const int id, const int status, const char *number, int type);
static void _hide(const int id);
static void _delete(void *data, Evas_Object * win, void *event_info);


void
phoneui_backend_incoming_call_show(const int id, const int status,
				    const char *number)
{
	_show(id, status, number, CALL_INCOMING);
}

void
phoneui_backend_incoming_call_hide(const int id)
{
	_hide(id);
}

void
phoneui_backend_outgoing_call_show(const int id, const int status,
				    const char *number)
{
	_show(id, status, number, CALL_ACTIVE);
}

void
phoneui_backend_outgoing_call_hide(const int id)
{
	_hide(id);
}


static void
_show(const int id, const int status, const char *number, int type)
{
	struct Window *win = window_new(D_("Call"));
	window_delete_callback_set(win, _delete);
	instance_manager_add(id, win);
	
	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(options, "id", GINT_TO_POINTER(id));
	g_hash_table_insert(options, "status", GINT_TO_POINTER(status));
	g_hash_table_insert(options, "number", g_strdup(number));
	g_hash_table_insert(options, "number_state",
			GINT_TO_POINTER(CALL_NUMBER_NUMBER));

	window_init(win);
	if (type == CALL_INCOMING) {
		window_view_show(win, options, call_incoming_view_show,
				 call_incoming_view_hide, NULL);
	}
	else if (type == CALL_ACTIVE) {
		window_view_show(win, options, call_active_view_show,
				 call_active_view_hide, NULL);
	}
	else {
		g_critical("Unknown call type: %d", type);
		g_hash_table_destroy(options);
	}
}


static void
_hide(const int id)
{
	g_debug("call_hide(id=%d)", id);
	call_common_active_call_remove(id);

	struct Window *win = instance_manager_remove(id);
	if (win) {
		window_destroy(win, NULL);
	}
	else {
		g_critical("Tried to hide a unitinialized window");
	}
}

static void
_delete(void *data, Evas_Object * win, void *event_info)
{
	g_debug("call_delete(), release call!");
	//ogsmd_call_release(call_id, NULL, NULL);
	//window_destroy(win, NULL);
}
