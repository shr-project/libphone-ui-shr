#include "sim-manager-view.h"
#include "ui-utils.h"

struct SimManagerListData {
	struct View *view;
	Evas_Object *layout;
	Evas_Object *list;
	Evas_Object *index;
	int count;
	int current;
};

struct SimManagerViewData {
	struct View view;
	struct SimMangerListData list_data;
	Evas_Object *bx, *hv;
	Evas_Object *bt_import_all, *bt_options, *bt_message, *bt_edit;
	Evas_Object *bt_delete;
};
static struct SimManagerViewData view;

static Elm_Genlist_Item_Class itc;

/* --- genlist callbacks --- */
static char *
gl_label_get(const void *data, Evas_Object * obj, const char *part)
{
	(void) obj;
	GValueArray *param = (GValueArray *) data;
	char *s = NULL;

	if (!strcmp(part, "elm.text")) {
		s = g_value_get_string(g_value_array_get_nth(param, 1));
		if (s && *s) {
			return s;
		}
		else {
			s = strdup(CONTACT_NAME_UNDEFINED_STRING);
		}
	}
	else if (!strcmp(part, "elm.text.sub")) {
		s = g_value_get_string(g_value_array_get_nth(param, 2));
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
	if (!strcmp(part, "elm.swallow.icon")) {
		const char *photo_file;
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
	(void) obj;
	(void) data;
	(void) part;
	return (0);
}


static void
gl_del(const void *data, Evas_Object * obj)
{
	(void) obj;
	(void) data;
}

static void
_list_import_all_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
}

static void
_list_options_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	evas_object_show(view.hv);
}

static void
_contact_delete_confirm_cb(int result, void *data)
{
	if (result != DIALOG_YES)
		return;

	Elm_Genlist_Item *it = (Elm_Genlist_Item *)data;
	GHashTable *properties =
		(it) ? (GHashTable *) elm_genlist_item_data_get(it) : NULL;
	if (properties) {
		int index = g_value_get_int(g_value_array_get_nth(properties,
								  0));
		// TODO: use a callback to show success/failure
		phoneui_utils_sim_contact_delete(index, NULL, NULL);
	}
}

static void
_list_delete_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	Elm_Genlist_Item *it;

	evas_object_hide(view.hv);

	it = elm_genlist_selected_item_get(view.list_data.list);
	if (it) {
		ui_utils_dialog(VIEW_PTR(view),
			D_("Really delete this contact?"),
			DIALOG_YES|DIALOG_NO, _contact_delete_confirm_cb, it);
	}
}

void
sim_manager_list_add(struct SimManagerListData *list_data)
{
	Evas_Object *win;
	win = ui_utils_view_window_get(list_data->view);
	list_data->index = NULL;
	list_data->list = elm_genlist_add(win);
	elm_genlist_horizontal_mode_set(list_data->list, ELM_LIST_LIMIT);
	evas_object_size_hint_align_set(list_data->list, 0.0, 0.0);
	elm_object_scale_set(list_data->list, 1.0);
	itc.item_style = "sim-contact";
	itc.func.label_get = gl_label_get;
	itc.func.state_get = gl_state_get;
	itc.func.del = gl_del;
	evas_object_show(list_data->list);
	if (list_data->layout) {
		elm_layout_content_set(list_data->layout, "contacts_list",
				list_data->list);
	}
}

