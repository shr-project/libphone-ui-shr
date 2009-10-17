#include "views.h"
#include "common-utils.h"

#include <phoneui/phoneui-utility.h>

static void
  frame_list_show(void *_data);
static void
  frame_list_hide(void *_data);
static void
  frame_list_new_clicked(void *_data, Evas_Object * obj, void *event_info);
static void
  frame_list_call_clicked(void *_data, Evas_Object * obj, void *event_info);
static void
  frame_list_options_clicked(void *_data, Evas_Object * obj, void *event_info);
static void
  frame_list_message_clicked(void *_data, Evas_Object * obj, void *event_info);
static void
  frame_list_edit_clicked(void *_data, Evas_Object * obj, void *event_info);
static void
  frame_list_delete_clicked(void *_data, Evas_Object * obj, void *event_info);
static void
  frame_list_refresh(void *_data);


/* --- main contact list view ----------------------------------------------- */

void *
contact_list_view_show(struct Window *win, void *_options)
{
	struct ContactListViewData *data =
		g_slice_alloc0(sizeof(struct ContactListViewData));
	data->win = win;
	data->inwin = NULL;
	data->query = NULL;
	data->current_index = NULL;

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



static void
frame_list_show(void *_data)
{
	struct ContactListViewData *data = (struct ContactListViewData *) _data;
	struct Window *win = data->win;

	g_debug("frame_list_show()");

	window_layout_set(win, CONTACTS_FILE, "list");

	g_debug("adding extension theme '%s'", CONTACTLIST_FILE);
	elm_theme_extension_add(CONTACTLIST_FILE);

	contact_list_add(data);

	data->bt1 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt1, D_("New"));
	evas_object_smart_callback_add(data->bt1, "clicked",
				       frame_list_new_clicked, data);
	window_swallow(win, "button_new", data->bt1);
	evas_object_show(data->bt1);

	data->bt2 = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt2, D_("Call"));
	evas_object_smart_callback_add(data->bt2, "clicked",
				       frame_list_call_clicked, data);
	window_swallow(win, "button_call", data->bt2);
	evas_object_show(data->bt2);

	data->bt_options = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt_options, D_("Options"));
	evas_object_smart_callback_add(data->bt_options, "clicked",
				       frame_list_options_clicked, data);
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
	evas_object_smart_callback_add(data->bt_message, "clicked",
				       frame_list_message_clicked, data);
	evas_object_show(data->bt_message);
	elm_box_pack_end(data->bx, data->bt_message);

	data->bt_edit = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt_edit, D_("Show"));
	evas_object_size_hint_min_set(data->bt_edit, 130, 80);
	evas_object_smart_callback_add(data->bt_edit, "clicked",
				       frame_list_edit_clicked, data);
	evas_object_show(data->bt_edit);
	elm_box_pack_end(data->bx, data->bt_edit);

	data->bt_delete = elm_button_add(window_evas_object_get(win));
	elm_button_label_set(data->bt_delete, D_("Delete"));
	evas_object_size_hint_min_set(data->bt_delete, 130, 80);
	evas_object_smart_callback_add(data->bt_delete, "clicked",
				       frame_list_delete_clicked, data);
	evas_object_show(data->bt_delete);
	elm_box_pack_end(data->bx, data->bt_delete);

	elm_hover_content_set(data->hv, "top", data->bx);

	contact_list_fill(data);
	evas_object_smart_callback_add(data->list, "longpressed", 
			frame_list_edit_clicked, data);
}

static void
frame_list_hide(void *_data)
{
	struct ContactListViewData *data = (struct ContactListViewData *) _data;

	g_debug("frame_list_hide()");

	evas_object_del(data->bt1);
	evas_object_del(data->bt2);
	evas_object_del(data->bt_options);
	evas_object_del(data->list);
	evas_object_del(data->index);
}

static void
frame_list_new_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct Window *win = window_new(D_("New Contact"));
	window_init(win);
	window_view_show(win, NULL, contact_show_view_show,
			 contact_show_view_hide, NULL);
}

