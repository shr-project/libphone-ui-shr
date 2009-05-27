#include "views.h"
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-dbus.h>
#include <frameworkd-glib/ogsmd/frameworkd-glib-ogsmd-sim.h>


struct ContactListViewData {
	struct Window *win;
	Evas_Object *list;
	Evas_Object *bx, *hv;
	Evas_Object *bt1, *bt2, *bt_options, *bt_message, *bt_edit, *bt_delete;
};


static Elm_Genlist_Item_Class itc;
DBusGProxy *GQuery = NULL;


static void 
frame_list_show(void *_data);
static void 
frame_list_hide(void *_data);
static void 
frame_list_new_clicked(void *_data, Evas_Object *obj, void *event_info);
static void 
frame_list_call_clicked(void *_data, Evas_Object *obj, void *event_info);
static void 
frame_list_options_clicked(void *_data, Evas_Object *obj, void *event_info);
static void 
frame_list_message_clicked(void *_data, Evas_Object *obj, void *event_info);
static void 
frame_list_edit_clicked(void *_data, Evas_Object *obj, void *event_info);
static void 
frame_list_delete_clicked(void *_data, Evas_Object *obj, void *event_info);
static void 
frame_list_refresh(void *_data);
static void 
frame_list_refresh_callback(struct ContactListViewData *data);


/* --- main contact list view ----------------------------------------------- */

void *
contact_list_view_show(struct Window *win, void *_options)
{
	struct ContactListViewData *data = g_slice_alloc0(sizeof(struct ContactListViewData));
	data->win = win;

	g_debug("contact_list_view_show()");

	window_frame_show(win, data, frame_list_show, frame_list_hide);
	window_show(win);

	return data;
}

void 
contact_list_view_hide(void *_data)
{
	g_debug("contact_list_view_hide()");
	g_slice_free(struct ContactListViewData, _data);
}



/* --- frame "list" --------------------------------------------------------- */

char *
gl_label_get(const void *data, Evas_Object *obj, const char *part)
{
	GHashTable *parameters = (GHashTable *)data;
	char *label = NULL;

	//g_debug("gl_label_get(part=%s)", part);

	if (!strcmp(part, "elm.text"))
		label = g_hash_table_lookup(parameters, "name");
	else if (!strcmp(part, "elm.text.sub"))
		label = g_hash_table_lookup(parameters, "number");

	return strdup(label);
}


Evas_Object *
gl_icon_get(const void *data, Evas_Object *obj, const char *part)
{
	return NULL;
}


Evas_Bool 
gl_state_get(const void *data, Evas_Object *obj, const char *part)
{
	return 0;
}


void 
gl_del(const void *data, Evas_Object *obj)
{
}


static gint 
_compare_entries(gconstpointer _a, gconstpointer _b)
{
	GHashTable **a = (GHashTable **)_a;
	GHashTable **b = (GHashTable **)_b;
	gpointer p; 
	const char *name_a, *name_b;

	p = g_hash_table_lookup(*a, "Name");
	if (!p) {
		name_a = "";
		g_debug("name a not found!!!!");
	} else
		name_a = g_value_get_string(p);

	p = g_hash_table_lookup(*b, "Name");
	if (!p) {
		name_b = "";
		g_debug("name b not found!!!!");
	} else
		name_b = g_value_get_string(p);

	return (strcasecmp(name_a, name_b));
}

