
#include "views.h"

#include <ctype.h> /* to upper */

static Elm_Genlist_Item_Class itc;


/* --- genlist callbacks --- */

static char *
gl_label_get(const void *data, Evas_Object * obj, const char *part)
{
	GHashTable *parameters = (GHashTable *) data;
	char *label = NULL;

	if (!strcmp(part, "elm.text")) {
		GValue *tmp = g_hash_table_lookup(parameters, "Name");
		if (tmp)
			label = g_value_get_string(tmp);
		if (!label || !*label)
			label = CONTACT_NAME_UNDEFINED_STRING;
	}
	else if (!strcmp(part, "elm.text.sub")) {
		GValue *tmp = g_hash_table_lookup(parameters, "Phone");
		if (tmp) {
			label = string_skip_tel_prefix(
					g_value_get_string(tmp));
		}
		if (!label || !*label)
			label = CONTACT_PHONE_UNDEFINED_STRING;
	}

	return (strdup(label));
}



static Evas_Object *
gl_icon_get(const void *data, Evas_Object * obj, const char *part)
{
	GHashTable *parameters = (GHashTable *) data;
	if (!strcmp(part, "elm.swallow.icon")) {
		const char *photo_file;
		GValue *tmp = g_hash_table_lookup(parameters, "Photo");
		if (tmp)
			photo_file = g_value_get_string(tmp);
		else
			photo_file = CONTACT_DEFAULT_PHOTO;
		Evas_Object *photo = elm_icon_add(obj);
		elm_icon_file_set(photo, photo_file, NULL);
		evas_object_size_hint_aspect_set(photo,
						 EVAS_ASPECT_CONTROL_VERTICAL,
						 1, 1);
		return (photo);
	}
	return (NULL);
}



static Eina_Bool
gl_state_get(const void *data, Evas_Object * obj, const char *part)
{
	return (0);
}


static void
gl_del(const void *data, Evas_Object * obj)
{
}


static void
gl_index_changed(void *data, Evas_Object * obj, void *event_info)
{
}

static void
gl_index_changed2(void *data, Evas_Object * obj, void *event_info)
{
	elm_genlist_item_top_bring_in(event_info);
}


static void
gl_index_selected(void *data, Evas_Object * obj, void *event_info)
{
	elm_genlist_item_top_bring_in(event_info);
}



static gint
_compare_entries(gconstpointer _a, gconstpointer _b)
{
	GHashTable **a = (GHashTable **) _a;
	GHashTable **b = (GHashTable **) _b;
	gpointer p;
	const char *name_a, *name_b;

	p = g_hash_table_lookup(*a, "Name");
	if (!p) {
		name_a = "";
		g_debug("name a not found!!!!");
	}
	else
		name_a = g_value_get_string(p);

	p = g_hash_table_lookup(*b, "Name");
	if (!p) {
		name_b = "";
		g_debug("name b not found!!!!");
	}
	else
		name_b = g_value_get_string(p);

	return (strcasecmp(name_a, name_b));
}
int /* thanks to evas */
utf8_get_next(const unsigned char *buf, int *iindex)
{
   /* Reads UTF8 bytes from @buf, starting at *@index and returns
    * the decoded code point at iindex offset, and advances iindex
    * to the next code point after this.
    *
    * Returns 0 to indicate there is no next char
    */
	int index = *iindex, len, r;
	unsigned char d, d2, d3, d4;

	/* if this char is the null terminator, exit */
	if (!buf[index])
		return 0;

	d = buf[index++];

	while (buf[index] && ((buf[index] & 0xc0) == 0x80))
		index++;

	len = index - *iindex;

	if (len == 1)
		r = d;
	else if (len == 2)
	{
		/* 2 bytes */
		d2 = buf[*iindex + 1];
		r = d & 0x1f; /* copy lower 5 */
		r <<= 6;
		r |= (d2 & 0x3f); /* copy lower 6 */
	}
	else if (len == 3)
	{
		/* 3 bytes */
		d2 = buf[*iindex + 1];
		d3 = buf[*iindex + 2];
		r = d & 0x0f; /* copy lower 4 */
		r <<= 6;
		r |= (d2 & 0x3f);
		r <<= 6;
		r |= (d3 & 0x3f);
	}
	else
	{
		/* 4 bytes */
		d2 = buf[*iindex + 1];
		d3 = buf[*iindex + 2];
		d4 = buf[*iindex + 3];
		r = d & 0x0f; /* copy lower 4 */
		r <<= 6;
		r |= (d2 & 0x3f);
		r <<= 6;
		r |= (d3 & 0x3f);
		r <<= 6;
		r |= (d4 & 0x3f);
	}

	*iindex = index;
	return r;
}


/* allocates space and returns the index part of a string */
static char *
_new_get_index(const char *_string)
{
	/*FIXME: handle the upper in a more sane manner,
	 * i.e, unicode support */
	char *string;
	int i;

	i = 0;
	utf8_get_next(_string, &i);

	string = malloc(i + 1);
	if (!string) {
		return NULL;
	}
	strncpy(string, _string, i);
	string[i] = '\0';

	if (i == 1) {/* i.e, an ascii char */
		string[0] = toupper(string[0]);
	}

	return string;
}

