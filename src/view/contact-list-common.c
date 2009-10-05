
#include "contact-list-common.h"

static Elm_Genlist_Item_Class itc;


/* --- genlist callbacks --- */

static char *
gl_label_get(const void *data, Evas_Object *obj, const char *part)
{
	GHashTable *parameters = (GHashTable *)data;
	char *label = NULL;

	if (!strcmp(part, "elm.text"))
		label = g_hash_table_lookup(parameters, "name");
	else if (!strcmp(part, "elm.text.sub"))
		label = g_hash_table_lookup(parameters, "number");

	return strdup(label);
}



static Evas_Object *
gl_icon_get(const void *data, Evas_Object *obj, const char *part)
{
	return NULL;
}



static Eina_Bool
gl_state_get(const void *data, Evas_Object *obj, const char *part)
{
	return 0;
}


static void
gl_del(const void *data, Evas_Object *obj)
{
}


static void
gl_index_changed(void *data, Evas_Object *obj, void *event_info)
{
}

static void
gl_index_changed2(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_top_bring_in(event_info);
}


static void
gl_index_selected(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_top_bring_in(event_info);
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
	Elm_Genlist_Item *it;

	GHashTable *entry = (GHashTable *)_entry;
	struct ContactListViewData *data = (struct ContactListViewData *)_data;
	const char *number;
	GValue *tmp = g_hash_table_lookup(entry, "Phone");
	if (tmp) {
	        number = g_value_get_string(tmp);
	}
	else {
	        number = "No Number";
	}

	const char *name = g_value_get_string(g_hash_table_lookup(entry, "Name"));

	if (number[0] == 't' && number[1] == 'e' && number[2] == 'l' && number[3] == ':')
		number += 4;

	GHashTable *parameters = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
	g_hash_table_insert(parameters, "path", strdup(g_value_get_string(g_hash_table_lookup(entry, "Path"))));
	g_hash_table_insert(parameters, "name", strdup(name));
	g_hash_table_insert(parameters, "number", strdup(number));

	it = elm_genlist_item_append(data->list, &itc, parameters/*item data*/, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	int idx = toupper(*name);
	if (idx && idx != data->current_index) {
		data->current_index = idx;
		elm_index_item_append(data->index, g_strdup_printf("%c", data->current_index), it);
	}
}




static void
_retrieve_callback2(void *_data)
{
	struct ContactListViewData *data = (struct ContactListViewData *)_data;

	g_ptr_array_sort(data->contacts, _compare_entries);
	g_ptr_array_foreach(data->contacts, _process_entry, data);
}




static void
_retrieve_callback(GError *error, GPtrArray *contacts, void *_data)
{
	if (error != NULL || contacts == NULL) {
		g_debug("dbus error !!!");
		return;
	}

	struct ContactListViewData *data = (struct ContactListViewData *)_data;
	data->contacts = contacts;

	async_trigger(_retrieve_callback2, data);
}



static void
_result_callback(GError *error, int count, void *_data)
{
	if (error == NULL) {
		struct ContactListViewData *data = (struct ContactListViewData *)_data;
		g_debug("result gave %d entries --> retrieving", count);
		if (!data->query) {
			g_debug("oops... query vanished!");
			return;
		}
		opimd_contact_query_get_multiple_results(data->query, count, _retrieve_callback, data);
	}
}



static void
_query_callback(GError *error, char *query_path, void *_data)
{
	if (error == NULL) {
		struct ContactListViewData *data = (struct ContactListViewData *)_data;
		g_debug("query path is %s", query_path);
		data->query = (DBusGProxy *)dbus_connect_to_opimd_contact_query (query_path);
		opimd_contact_query_get_result_count (data->query, _result_callback, data);
	}
}





void
contact_list_fill(struct ContactListViewData *data)
{
	GHashTable *qry = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
	data->current_index = 0;
	if (data->query) {
		opimd_contacts_query_dispose(data->query);
		data->query = NULL;
	}
	opimd_contacts_query(qry, _query_callback, data);
	g_hash_table_destroy(qry);
}


Evas_Object *
contact_list_add(struct ContactListViewData *data)
{
	data->list = elm_genlist_add(window_evas_object_get(data->win));
	elm_widget_scale_set(data->list, 1.0);
	window_swallow(data->win, "list", data->list);
	itc.item_style = "contact";
	itc.func.label_get = gl_label_get;
	itc.func.icon_get  = gl_icon_get;
	itc.func.state_get = gl_state_get;
	itc.func.del       = gl_del;
	evas_object_show(data->list);

	data->index = elm_index_add(window_evas_object_get(data->win));
	evas_object_size_hint_weight_set(data->index, 1.0, 1.0);
	elm_win_resize_object_add(window_evas_object_get(data->win), data->index);
	//window_swallow(data->win, "index", data->index);
	evas_object_show(data->index);
	evas_object_smart_callback_add(data->index, "delay,changed", gl_index_changed2, NULL);
	evas_object_smart_callback_add(data->index, "changed", gl_index_changed, NULL);
	evas_object_smart_callback_add(data->index, "selected", gl_index_selected, NULL);
	elm_index_item_go(data->index, 0);
}