static void 
_process_entry(gpointer _entry, gpointer _data)
{
	GHashTable *entry = (GHashTable *)_entry;
	struct ContactListViewData *data = (struct ContactListViewData *)_data;
	const char *number = g_value_get_string(g_hash_table_lookup(entry, "Phone"));

	if (number[0] == 't' && number[1] == 'e' && number[2] == 'l' && number[3] == ':')
		number += 4;

	g_debug("number is '%s'", number);

	GHashTable *parameters = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
	g_hash_table_insert(parameters, "path", strdup(g_value_get_string(g_hash_table_lookup(entry, "Path"))));
	g_hash_table_insert(parameters, "name", strdup(g_value_get_string(g_hash_table_lookup(entry, "Name"))));
	g_hash_table_insert(parameters, "number", strdup(number));

	elm_genlist_item_append(data->list, &itc, parameters/*item data*/, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
}

static void 
_retrieve_callback(GError *error, GPtrArray *messages, void *data)
{
	if (error != NULL || messages == NULL) {
		g_debug("dbus error !!!");
		return;
	}

	g_ptr_array_sort(messages, _compare_entries);
	g_ptr_array_foreach(messages, _process_entry, data);
}

static void 
_result_callback(GError *error, int count, void *data)
{
	if (error == NULL) {
		g_debug("result gave %d entries --> retrieving", count);
		opimd_contact_query_get_multiple_results(GQuery, count, _retrieve_callback, data);
	}
}

static void 
_query_callback(GError *error, char *query_path, void *data)
{
	if (error == NULL) {
		g_debug("query path is %s", query_path);
		GQuery = dbus_connect_to_opimd_contact_query (query_path);
		opimd_contact_query_get_result_count (GQuery, _result_callback, data);
	}
}



static void 
frame_list_show(void *_data)
{
	struct ContactListViewData *data = (struct ContactListViewData *)_data;
	struct Window *win = data->win;

	g_debug("frame_list_show()");

	window_layout_set(win, CONTACTS_FILE, "list");

	data->list = elm_genlist_add(window_evas_object_get(data->win));
	window_swallow(data->win, "list", data->list);
	itc.item_style     = "double_label";
	itc.func.label_get = gl_label_get;
	itc.func.icon_get  = gl_icon_get;
	itc.func.state_get = gl_state_get;
	itc.func.del       = gl_del;
	evas_object_show(data->list);

	data->bt1 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt1, D_("New"));
	evas_object_smart_callback_add(data->bt1, "clicked", frame_list_new_clicked, data);
	window_swallow(win, "button_new", data->bt1);
	evas_object_show(data->bt1);

	data->bt2 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt2, D_("Call"));
	evas_object_smart_callback_add(data->bt2, "clicked", frame_list_call_clicked, data);
	window_swallow(win, "button_call", data->bt2);
	evas_object_show(data->bt2);

	data->bt_options = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt_options, D_("Options"));
	evas_object_smart_callback_add(data->bt_options, "clicked", frame_list_options_clicked, data);
	window_swallow(win, "button_options", data->bt_options);
	evas_object_show(data->bt_options);


	/* Options */
	data->hv = elm_hover_add(window_evas_object_get(win));
	elm_hover_parent_set(data->hv, window_evas_object_get(win));
	elm_hover_target_set(data->hv, data->bt_options);

	data->bx = elm_box_add(window_evas_object_get(win));
	elm_box_horizontal_set(data->bx, 0);
	elm_box_homogenous_set(data->bx, 1);
	evas_object_show(data->bx);

	data->bt_message = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt_message, D_("SMS"));
	evas_object_size_hint_min_set(data->bt_message, 130, 80);
	evas_object_smart_callback_add(data->bt_message, "clicked", frame_list_message_clicked, data);
	evas_object_show(data->bt_message);
	elm_box_pack_end(data->bx, data->bt_message);

	data->bt_edit = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt_edit, D_("Edit"));
	evas_object_size_hint_min_set(data->bt_edit, 130, 80);
	evas_object_smart_callback_add(data->bt_edit, "clicked", frame_list_edit_clicked, data);
	evas_object_show(data->bt_edit);
	elm_box_pack_end(data->bx, data->bt_edit);

	data->bt_delete = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt_delete, D_("Delete"));
	evas_object_size_hint_min_set(data->bt_delete, 130, 80);
	evas_object_smart_callback_add(data->bt_delete, "clicked", frame_list_delete_clicked, data);
	evas_object_show(data->bt_delete);
	elm_box_pack_end(data->bx, data->bt_delete);

	elm_hover_content_set(data->hv, "top", data->bx);

	/* .. */
	GHashTable *qry = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
	opimd_contacts_query(qry, _query_callback, data);
	g_hash_table_destroy(qry);
}

static void 
frame_list_hide(void *_data)
{
	struct ContactListViewData *data = (struct ContactListViewData *)_data;

	g_debug("frame_list_hide()");

	evas_object_del(data->bt1);
	evas_object_del(data->bt2);
	evas_object_del(data->bt_options);
	evas_object_del(data->list);
}

