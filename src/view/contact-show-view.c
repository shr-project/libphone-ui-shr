#include "views.h"
#include "common-utils.h"


struct ContactFieldData {
	char *name;
	char *value;
};

struct ContactViewData {
	struct Window *win;
	Evas_Object *main, *name, *number;
	Evas_Object *photo;
	Evas_Object *list;
	Evas_Object *bt1, *bt2, *bt3;
	Evas_Object *hv1;
	Evas_Object *label_name, *label_number;
	Evas_Object *entry_name, *entry_number;
	Evas_Object *sc_name, *sc_number;
	Evas_Object *inwin;
	char *path;
	struct ContactFieldData *field;
	GHashTable *properties;
};

static void
  frame_show_show(void *_data);
static void
  frame_show_hide(void *_data);

static void
  frame_photo_show(void *_data);
static void
  frame_photo_hide(void *_data);

static void
  frame_edit_show(void *_data);
static void
  frame_edit_hide(void *_data);


static Elm_Genlist_Item_Class itc;

/* === frame "show" ============================================ */


static GValue *
_new_gvalue_string(const char *value)
{
	GValue *val = calloc(1, sizeof(GValue));
	if (!val) {
		return NULL;
	}
	g_value_init(val, G_TYPE_STRING);
	g_value_set_string(val, value);

	return val;
}


static char *
_get_entry(Evas_Object *obj)
{
	char *ret = elm_entry_entry_get(obj);
	ret = elm_entry_markup_to_utf8(ret); /* allocates! 8/
	return (ret);
}



/* --- smart callbacks --- */

static void
frame_show_close_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactViewData *data = (struct ContactViewData *) _data;
	window_destroy(data->win, NULL);
}


static void
frame_show_add_field_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactViewData *data = (struct ContactViewData *) _data;
	if (data->field) {
		g_free(data->field->name);
		g_free(data->field->value);
		g_free(data->field);
	}
	data->field = g_malloc(sizeof(struct ContactFieldData));
	data->field->name = g_strdup("");
	data->field->value = g_strdup("");
	window_frame_show(data->win, data, frame_edit_show, frame_edit_hide);
}


static void
frame_show_actions_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactViewData *data = (struct ContactViewData *) _data;
	evas_object_show(data->hv1);
}



static void
frame_show_action_call_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactViewData *data = (struct ContactViewData *) _data;
	GValue *tmp = g_hash_table_lookup(data->properties, "Phone");
	if (tmp) {
		char *number =
			common_utils_skip_prefix(g_value_get_string(tmp), "tel:");
		phonegui_call_initiate(number,
				    NULL, NULL);
	}
	evas_object_hide(data->hv1);
}


static char *
gl_field_label_get(const void *_data, Evas_Object * obj, const char *part)
{
	g_debug("gl_field_label_get (part=%s", part);
	struct ContactFieldData *data = (struct ContactFieldData *) _data;
	return (strdup(data->value));

}


static Evas_Object *
gl_field_icon_get(const void *_data, Evas_Object * obj, const char *part)
{
	g_debug("gl_field_icon_get (part=%s)", part);
	struct ContactFieldData *data = (struct ContactFieldData *) _data;
	Evas_Object *lbl = elm_label_add(obj);
	evas_object_size_hint_align_set(obj, 1.0, 0.0);
	elm_label_label_set(lbl, data->name);
	return (lbl);
}


static void
gl_field_del(const void *_data, Evas_Object * obj)
{
	g_debug("gl_field_del");
	struct ContactFieldData *data = (struct ContactFieldData *) _data;
	if (data) {
		if (data->name)
			free(data->name);
		if (data->value)
			free(data->value);
		free(data);
	}
}




static void
frame_show_action_sms_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactViewData *data = (struct ContactViewData *) _data;
	evas_object_hide(data->hv1);
	window_frame_show(data->win, data, frame_photo_show, frame_photo_hide);
}

static void
frame_show_name_clicked(void *_data, Evas_Object * obj, void *event_info)
{
}

static void
frame_show_number_clicked(void *_data, Evas_Object * obj, void *event_info)
{
}


static void
frame_show_photo_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	g_debug("you clicked on the Photo :-)");
}

static void
frame_show_edit_field(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactViewData *data = (struct ContactViewData *) _data;
	Elm_Genlist_Item *it = (Elm_Genlist_Item *)event_info;
	struct ContactFieldData *fd =
		(struct ContactFieldData *) elm_genlist_item_data_get(it);
	g_debug("editing field %s of %s", fd->name,
		data->path ? data->path : "new contact");
	data->field = g_slice_alloc0(sizeof(struct ContactFieldData));
	data->field->name = g_strdup(fd->name);
	data->field->value = g_strdup(fd->value);
	window_frame_show(data->win, data, frame_edit_show, frame_edit_hide);
}


