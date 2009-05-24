
#include "views.h"


struct ContactEditViewData {
	struct Window *win;
	Evas_Object *bt1, *bt2;
	Evas_Object *label_name, *label_number;
	Evas_Object *entry_name, *entry_number;
	Evas_Object *sc_name, *sc_number;
	Evas_Object *loading, *close_information;

	char *path, *name, *number;
	void (*callback)();
	void *callback_data;
	gboolean saved;

	GPtrArray *contacts;
};



static void frame_edit_show(void *_data);
static void frame_edit_hide(void *_data);
static void frame_edit_save_clicked(void *_data, Evas_Object *obj, void *event_info);
static void frame_edit_close_clicked(void *_data, Evas_Object *obj, void *event_info);
static char* frame_edit_value_get(Evas_Object *entry);
static gboolean frame_edit_needs_saving(struct ContactEditViewData *data);
static void frame_edit_save_new_callback(GError *error, char *path, void *_data);
static void frame_edit_save_update_callback(GError *error, void *_data);
static void frame_edit_save_callback2(struct ContactEditViewData *data);

static void frame_close_show(void *_data);
static void frame_close_hide(void *_data);
static void frame_close_yes_clicked(void *data, Evas_Object *obj, void *event_info);
static void frame_close_no_clicked(void *data, Evas_Object *obj, void *event_info);

static void frame_loading_show(void *data);
static void frame_loading_hide(void *data);


/* --- main contact edit view ----------------------------------------------------- */

void *contact_edit_view_show(struct Window *win, void *_options)
{
	GHashTable *options = (GHashTable *)_options;

	g_debug("contact_edit_view_show()");

	struct ContactEditViewData *data = g_slice_alloc0(sizeof(struct ContactEditViewData));
	data->win = win;
	data->path = NULL;
	data->name = NULL;
	data->number = NULL;
	data->saved = FALSE;
	data->callback = NULL;
	data->callback_data = NULL;

	if (options != NULL) {
		if (g_hash_table_lookup(options, "name")) {
			data->name = strdup(g_hash_table_lookup(options, "name"));
			data->path = strdup(g_hash_table_lookup(options, "path"));
		}

		if (g_hash_table_lookup(options, "number"))
			data->number = strdup(g_hash_table_lookup(options, "number"));

		data->callback = g_hash_table_lookup(options, "change_callback"); 
		data->callback_data = g_hash_table_lookup(options, "change_callback_data"); 
	}

	window_frame_show(win, data, frame_edit_show, frame_edit_hide);
	window_show(win);

	return data;
}

void contact_edit_view_hide(void *_data)
{
	struct ContactEditViewData *data = (struct ContactEditViewData *)_data;

	g_debug("contact_edit_view_hide()");

	if (data->saved && data->callback != NULL) {
		g_debug("calling data->callback...");
		data->callback(data->callback_data);
	}

	g_debug("deleting ContactEditViewData");
	//g_slice_free(struct ContactEditViewData, data);
}





/* --- frame "edit" --------------------------------------------------------------- */