static void
frame_list_call_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactListViewData *data = (struct ContactListViewData *) _data;

	g_debug("frame_list_call_clicked()");

	Elm_Genlist_Item *it = elm_genlist_selected_item_get(data->list);
	GHashTable *properties = it ? elm_genlist_item_data_get(it) : NULL;

	if (properties != NULL) {
		GValue *tmp = g_hash_table_lookup(properties, "Phone");
		if (tmp) {
			char *number =
				common_utils_skip_prefix(g_value_get_string(tmp), "tel:");
			phoneui_call_initiate(number,
					    NULL, NULL);
		}
	}
}

static void
frame_list_options_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactListViewData *data = (struct ContactListViewData *) _data;

	g_debug("frame_list_options_clicked()");

	evas_object_show(data->hv);
}

static void
frame_list_message_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactListViewData *data = (struct ContactListViewData *) _data;

	g_debug("frame_list_message_clicked()");

	evas_object_hide(data->hv);

	Elm_Genlist_Item *it = elm_genlist_selected_item_get(data->list);
	GHashTable *properties = it ? elm_genlist_item_data_get(it) : NULL;

	if (properties != NULL) {
		const char *photo;
		GValue *tmp = g_hash_table_lookup(properties, "Phone");
		if (!tmp) {
			g_debug("contact needs a number to send a message ;)");
			return;
		}
		GHashTable *options = g_hash_table_new(g_str_hash, g_str_equal);
		g_hash_table_insert(options, "number",
				g_value_get_string(tmp));

		tmp = g_hash_table_lookup(properties, "Name");
		if (tmp) {
			g_hash_table_insert(options, "name",
				g_value_get_string(tmp));
		}
		tmp = g_hash_table_lookup(properties, "Photo");
		if (tmp) {
			g_hash_table_insert(options, "photo",
				g_value_get_string(tmp));
		}

		struct Window *win = window_new(D_("Compose SMS"));
		window_init(win);
		window_view_show(win, options, message_new_view_show,
				 message_new_view_hide, NULL);
	}
}

static void
frame_list_edit_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	Elm_Genlist_Item *it;
	struct ContactListViewData *data = (struct ContactListViewData *) _data;

	g_debug("frame_list_edit_clicked()");

	evas_object_hide(data->hv);
	if (event_info) {
		it = (Elm_Genlist_Item *)event_info;
	}
	else {
		it = elm_genlist_selected_item_get(data->list);
	}
	GHashTable *properties = it ? elm_genlist_item_data_get(it) : NULL;

	if (properties != NULL) {
		struct Window *win = window_new(D_("Show Contact"));
		window_init(win);
		window_view_show(win, properties, contact_show_view_show,
				 contact_show_view_hide, NULL);
	}
}


static void
_delete_ok_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	struct ContactListViewData *data = (struct ContactListViewData *)_data;

	Elm_Genlist_Item *it = elm_genlist_selected_item_get(data->list);
	GHashTable *properties = (it) ? elm_genlist_item_data_get(it) : NULL;

	char *path = g_value_get_string(
			g_hash_table_lookup(properties, "Path"));

	phoneui_contact_delete(path, NULL, NULL);

	elm_genlist_item_del(it);

	if (data->inwin) {
		evas_object_del(data->inwin);
		data->inwin = NULL;
	}
}



static void
_delete_no_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	struct ContactListViewData *data = (struct ContactListViewData *)_data;
	if (data->inwin) {
		evas_object_del(data->inwin);
		data->inwin = NULL;
	}
}



static void
frame_list_delete_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactListViewData *data = (struct ContactListViewData *) _data;

	g_debug("frame_list_delete_clicked()");

	evas_object_hide(data->hv);

	Elm_Genlist_Item *it = elm_genlist_selected_item_get(data->list);
	if (it) {
		struct InwinButton *btn;
		GList *buttons = NULL;

		btn = malloc(sizeof(struct InwinButton));
		btn->label = D_("Yes");
		btn->callback = _delete_ok_clicked;
		buttons = g_list_append(buttons, btn);

		btn = malloc(sizeof(struct InwinButton));
		btn->label = D_("No");
		btn->callback = _delete_no_clicked;
		buttons = g_list_append(buttons, btn);

		data->inwin = 
			window_inwin_dialog(data->win,
					D_("Really delete this contact?"),
					buttons, data);

	}
}

static void
frame_list_refresh(void *_data)
{
	g_debug("frame_list_refresh");
	struct ContactListViewData *data = (struct ContactListViewData *) _data;
	elm_genlist_clear(data->list);
	contact_list_fill(data);
}