static void 
frame_list_new_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	struct ContactListViewData *data = (struct ContactListViewData *)_data;

	g_debug("frame_list_new_clicked()");

	GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(options, "change_callback", frame_list_refresh);
	g_hash_table_insert(options, "change_callback_data", data);

	struct Window *win = window_new(D_("New Contact"));
	window_init(win);
	window_view_show(win, options, contact_edit_view_show, contact_edit_view_hide);
}

static void 
frame_list_call_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	struct ContactListViewData *data = (struct ContactListViewData *)_data;

	g_debug("frame_list_call_clicked()");

	Elm_Genlist_Item *it = elm_genlist_selected_item_get(data->list);
	GHashTable *properties = it ? elm_genlist_item_data_get(it) : NULL;

	if (properties != NULL)
		ogsmd_call_initiate(g_hash_table_lookup(properties, "number"), "voice", NULL, NULL);
}

static void 
frame_list_options_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	struct ContactListViewData *data = (struct ContactListViewData *)_data;

	g_debug("frame_list_options_clicked()");

	evas_object_show(data->hv);
}

static void 
frame_list_message_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	struct ContactListViewData *data = (struct ContactListViewData *)_data;

	g_debug("frame_list_message_clicked()");

	evas_object_hide(data->hv);

	Elm_Genlist_Item *it = elm_genlist_selected_item_get(data->list);
	GHashTable *properties = it ? elm_genlist_item_data_get(it) : NULL;

	if (properties != NULL) {
		assert(g_hash_table_lookup(properties, "number") != NULL);

		GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
		g_hash_table_insert(options, "recipient", g_hash_table_lookup(properties, "number"));

		struct Window *win = window_new(D_("Compose SMS"));
		window_init(win);
		window_view_show(win, options, message_new_view_show, message_new_view_hide);
	}
}

static void 
frame_list_edit_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	struct ContactListViewData *data = (struct ContactListViewData *)_data;

	g_debug("frame_list_edit_clicked()");

	evas_object_hide(data->hv);

	Elm_Genlist_Item *it = elm_genlist_selected_item_get(data->list);
	GHashTable *properties = it ? elm_genlist_item_data_get(it) : NULL;

	if (properties != NULL) {
		GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
		g_hash_table_insert(options, "path", g_hash_table_lookup(properties, "path"));
		g_hash_table_insert(options, "name", g_hash_table_lookup(properties, "name"));
		g_hash_table_insert(options, "number", g_hash_table_lookup(properties, "number"));
		g_hash_table_insert(options, "change_callback", frame_list_refresh);
		g_hash_table_insert(options, "change_callback_data", data);

		struct Window *win = window_new(D_("Edit Contact"));
		window_init(win);
		window_view_show(win, options, contact_edit_view_show, contact_edit_view_hide);
	}
}

static void 
frame_list_delete_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	struct ContactListViewData *data = (struct ContactListViewData *)_data;

	g_debug("frame_list_delete_clicked()");

	evas_object_hide(data->hv);

	Elm_Genlist_Item *it = elm_genlist_selected_item_get(data->list);
	GHashTable *properties = it ? elm_genlist_item_data_get(it) : NULL;

	if (properties != NULL) {
		GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
		g_hash_table_insert(options, "path", g_hash_table_lookup(properties, "path"));
		g_hash_table_insert(options, "delete_callback", frame_list_refresh);
		g_hash_table_insert(options, "delete_callback_data", data);

		struct Window *win = window_new(D_("Delete Contact"));
		window_init(win);
		window_view_show(win, options, contact_delete_view_show, contact_delete_view_hide);
	}
}

static void 
frame_list_refresh(void *_data)
{
	g_debug("frame_list_refresh");
	struct ContactListViewData *data = (struct ContactListViewData *)_data;
	async_trigger(frame_list_refresh_callback, data);
}

static void 
frame_list_refresh_callback(struct ContactListViewData *data)
{
	g_debug("frame_list_refresh_callback");
	elm_genlist_clear(data->list);

	GHashTable *qry = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
	opimd_contacts_query(qry, _query_callback, data);
	g_hash_table_destroy(qry);
}