static void
frame_show_show(void *_data)
{
	GValue *tmp;
	char *s;
	GHashTableIter iter;
	gpointer key, value;
	struct ContactViewData *data = (struct ContactViewData *) _data;
	Evas_Object *lbl;

	g_debug("frame_show_show()");

	window_layout_set(data->win, CONTACTS_FILE, "show");

	g_debug("loading name and number");
	/* --- name and number --- */
	if (data->path) {
		tmp = g_hash_table_lookup(data->properties, "Name");
		if (tmp)
			s = g_value_get_string(tmp);
		else
			s = CONTACT_NAME_UNDEFINED_STRING;
		window_text_set(data->win, "name", s);

		tmp = g_hash_table_lookup(data->properties, "Phone");
		if (tmp) {
			s = g_value_get_string(tmp);
			s = common_utils_skip_prefix(s, tmp);
		}
		else
			s = "";
		window_text_set(data->win, "number", s);
	}
	else {
		window_text_set(data->win, "name", CONTACT_NAME_UNDEFINED_STRING);
		window_text_set(data->win, "number", "");
	}


	g_debug("loading photo");
	/* --- photo --- */
	tmp = g_hash_table_lookup(data->properties, "Photo");
	if (tmp) {
		s = g_value_get_string(tmp);
		if (!strncmp(s, "file://", 7))
			s += 7;
	}
	else
		s = CONTACT_DEFAULT_PHOTO;
	data->photo = elm_icon_add(window_evas_object_get(data->win));
	elm_icon_file_set(data->photo, s, NULL);
	window_swallow(data->win, "photo", data->photo);
	evas_object_smart_callback_add(data->photo, "clicked",
				       frame_show_photo_clicked, data);
	evas_object_show(data->photo);

	g_debug("loading field list");
	/* --- list of fields --- */

	g_debug("adding extension theme '%s'", CONTACTLIST_FILE);
	elm_theme_extension_add(CONTACTLIST_FILE);

	data->list = elm_genlist_add(window_evas_object_get(data->win));
	elm_scroller_policy_set(data->list, ELM_SCROLLER_POLICY_OFF,
				ELM_SCROLLER_POLICY_AUTO);
	elm_genlist_horizontal_mode_set(data->list, ELM_LIST_LIMIT);
	evas_object_size_hint_align_set(data->list, 0.0, 0.0);
	elm_widget_scale_set(data->list, 1.0);

	itc.item_style = "contactfield";
	itc.func.label_get = gl_field_label_get;
	itc.func.icon_get = gl_field_icon_get;
	itc.func.state_get = NULL;
	itc.func.del = gl_field_del;

	window_swallow(data->win, "fields", data->list);
	evas_object_show(data->list);
	evas_object_smart_callback_add(data->list, "selected",
				       frame_show_edit_field, data);

	g_hash_table_iter_init(&iter, data->properties);
	while (g_hash_table_iter_next(&iter, &key, &value)) {

		if (!strcmp(key, "Path"))
			continue;

		g_debug("adding field %s='%s' to list", key,
			g_value_get_string(value));

		struct ContactFieldData *fd =
			g_malloc(sizeof(struct ContactFieldData));
		fd->name = g_strdup(key);
		fd->value = g_strdup(g_value_get_string(value));

		elm_genlist_item_append(data->list, &itc, fd, NULL,
					ELM_GENLIST_ITEM_NONE, NULL, NULL);
	}

	g_debug("setting up button bar");

	/* --- button bar --- */
	data->bt1 = elm_button_add(window_evas_object_get(data->win));
	elm_button_label_set(data->bt1, D_("Close"));
	evas_object_smart_callback_add(data->bt1, "clicked",
				       frame_show_close_clicked, data);
	window_swallow(data->win, "button_back", data->bt1);
	evas_object_show(data->bt1);

	data->bt2 = elm_button_add(window_evas_object_get(data->win));
	elm_button_label_set(data->bt2, D_("Add Field"));
	evas_object_smart_callback_add(data->bt2, "clicked",
				       frame_show_add_field_clicked, data);
	window_swallow(data->win, "button_edit", data->bt2);
	evas_object_show(data->bt2);

	data->bt3 = elm_button_add(window_evas_object_get(data->win));
	elm_button_label_set(data->bt3, D_("Actions"));
	evas_object_smart_callback_add(data->bt3, "clicked",
				       frame_show_actions_clicked, data);
	window_swallow(data->win, "button_actions", data->bt3);
	evas_object_show(data->bt3);

	data->hv1 = elm_hover_add(window_evas_object_get(data->win));
	elm_hover_parent_set(data->hv1, window_evas_object_get(data->win));
	elm_hover_target_set(data->hv1, data->bt3);

	Evas_Object *box1 = elm_box_add(window_evas_object_get(data->win));
	elm_box_horizontal_set(box1, 0);
	elm_box_homogenous_set(box1, 1);
	evas_object_show(box1);

	Evas_Object *box2 = elm_box_add(window_evas_object_get(data->win));
	evas_object_show(box2);

	Evas_Object *ico = elm_icon_add(window_evas_object_get(data->win));
	elm_icon_file_set(ico, ICON_CALL, NULL);
	evas_object_size_hint_aspect_set(ico, EVAS_ASPECT_CONTROL_VERTICAL, 1,
					 1);
	evas_object_show(ico);
	elm_box_pack_end(box2, ico);

	Evas_Object *btn = elm_button_add(window_evas_object_get(data->win));
	elm_button_label_set(btn, D_("Call"));
	evas_object_size_hint_min_set(btn, 130, 80);
	evas_object_smart_callback_add(btn, "clicked",
				       frame_show_action_call_clicked, data);
	evas_object_show(btn);
	elm_box_pack_end(box2, btn);

	elm_box_pack_end(box1, box2);

	box2 = elm_box_add(window_evas_object_get(data->win));
	evas_object_show(box2);

	ico = elm_icon_add(window_evas_object_get(data->win));
	elm_icon_file_set(ico, ICON_SMS, NULL);
	evas_object_size_hint_aspect_set(ico, EVAS_ASPECT_CONTROL_VERTICAL, 1,
					 1);
	evas_object_show(ico);
	elm_box_pack_end(box2, ico);

	btn = elm_button_add(window_evas_object_get(data->win));
	elm_button_label_set(btn, D_("SMS"));
	evas_object_size_hint_min_set(btn, 130, 80);
	evas_object_smart_callback_add(btn, "clicked",
				       frame_show_action_sms_clicked, data);
	evas_object_show(btn);
	elm_box_pack_end(box2, btn);

	elm_box_pack_end(box1, box2);

	elm_hover_content_set(data->hv1, "top", box1);

	g_debug("frame_show_show DONE");
}


