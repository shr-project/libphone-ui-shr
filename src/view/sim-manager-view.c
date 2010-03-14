#include <glib.h>
#include <glib-object.h>

#include <phoneui/phoneui.h>
#include <phoneui/phoneui-utils.h>
#include <phoneui/phoneui-info.h>

#include "common-utils.h"
#include "sim-manager-view.h"
#include "ui-utils.h"
#include "views.h"

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
	struct SimManagerListData list_data;
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
	char *s = NULL;
	GValueArray *prop = (GValueArray *) data;

	if (!strcmp(part, "elm.text")) {
		s = phoneui_utils_sim_manager_display_phone_get(prop);
		if (s && *s) {
			return s;
		}
		else {
			s = strdup(CONTACT_NAME_UNDEFINED_STRING);
		}
	}
	else if (!strcmp(part, "elm.text.sub")) {
		s = phoneui_utils_sim_manager_display_name_get(prop);
		if (s && *s) {
			return s;
		}
		else {
			s = strdup(CONTACT_PHONE_UNDEFINED_STRING);
		}
	}

	return s;
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
_list_edit_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	evas_object_show(view.hv);
}

static void
_import_contact_cb(GError *error, char *path, gpointer data)
{
	(void) path;
	(void) data;
	if (error) {
		g_warning("Adding the contact failed");
		ui_utils_dialog(VIEW_PTR(view),
			D_("Failed to import contact!"),
			DIALOG_OK, NULL, NULL);
	}
	else {
		ui_utils_dialog(VIEW_PTR(view),
			D_("Contact added succesfull"),
			DIALOG_OK, NULL, NULL);
	}
}

static void
_list_import_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	Elm_Genlist_Item *it;
	char *name = NULL;
	char *phone = NULL;
	GValue *gval;

	evas_object_show(view.hv);

	it = elm_genlist_selected_item_get(view.list_data.list);
	if (it) {
		GValueArray *prop =
		(it) ? (GValueArray *) elm_genlist_item_data_get(it) : NULL;
		if (prop) {
			name = phoneui_utils_sim_manager_display_name_get(prop);
			phone = phoneui_utils_sim_manager_display_phone_get(prop);
		}
	}
	if (name && phone) {
		GHashTable *qry = g_hash_table_new_full
		     (g_str_hash, g_str_equal, NULL, common_utils_gvalue_free);
		gval = common_utils_new_gvalue_string(name);
		g_hash_table_insert(qry, "Name", gval);
		gval = common_utils_new_gvalue_string(phone);
		g_hash_table_insert(qry, "Phone", gval);
		phoneui_utils_contact_add(qry, _import_contact_cb, NULL);
		g_hash_table_destroy(qry);
	}
}

static void
_list_import_all_clicked(void *data, Evas_Object * obj, void *event_info)
{
	(void) data;
	(void) obj;
	(void) event_info;
	evas_object_show(view.hv);
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
_contact_delete_confirmiation_cb(GError *error, void *data)
{
	(void) data;
	if (error) {
		g_warning("Error while deleting entry!");
		ui_utils_dialog(VIEW_PTR(view),
			D_("Error while deleting entry!"),
			DIALOG_OK, NULL, NULL);
	}
}

static void
_contact_delete_confirm_cb(int result, void *data)
{
	if (result != DIALOG_YES)
		return;

	Elm_Genlist_Item *it = (Elm_Genlist_Item *)data;
	GValueArray *prop =
		(it) ? (GValueArray *) elm_genlist_item_data_get(it) : NULL;
	if (prop) {
		int index = phoneui_utils_sim_manager_display_index_get(prop);
		phoneui_utils_sim_contact_delete(index,
				_contact_delete_confirmiation_cb, NULL);
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
	return elm_genlist_item_append(list_data->list, &itc, entry,
				     NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
}

static void
_process_entry_cb(void *_entry, void *_data)
{
	Elm_Genlist_Item *it;
	GValueArray *entry = (GValueArray *) _entry;
	struct SimManagerListData *list_data =
				(struct SimManagerListData *) _data;
	it = sim_manager_list_item_add(list_data, entry);
	if (!it) {
		g_warning("Failed adding a contact to the list");
		return;
	}
}


static void
_process_entry(GError *error, GPtrArray *entry, void *_data)
{
	(void) error;
	g_ptr_array_foreach(entry, _process_entry_cb, _data);
}

void
sim_manager_list_fill(struct SimManagerListData *list_data)
{
	g_debug("sim_manager_list_fill()");
	list_data->current = 0;
	phoneui_utils_sim_manager_contacts_get(_process_entry, list_data);
}

static void
_delete_cb(struct View *view, Evas_Object *obj, void *event_info)
{
	(void)view;
	(void)obj;
	(void)event_info;
	sim_manager_view_hide();
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
				       _list_import_clicked, NULL);
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