Elm_Genlist_Item *
sim_manager_list_item_add(struct SimManagerListData *list_data,
			GValueArray *entry)
{
	return elm_genlist_item_append(list_data->list, &itc,
				     g_hash_table_ref(entry) /*item data */ ,
				     NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
}

static void
_process_entry(void *_entry, void *_data)
{
	Elm_Genlist_Item *it;
	GValueArray *entry = (GValueArray *)_entry;
	struct SimManagerListData *list_data =
				(struct SimManagerListData *) _data;
	it = sim_manager_list_item_add(list_data, entry);
	if (!it) {
		g_warning("Failed adding a contact to the list");
		return;
	}
}
void
sim_manager_list_fill(struct ContactListData *list_data)
{
	g_debug("sim_manager_list_fill()");
	list_data->current = 0;
	phoneui_utils_sim_manager_get(&list_data->count, _process_entry,
				      list_data);
}

int
sim_manager_view_init()
{
	Evas_Object *win;
	int ret;
	ret = ui_utils_view_init(VIEW_PTR(view), ELM_WIN_BASIC,
				 D_("SIM Manager"), NULL, NULL, NULL);

	if (ret) {
		g_critical("Failed to init sim manager view");
		return ret;
	}

	win = ui_utils_view_window_get(VIEW_PTR(view));
	ui_utils_view_delete_callback_set(VIEW_PTR(view), _delete_cb);

	ui_utils_view_layout_set(VIEW_PTR(view), DEFAULT_THEME,
				 "phoneui/settings/sim-manager");
        elm_theme_extension_add(DEFAULT_THEME);
	view.list_data.layout = view.view.layout;
	sim_manager_list_add(&view.list_data);

	view.bt_import_all = elm_button_add(win);
	elm_button_label_set(view.bt_import_all, D_("Import all"));
	evas_object_smart_callback_add(view.bt_import_all, "clicked",
				       _list_import_all_clicked, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_import_all",
			      view.bt_import_all);
	evas_object_show(view.bt_import_all);

	view.bt_options = elm_button_add(win);
	elm_button_label_set(view.bt_options, D_("Options"));
	evas_object_smart_callback_add(view.bt_options, "clicked",
				       _list_options_clicked, NULL);
	ui_utils_view_swallow(VIEW_PTR(view), "button_options",
			      view.bt_options);
	evas_object_show(view.bt_options);

	/* Options */
	view.hv = elm_hover_add(win);
	elm_hover_parent_set(view.hv, win);
	elm_hover_target_set(view.hv, view.bt_options);

	view.bx = elm_box_add(win);
	elm_box_horizontal_set(view.bx, 0);
	elm_box_homogenous_set(view.bx, 1);
	evas_object_show(view.bx);

	view.bt_message = elm_button_add(win);
	elm_button_label_set(view.bt_message, D_("Import"));
	evas_object_size_hint_min_set(view.bt_message, 130, 80);
	evas_object_smart_callback_add(view.bt_message, "clicked",
				       _list_message_clicked, NULL);
	evas_object_show(view.bt_message);
	elm_box_pack_end(view.bx, view.bt_message);

	view.bt_edit = elm_button_add(win);
	elm_button_label_set(view.bt_edit, D_("Edit"));
	evas_object_size_hint_min_set(view.bt_edit, 130, 80);
	evas_object_smart_callback_add(view.bt_edit, "clicked",
				       _list_edit_clicked, NULL);
	evas_object_show(view.bt_edit);
	elm_box_pack_end(view.bx, view.bt_edit);

	view.bt_delete = elm_button_add(win);
	elm_button_label_set(view.bt_delete, D_("Delete"));
	evas_object_size_hint_min_set(view.bt_delete, 130, 80);
	evas_object_smart_callback_add(view.bt_delete, "clicked",
				       _list_delete_clicked, NULL);
	evas_object_show(view.bt_delete);
	elm_box_pack_end(view.bx, view.bt_delete);

	elm_hover_content_set(view.hv, "top", view.bx);

	sim_manager_list_fill(&view.list_data);

	return 0;
}

int
sim_manager_view_is_init()
{
	return ui_utils_view_is_init(VIEW_PTR(view));
}

void
sim_manager_view_deinit()
{
	ui_utils_view_deinit(VIEW_PTR(view));
}

void
sim_manager_view_show()
{
	evas_object_hide(view.hv);
	ui_utils_view_show(VIEW_PTR(view));
}

void
sim_manager_view_hide()
{
	ui_utils_view_hide(VIEW_PTR(view));
}