static void
frame_show_hide(void *_data)
{
	g_debug("frame_show_hide");
	struct ContactViewData *data = (struct ContactViewData *) _data;
	evas_object_del(data->bt1);
	evas_object_del(data->bt2);
	evas_object_del(data->bt3);
	evas_object_del(data->hv1);
	evas_object_del(data->photo);
	evas_object_del(data->list);
	g_debug("frame_show_hide DONE");
}



/* === frame "photo" ===================================== */

static void
frame_photo_back_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactViewData *data = (struct ContactViewData *) _data;
	window_frame_show(data->win, data, frame_show_show, frame_show_hide);
}


static void
frame_photo_select_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactViewData *data = (struct ContactViewData *) _data;
	window_frame_show(data->win, data, frame_show_show, frame_show_hide);
}


static void
frame_photo_remove_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactViewData *data = (struct ContactViewData *) _data;
	window_frame_show(data->win, data, frame_show_show, frame_show_hide);
}


static void
frame_photo_show(void *_data)
{
	GValue *tmp;
	char *s;

	g_debug("frame_photo_show");
	struct ContactViewData *data = (struct ContactViewData *) _data;

	window_layout_set(data->win, CONTACTS_FILE, "edit_photo");

	tmp = g_hash_table_lookup(data->properties, "Photo");
	if (tmp) {
		s = g_value_get_string(tmp);
		if (!strncmp(s, "file://", 7))
			s += 7;
	}
	else
		s = "";

	window_text_set(data->win, "file", s);

	data->photo = elm_icon_add(window_evas_object_get(data->win));
	elm_icon_file_set(data->photo, s, NULL);
	window_swallow(data->win, "photo", data->photo);
	evas_object_show(data->photo);

	data->list = elm_fileselector_add(window_evas_object_get(data->win));
	elm_fileselector_path_set(data->list, s);
	window_swallow(data->win, "selector", data->list);
	evas_object_show(data->list);

	data->bt1 = elm_button_add(window_evas_object_get(data->win));
	elm_button_label_set(data->bt1, D_("Back"));
	window_swallow(data->win, "button_back", data->bt1);
	evas_object_smart_callback_add(data->bt1, "clicked",
				       frame_photo_back_clicked, data);
	evas_object_show(data->bt1);

	data->bt2 = elm_button_add(window_evas_object_get(data->win));
	elm_button_label_set(data->bt2, D_("Back"));
	window_swallow(data->win, "button_select", data->bt2);
	evas_object_smart_callback_add(data->bt2, "clicked",
				       frame_photo_select_clicked, data);
	evas_object_show(data->bt2);

	data->bt3 = elm_button_add(window_evas_object_get(data->win));
	elm_button_label_set(data->bt3, D_("Back"));
	window_swallow(data->win, "button_remove", data->bt3);
	evas_object_smart_callback_add(data->bt3, "clicked",
				       frame_photo_remove_clicked, data);
	evas_object_show(data->bt3);
}



