#include "phoneui-call.h"
#include "call-common.h"
#include <glib.h>
#include "window.h"
#include "views.h"

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

	struct Window *win = instance_manager_remove(INSTANCE_CALL, id);
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
