
#include <glib.h>
#include <phoneui/phoneui-utils-calls.h>
#include "phoneui-call.h"
#include "call-common.h"
#include "phoneui-shr.h"
#include "window.h"
#include "views.h"

/* HACK UNTIL WE REMOVE THIS INSTANCE THING COMPLETELY*/
#include <Evas.h>
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
		(int) win);
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

	g_debug("add window: %d", (int) win->win);

	instances[instances_size - 1].id = id;
	instances[instances_size - 1].win = win;

	g_debug("done");
}

struct Window *
instance_manager_remove(int id)
{
	struct Window *win = NULL;
	int i, j;
	for (i = 0; i < instances_size; i++) {
		if (instances[i].id == id) {
			win = instances[i].win;
			break;
		}
	}

	for (j = i + 1 ; j < instances_size ; j++) {
		instances[j - 1].id = instances[j].id;
		instances[j - 1].win = instances[j].win;
	}
	if (win) {
		instances_size--;
		/*FIXME: handle if can't allocate */
		instances = realloc(instances, sizeof(struct Instance) * instances_size);
	}

	return win;
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
static void _delete(void *_data, Evas_Object * win, void *event_info);


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
	instance_manager_add(id, win);

	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(options, "id", GINT_TO_POINTER(id));
	g_hash_table_insert(options, "status", GINT_TO_POINTER(status));
	g_hash_table_insert(options, "number", g_strdup(number));
	g_hash_table_insert(options, "number_state",
			GINT_TO_POINTER(CALL_NUMBER_NUMBER));

	window_init(win);
	window_delete_callback_set(win, _delete);
	if (type == CALL_INCOMING) {
		window_view_show(win, options, (void * (*)(struct Window *, void *)) call_incoming_view_show,
				 (void (*)(void *)) call_incoming_view_hide, NULL);
	}
	else if (type == CALL_ACTIVE) {
		window_view_show(win, options,(void * (*)(struct Window *, void *)) call_active_view_show,
				 (void (*)(void *)) call_active_view_hide, NULL);
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
_delete(void *_data, Evas_Object * win, void *event_info)
{
	(void) win;
	(void) event_info;
 	struct CallActiveViewData *data;
	g_debug("call_delete(), release call!");

 	data = ((struct Window *)_data)->view_data;
 	phoneui_utils_call_release(data->parent.id, NULL, NULL);
}