static void
frame_photo_hide(void *_data)
{
	struct ContactViewData *data = (struct ContactViewData *) _data;

	evas_object_del(data->photo);
	evas_object_del(data->list);
	evas_object_del(data->bt1);
	evas_object_del(data->bt2);
	evas_object_del(data->bt3);
}


/* === frame "edit" ====================================== */



static char *
frame_edit_value_get(Evas_Object * entry)
{
	g_debug("frame_edit_value_get()");

	char *value;
	value = g_strstrip(elm_entry_markup_to_utf8(elm_entry_entry_get(entry)));

	return (value);
}


gboolean
frame_edit_data_changed(struct ContactViewData * data)
{
	if (strcmp(data->field->name, _get_entry(data->entry_name)))
		return (TRUE);
	if (strcmp(data->field->value, _get_entry(data->entry_number)))
		return (TRUE);

	return (FALSE);
}


static void
_load_show(void *_data)
{
	struct ContactViewData *data = (struct ContactViewData *)_data;
	window_frame_show(data->win, data, frame_show_show, frame_show_hide);
}


static void
_on_new_saved(GError *error, const char *path, void *_data)
{
	struct ContactViewData *data = (struct ContactViewData *)_data;

	if (!error) {
		data->path = g_strdup(path);
	}

	async_trigger(_load_show, data);
}


static void
frame_edit_save_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactViewData *data = (struct ContactViewData *) _data;

	g_debug("frame_edit_save_clicked()");

	if (frame_edit_data_changed(data)) {
		g_debug("---> %s needs saving (%s=%s)", data->path, data->field->name, data->field->value);
		const char *name = _get_entry(data->entry_name);
		const char *value = _get_entry(data->entry_number);

		/* if the name of the field changed we have to delete
		 * the old field by setting it to an empty string */
		if (strcmp(name, data->field->name) != 0) {
			g_debug("field name changed... removing the old one");
			g_hash_table_insert(data->properties, 
					g_strdup(data->field->name),
					_new_gvalue_string(""));
		}

		g_hash_table_insert(data->properties, g_strdup(name),
				    _new_gvalue_string(value));

		if (data->path)
			opimd_contact_update(data->path, data->properties, NULL,
					     NULL);
		else {
			opimd_contacts_add(data->properties, _on_new_saved, data);
			/* for new contacts we have to get the path for the
			 * contact via the dbus callback... return here and
			 * load the show frame via the callback */
			return;
		}
	}

	window_frame_show(data->win, data, frame_show_show, frame_show_hide);
}



static void
frame_no_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactViewData *data = (struct ContactViewData *) _data;
	if (data->inwin) {
		evas_object_del(data->inwin);
		data->inwin = NULL;
	}
}


static void
frame_yes_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct ContactViewData *data = (struct ContactViewData *) _data;
	if (data->inwin) {
		evas_object_del(data->inwin);
		data->inwin = NULL;
	}
	window_frame_show(data->win, data, frame_show_show, frame_show_hide);
}



static void
frame_edit_close_clicked(void *_data, Evas_Object * obj, void *event_info)
{
	struct InwinButton *btn;
	GList *buttons = NULL;
	struct ContactViewData *data = (struct ContactViewData *) _data;

	g_debug("frame_edit_close_clicked()");

	if (frame_edit_data_changed(data)) {
		btn = malloc(sizeof(struct InwinButton));
		btn->label = D_("Yes");
		btn->callback = frame_yes_clicked;
		buttons = g_list_append(buttons, btn);
		btn = malloc(sizeof(struct InwinButton));
		btn->label = D_("No");
		btn->callback = frame_no_clicked;
		buttons = g_list_append(buttons, btn);

		data->inwin =
			window_inwin_dialog(data->win,
					    D_("Really discard all changes?"),
					    buttons, data);

		return;
	}

	window_frame_show(data->win, data, frame_show_show, frame_show_hide);
}