static void frame_edit_show(void *_data)
{
	struct ContactEditViewData *data = (struct ContactEditViewData *)_data;

	g_debug("frame_edit_show()");

	//elm_win_keyboard_mode_set(data->win->win, ELM_WIN_KEYBOARD_ALPHA);

	window_layout_set(data->win, CONTACTS_FILE, "edit");
	if (data->path)
		window_text_set(data->win, "title", D_("Edit Contact"));
	else
		window_text_set(data->win, "title", D_("New Contact"));

	data->bt1 = elm_button_add(data->win->win);
	elm_button_label_set(data->bt1, D_("Close"));
	evas_object_smart_callback_add(data->bt1, "clicked", frame_edit_close_clicked, data);
	window_swallow(data->win, "button_back", data->bt1);
	evas_object_show(data->bt1);

	data->bt2 = elm_button_add(data->win->win);
	elm_button_label_set(data->bt2, D_("Save"));
	evas_object_smart_callback_add(data->bt2, "clicked", frame_edit_save_clicked, data);
	window_swallow(data->win, "button_save", data->bt2);
	evas_object_show(data->bt2);

	data->label_name = elm_label_add(data->win->win);
	elm_label_label_set( data->label_name,  D_("Name: "));
	window_swallow(data->win, "label_name", data->label_name);
	evas_object_show(data->label_name);

	data->sc_name = elm_scroller_add(data->win->win);
	data->entry_name = elm_entry_add(data->win->win);
	if (data->name)
		elm_entry_entry_set(data->entry_name, data->name);
	elm_widget_focus_set(data->entry_name, 1);

	elm_scroller_content_set(data->sc_name, data->entry_name);
	evas_object_show(data->entry_name);

	window_swallow(data->win, "entry_name", data->sc_name);
	evas_object_show(data->sc_name);

	data->label_number = elm_label_add(data->win->win);
	elm_label_label_set( data->label_number,  D_("Number: "));
	window_swallow(data->win, "label_number", data->label_number);
	evas_object_show(data->label_number);

	data->sc_number = elm_scroller_add(data->win->win);
	data->entry_number = elm_entry_add(data->win->win);
	if (data->number)
		elm_entry_entry_set(data->entry_number, data->number);

	elm_scroller_content_set(data->sc_number, data->entry_number);
	evas_object_show(data->entry_number);

	window_swallow(data->win, "entry_number", data->sc_number);
	evas_object_show(data->sc_number);

	elm_object_focus(data->entry_name);
}


static void frame_edit_hide(void *_data)
{
	struct ContactEditViewData *data = (struct ContactEditViewData *)_data;

	g_debug("frame_edit_hide()");

	if (data->name)
		free(data->name);
	if (data->number)
		free(data->number);

	data->name = frame_edit_value_get(data->entry_name);
	data->number = frame_edit_value_get(data->entry_number);

	evas_object_del(data->bt1);
	evas_object_del(data->bt2);
	evas_object_del(data->entry_name);
	evas_object_del(data->entry_number);
	evas_object_del(data->sc_name);
	evas_object_del(data->sc_number);
	evas_object_del(data->label_name);
	evas_object_del(data->label_number);

	//elm_win_keyboard_win_set(data->win->win, 0);
}


static void frame_edit_save_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	struct ContactEditViewData *data = (struct ContactEditViewData *)_data;

	g_debug("frame_edit_save_clicked()");

	if (!frame_edit_needs_saving(data)) {
		window_destroy(data->win, NULL);
		return;
	}

	GHashTable *contact_data = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
	GValue *value = g_slice_alloc0(sizeof(GValue));
	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, frame_edit_value_get(data->entry_number));
	g_hash_table_insert(contact_data, "Phone", value);
	value = g_slice_alloc0(sizeof(GValue));
	g_value_init(value, G_TYPE_STRING);
	g_value_set_string(value, frame_edit_value_get(data->entry_name));
	g_hash_table_insert(contact_data, "Name", value);

	if (data->path)
		opimd_contact_update(data->path, contact_data, frame_edit_save_update_callback, data);
	else
		opimd_contacts_add(contact_data, frame_edit_save_new_callback, data);

	data->saved = TRUE;

	window_frame_show(data->win, data, frame_loading_show, frame_loading_hide);
}


static void frame_edit_save_new_callback(GError *error, char *path, void *_data)
{
	struct ContactEditViewData *data = (struct ContactEditViewData *)_data;
	g_debug("frame_edit_save_new_callback() --> %s", path);
	async_trigger(frame_edit_save_callback2, data);
}

static void frame_edit_save_update_callback(GError *error, void *_data)
{
	struct ContactEditViewData *data = (struct ContactEditViewData *)_data;
	g_debug("frame_edit_save_update_callback()");
	async_trigger(frame_edit_save_callback2, data);
}

static void frame_edit_save_callback2(struct ContactEditViewData *data)
{
	g_debug("frame_edit_save_callback2()");
	if (data->name)
		free(data->name);
	if (data->number)
		free(data->number);
	window_destroy(data->win, NULL);

}


static void frame_edit_close_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	struct ContactEditViewData *data = (struct ContactEditViewData *)_data;

	g_debug("frame_edit_close_clicked()");

	if (frame_edit_needs_saving(data))
		window_frame_show(data->win, data, frame_close_show, frame_close_hide);
	else
		window_destroy(data->win, NULL);
}


