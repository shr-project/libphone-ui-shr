#include "views.h"
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-dbus.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-sim.h>


struct MessageShowViewData {
	struct Window *win;
	char *status, *number, *content, *name, *date, *path;
	int id;
	GHashTable *properties;
	GHashTable *query;
	GValueArray *message;
	Evas_Object *content_entry, *bt1, *bt2, *bt3, *hv, *bx, *hbt1, *hbt2,
		*hbt3;

	void (*callback) ();
	void *callback_data;
};

typedef enum {
	MODE_FOLDERS,
	MODE_LIST,
	MODE_MESSAGE,
	MODE_NEW1,
	MODE_NEW2,
	MODE_DELETE
} MessagesModes;



void *message_show_view_show(struct Window *win, void *_options);
void
  message_show_view_hide(void *_data);

static void


message_show_view_close_clicked(void *_data, Evas_Object * obj,
				void *event_info);
static void


message_show_view_answer_clicked(void *_data, Evas_Object * obj,
				 void *event_info);
static void


message_show_view_delete_clicked(void *_data, Evas_Object * obj,
				 void *event_info);
static void
  message_show_view_delete_callback(void *_data);
static void
  message_show_view_delete_callback_callback(struct MessageShowViewData *data);
static void


message_show_view_call_clicked(void *_data, Evas_Object * obj,
			       void *event_info);
static void
  my_hover_bt_1(void *_data, Evas_Object * obj, void *event_info);

static void
  name_callback(GError * error, char *name, gpointer _data);
static void
  name_callback2(struct MessageShowViewData *data);
static void
  retrieve_callback(GHashTable * properties, gpointer _data);
static void
  retrieve_callback2(struct MessageShowViewData *data);



/* --- main message show view functions ------------------------------------- */

void *
message_show_view_show(struct Window *win, void *_options)
{
	assert(win != NULL);
	assert(_options != NULL);
	GHashTable *options = (GHashTable *) _options;

	g_debug("message_show_view_show()");

	struct MessageShowViewData *data =
		g_slice_alloc0(sizeof(struct MessageShowViewData));
	data->win = win;

	char *direction, *status, *tmp;
	status = g_hash_table_lookup(options, "status");
	direction = g_hash_table_lookup(options, "direction");
	tmp = malloc(strlen(status) + strlen(direction) + 4);
	sprintf(tmp, "%s (%s)", status, direction);

	data->path = g_hash_table_lookup(options, "path");
	data->name = g_hash_table_lookup(options, "name");
	data->number = g_hash_table_lookup(options, "number");
	data->content = g_hash_table_lookup(options, "content");
	data->status = tmp;
	data->date = g_hash_table_lookup(options, "date");

	if (options != NULL) {
		data->callback =
			g_hash_table_lookup(options, "delete_callback");
		data->callback_data =
			g_hash_table_lookup(options, "delete_callback_data");
	}

	retrieve_callback(options, data);

	return data;
}

void
message_show_view_hide(void *_data)
{
	struct MessageShowViewData *data = (struct MessageShowViewData *) _data;
	struct Window *win = data->win;

	g_debug("message_show_view_hide()");

	evas_object_del(data->bt1);
	evas_object_del(data->bt2);
	evas_object_del(data->bt3);
	evas_object_del(data->hbt1);
	evas_object_del(data->hbt2);
	evas_object_del(data->hbt3);
	evas_object_del(data->bx);
	evas_object_del(data->hv);

	g_slice_free(struct MessageShowViewData, data);
}


/* --- evas callbacks ------------------------------------------------------- */

static void
message_show_view_close_clicked(void *_data, Evas_Object * obj,
				void *event_info)
{
	struct MessageShowViewData *data = (struct MessageShowViewData *) _data;

	g_debug("message_show_view_close_clicked()");

	window_destroy(data->win, NULL);
}

static void
message_show_view_answer_clicked(void *_data, Evas_Object * obj,
				 void *event_info)
{
	struct MessageShowViewData *data = (struct MessageShowViewData *) _data;

	g_debug("message_show_view_answer_clicked()");

	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(options, "name", data->name);
	g_hash_table_insert(options, "number", data->number);

	struct Window *win = window_new(D_("Compose SMS"));
	window_init(win);
	window_view_show(win, options, message_new_view_show,
			 message_new_view_hide);
}

static void
message_show_view_call_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct MessageShowViewData *data = (struct MessageShowViewData *) _data;

	g_debug("message_show_view_call_clicked()");

	ogsmd_call_initiate(data->number, "voice", NULL, NULL);
}

static void
message_show_view_delete_clicked(void *_data, Evas_Object * obj,
				 void *event_info)
{
	struct MessageShowViewData *data = (struct MessageShowViewData *) _data;

	g_debug("message_show_view_delete_clicked()");

	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(options, "path", data->path);
	g_hash_table_insert(options, "delete_callback",
			    message_show_view_delete_callback);
	g_hash_table_insert(options, "delete_callback_data", data);

	struct Window *win = window_new(D_("Delete Message"));
	window_init(win);
	window_view_show(win, options, message_delete_view_show,
			 message_delete_view_hide);
}

