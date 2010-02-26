
#include "views.h"

#include "common-utils.h"
#include <ctype.h> /* to upper */

static Elm_Genlist_Item_Class itc;

/* --- genlist callbacks --- */
static char *
gl_label_get(const void *data, Evas_Object * obj, const char *part)
{
	GHashTable *parameters = (GHashTable *) data;
	char *s = NULL;

	if (!strcmp(part, "elm.text")) {
		s = phoneui_utils_contact_display_name_get(parameters);
		if (s && *s) {
			return s;
		}
		else {
			s = strdup(CONTACT_NAME_UNDEFINED_STRING);
		}
	}
	else if (!strcmp(part, "elm.text.sub")) {
		s = phoneui_utils_contact_display_phone_get(parameters);
		if (s && *s) {
			return s;
		}
		else {
			s = strdup(CONTACT_PHONE_UNDEFINED_STRING);
		}
	}

	return s;
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
		Evas_Object *photo = elm_image_add(obj);
		elm_image_file_set(photo, photo_file, NULL);
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

Elm_Genlist_Item *
contact_list_item_add(struct ContactListViewData *view,
		      GHashTable *entry, int sortin)
{
	GHashTable *other;
	Elm_Genlist_Item *it;

	if (sortin) {
		/* find the correct position to insert the new one */
		it = elm_genlist_first_item_get(view->list);
		while (it) {
			other = (GHashTable *)elm_genlist_item_data_get(it);
			if (phoneui_utils_contact_compare(entry, other) < 0)
				break;
			it = elm_genlist_item_next_get(it);
		}
		if (it) {
			g_debug("Found a contact to insert before");
			return elm_genlist_item_insert_before(view->list, &itc,
					g_hash_table_ref(entry), it,
					ELM_GENLIST_ITEM_NONE, NULL, NULL);
		}
	}
	g_debug("Appending contact to the end of the list");
	return elm_genlist_item_append(view->list, &itc,
				     g_hash_table_ref(entry) /*item data */ ,
				     NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
}

void
contact_list_fill_index(struct ContactListViewData *view)
{
	/* FIXME: limit to 13 indexes - BAD, gotta find a way to calculate
	 * this, furthermore, should probably choose the best indexes better */
	static int limit = 13;
	Evas_Object *win;
	Elm_Genlist_Item *it, *current_index_item;
	GHashTable *entry;
	char *idx, *current_index = NULL;
	char *name;
	int contact_count;
	int index_count;
	int new_index;

	win = ui_utils_view_window_get(VIEW_PTR(*view));
	if (view->index) {
		evas_object_del(view->index);
	}
	view->index = elm_index_add(win);
	elm_win_resize_object_add(win, view->index);
 	ui_utils_view_swallow(VIEW_PTR(*view), "index", view->index);
	evas_object_size_hint_weight_set(view->index, 1.0, 0.0);
	evas_object_show(view->index);
	evas_object_smart_callback_add(view->index, "delay,changed",
				       gl_index_changed2, NULL);
	evas_object_smart_callback_add(view->index, "changed", gl_index_changed,
				       NULL);
	evas_object_smart_callback_add(view->index, "selected",
				       gl_index_selected, NULL);
	index_count = view->contact_count / limit;
	it = elm_genlist_first_item_get(view->list);
	while (it) {
		entry = (GHashTable *)elm_genlist_item_data_get(it);
		name = phoneui_utils_contact_display_name_get(entry);
		idx = _new_get_index(name);

		if (!current_index || strcmp(idx, current_index)) {
			if (current_index) {
				free(current_index);
			}
			current_index = idx;
			current_index_item = it;
			new_index = 1;
		}
		else {
			new_index = 0;
			free(idx);
		}
		if (index_count < 1 && new_index) {
			g_debug("Adding index %s", current_index);
			elm_index_item_append(view->index, current_index,
						current_index_item);
			index_count = view->contact_count / limit;
		}
		index_count--;
		it = elm_genlist_item_next_get(it);
	}
}

static void
_process_entry(void *_entry, void *_data)
{
	Elm_Genlist_Item *it;
	GHashTable *entry = (GHashTable *)_entry;
	struct ContactListViewData *data = (struct ContactListViewData *) _data;
	it = contact_list_item_add(data, entry, 0);
	if (!it) {
		g_warning("Failed adding a contact to the list");
		return;
	}

	data->contact_current++;
	if (data->contact_count == data->contact_current) {
		contact_list_fill_index(data);
	}
}

void
contact_list_fill(struct ContactListViewData *data)
{
	g_debug("contact_list_fill()");
	data->contact_current = 0;
	phoneui_utils_contacts_get(&data->contact_count, _process_entry, data);
}


Evas_Object *
contact_list_add(struct ContactListViewData *data)
{
	Evas_Object *win = ui_utils_view_window_get(VIEW_PTR(*data));
	data->list = elm_genlist_add(win);
	elm_genlist_horizontal_mode_set(data->list, ELM_LIST_LIMIT);
	evas_object_size_hint_align_set(data->list, 0.0, 0.0);
	elm_object_scale_set(data->list, 1.0);
	ui_utils_view_swallow(VIEW_PTR(*data), "list", data->list);
	itc.item_style = "contact";
	itc.func.label_get = gl_label_get;
	itc.func.icon_get = gl_icon_get;
	itc.func.state_get = gl_state_get;
	itc.func.del = gl_del;
	evas_object_show(data->list);

}