static char* frame_edit_value_get(Evas_Object *entry)
{
	g_debug("frame_edit_value_get()");

	char *value = g_strstrip(strdup(elm_entry_entry_get(entry)));
	string_strip_html(value);

	return (value);
}


static gboolean frame_edit_needs_saving(struct ContactEditViewData *data)
{
	gboolean ret = TRUE;
	char *temp_name = frame_edit_value_get(data->entry_name);
	char *temp_number = frame_edit_value_get(data->entry_number);
	if (!strlen(temp_name) || !strlen(temp_number) || !string_is_number(temp_number))
		ret = FALSE;
	else if (data->name && (strcmp(temp_name, data->name) == 0) && data->number && (strcmp(temp_number, data->number) == 0))
		ret = FALSE;
	free (temp_name);
	free (temp_number);

	return (ret);
}

/*void contacts_sim_full_show(struct ContactEditViewData *data) 
  {
  window_layout_set(win, UI_FILE, "dialog");
  window_text_set(win, "content", "Your storage is full. Before adding new contacts, you have to delete some old ones.");

  data->bt1 = elm_button_add(window_evas_object_get(win));
  elm_button_label_set(data->bt1, "Close");
  evas_object_smart_callback_add(data->bt1, "clicked", contacts_button_close_clicked, win);
  window_swallow(win, "button_close", data->bt1);
  evas_object_show(data->bt1);
  }

  void contacts_sim_full_hide(struct ContactEditViewData *data) 
  {
  window_unswallow(win, data->bt1);
  evas_object_del(data->bt1);
  }*/




/* --- frame "close" (confirm changes) -------------------------------------------- */

static void frame_close_show(void *_data)
{
	struct ContactEditViewData *data = (struct ContactEditViewData *)_data;

	g_debug("frame_close_show()");

	window_layout_set(data->win, DIALOG_FILE, "close");

	data->close_information = elm_label_add(data->win->win);
	elm_label_label_set(data->close_information,  D_("Do you really want to quit?"));
	window_swallow(data->win, "information", data->close_information);
	evas_object_show(data->close_information);

	data->bt1 = elm_button_add(data->win->win);
	elm_button_label_set(data->bt1, D_("Yes"));
	evas_object_smart_callback_add(data->bt1, "clicked", frame_close_yes_clicked, data);
	window_swallow(data->win, "button_yes", data->bt1);
	evas_object_show(data->bt1);

	data->bt2 = elm_button_add(data->win->win);
	elm_button_label_set(data->bt2, D_("No"));
	evas_object_smart_callback_add(data->bt2, "clicked", frame_close_no_clicked, data);
	window_swallow(data->win, "button_no", data->bt2);
	evas_object_show(data->bt2);
}

static void frame_close_hide(void *_data)
{
	struct ContactEditViewData *data = (struct ContactEditViewData *)_data;

	g_debug("frame_close_hide()");

	evas_object_del(data->bt1);
	evas_object_del(data->bt2);
	evas_object_del(data->close_information);
}

static void frame_close_yes_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	struct ContactEditViewData *data = (struct ContactEditViewData *)_data;

	g_debug("frame_close_yes_clicked()");

	window_destroy(data->win, NULL);
}

static void frame_close_no_clicked(void *_data, Evas_Object *obj, void *event_info)
{
	struct ContactEditViewData *data = (struct ContactEditViewData *)_data;
	window_frame_show(data->win, data, frame_edit_show, frame_edit_hide);
}


/*
 * Frame "loading"
 */

static void frame_loading_show(void *_data) 
{
	struct ContactEditViewData *data = (struct ContactEditViewData *)_data;
	window_layout_set(data->win, CONTACTS_FILE, "loading");
	data->loading = elm_label_add( window_evas_object_get(data->win) );
	elm_label_label_set(data->loading,  D_("Loading... "));
	window_swallow(data->win, "text", data->loading);
	evas_object_show(data->loading);
}

static void frame_loading_hide(void *_data) 
{
	struct ContactEditViewData *data = (struct ContactEditViewData *)_data;
	evas_object_del(data->loading);
}