static void
frame_edit_show(void *_data)
{
	GValue *tmp;
	struct ContactViewData *data = (struct ContactViewData *) _data;

	g_debug("frame_edit_show()");

	window_layout_set(data->win, CONTACTS_FILE, "edit");

	window_text_set(data->win, "title", D_("Edit Field"));

	data->bt1 = elm_button_add(data->win->win);
	elm_button_label_set(data->bt1, D_("Back"));
	evas_object_smart_callback_add(data->bt1, "clicked",
				       frame_edit_close_clicked, data);
	window_swallow(data->win, "button_back", data->bt1);
	evas_object_show(data->bt1);

	data->bt2 = elm_button_add(data->win->win);
	elm_button_label_set(data->bt2, D_("Save"));
	evas_object_smart_callback_add(data->bt2, "clicked",
				       frame_edit_save_clicked, data);
	window_swallow(data->win, "button_save", data->bt2);
	evas_object_show(data->bt2);

	data->label_name = elm_label_add(data->win->win);
	elm_label_label_set(data->label_name, D_("Field: "));
	window_swallow(data->win, "label_name", data->label_name);
	evas_object_show(data->label_name);

	data->sc_name = elm_scroller_add(data->win->win);
	data->entry_name = elm_entry_add(data->win->win);
	evas_object_size_hint_weight_set(data->entry_name, 1.0, 1.0);
	evas_object_size_hint_align_set(data->entry_name, -1.0, -1.0);
	elm_entry_entry_set(data->entry_name, data->field->name);
	//elm_widget_focus_set(data->entry_name, 1);

	elm_scroller_content_set(data->sc_name, data->entry_name);
	evas_object_show(data->entry_name);

	window_swallow(data->win, "entry_name", data->sc_name);
	evas_object_show(data->sc_name);

	data->label_number = elm_label_add(data->win->win);
	elm_label_label_set(data->label_number, D_("Value: "));
	window_swallow(data->win, "label_number", data->label_number);
	evas_object_show(data->label_number);

	data->sc_number = elm_scroller_add(data->win->win);
	data->entry_number = elm_entry_add(data->win->win);
	evas_object_size_hint_weight_set(data->entry_number, 1.0, 1.0);
	evas_object_size_hint_align_set(data->entry_number, -1.0, -1.0);
	elm_entry_entry_set(data->entry_number, data->field->value);

	elm_scroller_content_set(data->sc_number, data->entry_number);
	evas_object_show(data->entry_number);

	window_swallow(data->win, "entry_number", data->sc_number);
	evas_object_show(data->sc_number);

	/* set focus to the data if name of the
	 * field is not empty */
	if (*data->field->name)
		elm_object_focus(data->entry_number);
	else
		elm_object_focus(data->entry_name);
}


static void
frame_edit_hide(void *_data)
{
	struct ContactViewData *data = (struct ContactViewData *) _data;

	g_debug("frame_edit_hide()");

	evas_object_del(data->bt1);
	evas_object_del(data->bt2);
	evas_object_del(data->entry_name);
	evas_object_del(data->entry_number);
	evas_object_del(data->sc_name);
	evas_object_del(data->sc_number);
	evas_object_del(data->label_name);
	evas_object_del(data->label_number);

	g_slice_free1(sizeof(struct ContactFieldData), data->field);
	data->field = NULL;
}





/* === main contact view ================================= */

void *
contact_show_view_show(struct Window *win, void *_data)
{
	g_debug("contact_view_show()");

	struct ContactViewData *data =
		g_slice_alloc0(sizeof(struct ContactViewData));
	data->win = win;
	if (_data) {
		data->properties = (GHashTable *) _data;
		GValue *tmp = g_hash_table_lookup(data->properties, "Path");
		if (tmp)
			data->path = g_value_get_string(tmp);
		else
			data->path = NULL;
	}
	else {
		data->properties =
			g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
					      free);
		g_hash_table_insert(data->properties, "Name", 
				_new_gvalue_string(""));
		g_hash_table_insert(data->properties, "Phone", 
				_new_gvalue_string(""));
		data->path = NULL;
	}
	data->field = NULL;

	window_frame_show(win, data, frame_show_show, frame_show_hide);
	window_show(win);

	return (data);
}


void
contact_show_view_hide(void *_data)
{
	//struct ContactViewData *data = (struct ContactViewData *)_data;

	g_debug("contact_view_hide()");

}