static void
_process_entry(gpointer _entry, gpointer _data)
{
	/* FIXME: limit to 13 indexes - BAD, gotta find a way to calculate
	 * this, furthermore, should probably choose the best indexes better */
	static int limit = 13;
	Elm_Genlist_Item *it;
	char *idx;
	GHashTable *entry = (GHashTable *) _entry;
	struct ContactListViewData *data = (struct ContactListViewData *) _data;

	it = elm_genlist_item_append(data->list, &itc,
				     g_hash_table_ref(entry) /*item data */ ,
				     NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	GValue *tmp = g_hash_table_lookup(entry, "Name");
	const char *name;
	if (tmp) {
		name = g_value_get_string(tmp);
	}
	else {
		name = "";
	}
	if (!name || !*name) {
		return;
	}

	/* if we only started, count how many indexes we should pass
	 * before entering a new one */
	if (!data->current_index) {
		data->index_count = data->contact_count / limit;
		data->new_index = 0;
	}
	
	idx = _new_get_index(name);
	if (!data->current_index || strcmp(idx, data->current_index)) {
		if (data->current_index) {
			free(data->current_index);
		}
		data->current_index = idx;
		data->current_index_item = it;
		data->new_index = 1;
	}
	else {
		free(idx);
	}
	if (data->index_count < 1 && data->new_index) {
		g_debug("Adding index %s", data->current_index);
		elm_index_item_append(data->index, data->current_index,
					data->current_index_item);
		data->index_count = data->contact_count / limit;
		data->new_index = 0;
	}
	data->index_count--;
}




static void
_retrieve_callback2(void *_data)
{
	struct ContactListViewData *data = (struct ContactListViewData *) _data;

	g_ptr_array_sort(data->contacts, _compare_entries);
	g_ptr_array_foreach(data->contacts, _process_entry, data);

	elm_index_item_go(data->index, 0);
}




static void
_retrieve_callback(GError * error, GPtrArray * contacts, void *_data)
{
	if (error != NULL || contacts == NULL) {
		g_debug("dbus error !!!");
		return;
	}

	struct ContactListViewData *data = (struct ContactListViewData *) _data;
	data->contacts = contacts;

	async_trigger(_retrieve_callback2, data);
}



static void
_result_callback(GError * error, int count, void *_data)
{
	if (error == NULL) {
		struct ContactListViewData *data =
			(struct ContactListViewData *) _data;
		g_debug("result gave %d entries --> retrieving", count);
		if (!data->query) {
			g_debug("oops... query vanished!");
			return;
		}
		data->contact_count = count;
		opimd_contact_query_get_multiple_results(data->query, count,
							 _retrieve_callback,
							 data);
	}
}



static void
_query_callback(GError * error, char *query_path, void *_data)
{
	if (error == NULL) {
		struct ContactListViewData *data =
			(struct ContactListViewData *) _data;
		g_debug("query path is %s", query_path);
		data->query = (DBusGProxy *)
			dbus_connect_to_opimd_contact_query(query_path);
		opimd_contact_query_get_result_count(data->query,
						     _result_callback, data);
	}
}





void
contact_list_fill(struct ContactListViewData *data)
{
	GHashTable *qry =
		g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
	data->current_index = NULL;
	data->current_index_item = NULL;
	if (data->query) {
		opimd_contacts_query_dispose(data->query);
		data->query = NULL;
	}
	data->contact_count = 0;
	data->index_count = 0;
	opimd_contacts_query(qry, _query_callback, data);
	g_hash_table_destroy(qry);
}


Evas_Object *
contact_list_add(struct ContactListViewData *data)
{
	data->list = elm_genlist_add(window_evas_object_get(data->win));
	elm_genlist_horizontal_mode_set(data->list, ELM_LIST_LIMIT);
	evas_object_size_hint_align_set(data->list, 0.0, 0.0);
	elm_widget_scale_set(data->list, 1.0);
	window_swallow(data->win, "list", data->list);
	itc.item_style = "contact";
	itc.func.label_get = gl_label_get;
	itc.func.icon_get = gl_icon_get;
	itc.func.state_get = gl_state_get;
	itc.func.del = gl_del;
	evas_object_show(data->list);

	data->index = elm_index_add(window_evas_object_get(data->win));
	//evas_object_size_hint_weight_set(data->index, 1.0, 1.0);
	//elm_win_resize_object_add(window_evas_object_get(data->win), data->index);
	window_swallow(data->win, "index", data->index);
	evas_object_show(data->index);
	evas_object_smart_callback_add(data->index, "delay,changed",
				       gl_index_changed2, NULL);
	evas_object_smart_callback_add(data->index, "changed", gl_index_changed,
				       NULL);
	evas_object_smart_callback_add(data->index, "selected",
				       gl_index_selected, NULL);
}