static void
message_show_view_delete_callback(void *_data)
{
	struct MessageShowViewData *data = (struct MessageShowViewData *) _data;

	g_debug("message_show_view_delete_callback()");

	async_trigger(message_show_view_delete_callback_callback, data);
}

static void
message_show_view_delete_callback_callback(struct MessageShowViewData *data)
{
	window_destroy(data->win, NULL);

	if (data->callback != NULL)
		data->callback(data->callback_data);
}

static void
my_hover_bt_1(void *_data, Evas_Object * obj, void *event_info)
{
	Evas_Object *hv = (Evas_Object *) _data;

	g_debug("my_hover_bt_1()");

	evas_object_show(hv);
}



/* --- dbus/libframeworkd callbacks ----------------------------------------- */


static void
message_common_name_callback(GError * error, char *name, void *_data)
{
	struct MessageShowViewData *data = (struct MessageShowViewData *) _data;

	g_debug("got contact: \"%s\" error? (%d)", name, error);
	if (error == NULL && *name) {
		data->name = strdup(name);
	}
	async_trigger(name_callback2, data);
}


static void
name_callback2(struct MessageShowViewData *data)
{
	g_debug("name updating...");
	if (data->name) {
		window_text_set(data->win, "text_number", data->name);
	}
	else {
		window_text_set(data->win, "text_number", data->number);
	}
}

static void
retrieve_callback(GHashTable * properties, gpointer _data)
{
	struct MessageShowViewData *data = (struct MessageShowViewData *) _data;

	g_debug("retrieve_callback()");

	data->properties = properties;	// TODO: copy
	//data->query = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);

	phonegui_contact_lookup(data->number, message_common_name_callback,
				data);

	g_debug("loading message data...");

	async_trigger(retrieve_callback2, data);
}

static void
message_show_view_new_contact_clicked(struct MessageShowViewData *data,
				      Evas_Object * obj, void *event_info)
{
	phonegui_contacts_new_show(NULL, data->number);
}

static void
retrieve_callback2(struct MessageShowViewData *data)
{
	struct Window *win = data->win;

	window_layout_set(win, MESSAGE_FILE, "message_show");


	char *content = string_replace_with_tags(data->content);

	window_text_set(win, "text_status", data->status);
	window_text_set(win, "text_number", data->number);
	window_text_set(win, "text_content", content);
	window_text_set(win, "text_date", data->date);
	window_text_set(win, "label_number", D_("From:"));
	window_text_set(win, "label_date", D_("Date:"));
	window_text_set(win, "label_status", D_("Status:"));

	data->bt1 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt1, D_("Close"));
	evas_object_smart_callback_add(data->bt1, "clicked",
				       message_show_view_close_clicked, data);
	window_swallow(win, "button_close", data->bt1);
	evas_object_show(data->bt1);


	// Options button with hover
	data->hv = elm_hover_add(window_evas_object_get(win));

	data->bt2 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt2, D_("Options"));
	evas_object_smart_callback_add(data->bt2, "clicked", my_hover_bt_1,
				       data->hv);
	window_swallow(win, "button_options", data->bt2);
	evas_object_show(data->bt2);

	elm_hover_parent_set(data->hv, window_evas_object_get(win));
	elm_hover_target_set(data->hv, data->bt2);

	data->bx = elm_box_add(window_evas_object_get(win));
	elm_box_horizontal_set(data->bx, 0);
	elm_box_homogenous_set(data->bx, 1);
	evas_object_show(data->bx);

	data->hbt1 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->hbt1, D_("Delete"));
	evas_object_size_hint_min_set(data->hbt1, 140, 80);
	evas_object_smart_callback_add(data->hbt1, "clicked",
				       message_show_view_delete_clicked, data);
	evas_object_show(data->hbt1);
	elm_box_pack_end(data->bx, data->hbt1);

	data->hbt2 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->hbt2, D_("Call"));
	evas_object_size_hint_min_set(data->hbt2, 140, 80);
	evas_object_smart_callback_add(data->hbt2, "clicked",
				       message_show_view_call_clicked, data);
	evas_object_show(data->hbt2);
	elm_box_pack_end(data->bx, data->hbt2);

	data->hbt3 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->hbt3, D_("Add Contact"));
	evas_object_size_hint_min_set(data->hbt3, 140, 80);
	evas_object_smart_callback_add(data->hbt3, "clicked",
				       message_show_view_new_contact_clicked,
				       data);
	evas_object_show(data->hbt3);
	elm_box_pack_end(data->bx, data->hbt3);

	elm_hover_content_set(data->hv, "top", data->bx);


	data->bt3 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt3, D_("Answer"));
	evas_object_smart_callback_add(data->bt3, "clicked",
				       message_show_view_answer_clicked, data);
	window_swallow(win, "button_answer", data->bt3);
	evas_object_show(data->bt3);


	window_show(win);
}
